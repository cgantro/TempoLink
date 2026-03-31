#include "tempolink/juce/network/SignalingClient.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <utility>

#include <juce_events/juce_events.h>

namespace {

juce::String BuildWebSocketPath(const std::string& room_code,
                                const std::string& user_id) {
  const juce::String escaped_room =
      juce::URL::addEscapeChars(juce::String(room_code), true);
  const juce::String escaped_user =
      juce::URL::addEscapeChars(juce::String(user_id), true);
  return "/ws/signaling?roomCode=" + escaped_room + "&userId=" + escaped_user;
}

std::uint16_t ReadU16BE(const std::uint8_t* bytes) {
  return static_cast<std::uint16_t>((bytes[0] << 8) | bytes[1]);
}

std::uint64_t ReadU64BE(const std::uint8_t* bytes) {
  std::uint64_t value = 0;
  for (int i = 0; i < 8; ++i) {
    value = (value << 8) | bytes[i];
  }
  return value;
}

int FindHeaderEnd(const char* data, int size) {
  if (data == nullptr || size < 4) {
    return -1;
  }
  for (int i = 0; i <= size - 4; ++i) {
    if (data[i] == '\r' && data[i + 1] == '\n' && data[i + 2] == '\r' &&
        data[i + 3] == '\n') {
      return i + 4;
    }
  }
  return -1;
}

}  // namespace

SignalingClient::SignalingClient() = default;

SignalingClient::~SignalingClient() { disconnect(); }

bool SignalingClient::connect(const std::string& host, int port,
                              const std::string& room_code,
                              const std::string& user_id,
                              EventCallback callback) {
  disconnect();
  callback_ = std::move(callback);

  const juce::String path_query = BuildWebSocketPath(room_code, user_id);
  if (!performHandshake(host, port, path_query.toStdString())) {
    emitEvent(Event{Event::Type::Error, "", "", {}, "Signaling websocket handshake failed"});
    return false;
  }

  connected_ = true;
  receive_thread_ = std::thread([this] { receiveLoop(); });
  return true;
}

void SignalingClient::disconnect() {
  const bool was_connected = connected_.load();
  if (!was_connected) {
    if (receive_thread_.joinable()) {
      receive_thread_.join();
    }
    socket_.close();
    return;
  }

  const juce::MemoryBlock empty;
  sendFrame(0x8, empty);  // close frame
  connected_ = false;
  socket_.close();
  if (receive_thread_.joinable()) {
    receive_thread_.join();
  }
}

bool SignalingClient::isConnected() const { return connected_.load(); }

bool SignalingClient::performHandshake(const std::string& host, int port,
                                       const std::string& path_query) {
  pending_read_buffer_.reset();
  if (!socket_.connect(host, port, 3000)) {
    return false;
  }

  juce::MemoryBlock key_bytes;
  key_bytes.setSize(16);
  juce::Random::getSystemRandom().fillBitsRandomly(key_bytes.getData(),
                                                    key_bytes.getSize() * 8);
  const juce::String sec_key = juce::Base64::toBase64(key_bytes.getData(), 16);

  juce::String request;
  request << "GET " << path_query << " HTTP/1.1\r\n";
  request << "Host: " << host << ":" << port << "\r\n";
  request << "Upgrade: websocket\r\n";
  request << "Connection: Upgrade\r\n";
  request << "Sec-WebSocket-Version: 13\r\n";
  request << "Sec-WebSocket-Key: " << sec_key << "\r\n\r\n";

  if (socket_.write(request.toRawUTF8(), request.getNumBytesAsUTF8()) <= 0) {
    socket_.close();
    return false;
  }

  juce::MemoryOutputStream response;
  const int deadline_ms = 2500;
  int elapsed = 0;
  while (elapsed < deadline_ms) {
    if (socket_.waitUntilReady(true, 100) <= 0) {
      elapsed += 100;
      continue;
    }

    char buffer[512];
    const int read = socket_.read(buffer, static_cast<int>(sizeof(buffer)), false);
    if (read <= 0) {
      break;
    }
    response.write(buffer, static_cast<size_t>(read));
    const auto* raw = static_cast<const char*>(response.getData());
    const int size = static_cast<int>(response.getDataSize());
    const int header_end = FindHeaderEnd(raw, size);
    if (header_end > 0) {
      const juce::String header =
          juce::String::fromUTF8(raw, header_end).trimEnd();
      if (!header.startsWithIgnoreCase("HTTP/1.1 101")) {
        break;
      }

      if (size > header_end) {
        pending_read_buffer_.append(raw + header_end,
                                    static_cast<size_t>(size - header_end));
      }
      return true;
    }
  }

  socket_.close();
  return false;
}

void SignalingClient::receiveLoop() {
  while (connected_) {
    std::uint8_t header[2];
    if (!readExact(header, 2, 1000)) {
      continue;
    }

    const std::uint8_t opcode = header[0] & 0x0F;
    const bool masked = (header[1] & 0x80U) != 0;
    std::uint64_t payload_len = static_cast<std::uint64_t>(header[1] & 0x7FU);

    if (payload_len == 126) {
      std::uint8_t ext[2];
      if (!readExact(ext, 2, 1000)) {
        break;
      }
      payload_len = ReadU16BE(ext);
    } else if (payload_len == 127) {
      std::uint8_t ext[8];
      if (!readExact(ext, 8, 1000)) {
        break;
      }
      payload_len = ReadU64BE(ext);
    }

    std::array<std::uint8_t, 4> mask_key{0, 0, 0, 0};
    if (masked) {
      if (!readExact(mask_key.data(), 4, 1000)) {
        break;
      }
    }

    juce::MemoryBlock payload;
    payload.setSize(static_cast<size_t>(payload_len));
    if (payload_len > 0 && !readExact(payload.getData(),
                                      static_cast<int>(payload_len), 1000)) {
      break;
    }

    if (masked) {
      auto* bytes = static_cast<std::uint8_t*>(payload.getData());
      for (std::uint64_t i = 0; i < payload_len; ++i) {
        bytes[i] ^= mask_key[static_cast<std::size_t>(i % 4)];
      }
    }

    if (opcode == 0x8) {  // close
      break;
    }
    if (opcode == 0x9) {  // ping
      sendPong(payload);
      continue;
    }
    if (opcode == 0x1) {  // text
      const juce::String text(static_cast<const char*>(payload.getData()),
                              static_cast<int>(payload.getSize()));
      dispatchTextMessage(text);
    }
  }

  connected_ = false;
}

bool SignalingClient::readExact(void* out, int bytes, int timeout_ms) {
  auto* dst = static_cast<char*>(out);
  int total = 0;

  if (pending_read_buffer_.getSize() > 0) {
    const int cached =
        std::min(bytes, static_cast<int>(pending_read_buffer_.getSize()));
    std::memcpy(dst, pending_read_buffer_.getData(), static_cast<size_t>(cached));
    pending_read_buffer_.removeSection(0, static_cast<size_t>(cached));
    total += cached;
  }

  while (total < bytes) {
    if (!connected_) {
      return false;
    }
    const int ready = socket_.waitUntilReady(true, timeout_ms);
    if (ready <= 0) {
      return false;
    }
    const int n = socket_.read(dst + total, bytes - total, false);
    if (n <= 0) {
      return false;
    }
    total += n;
  }
  return true;
}

void SignalingClient::dispatchTextMessage(const juce::String& message_text) {
  const juce::var root = juce::JSON::parse(message_text);
  auto* obj = root.getDynamicObject();
  if (obj == nullptr) {
    return;
  }

  const juce::String type = obj->getProperty("type").toString();
  const std::string room_code =
      obj->getProperty("roomCode").toString().toStdString();
  const juce::var payload = obj->getProperty("payload");
  auto* payload_obj = payload.getDynamicObject();

  if (type == "room.joined" && payload_obj != nullptr) {
    Event event;
    event.type = Event::Type::RoomJoined;
    event.room_code = room_code;
    const juce::var participants = payload_obj->getProperty("participants");
    if (participants.isArray()) {
      if (const auto* arr = participants.getArray(); arr != nullptr) {
        for (const auto& item : *arr) {
          event.participants.push_back(item.toString().toStdString());
        }
      }
    }
    emitEvent(std::move(event));
    return;
  }

  if ((type == "peer.joined" || type == "peer.left") && payload_obj != nullptr) {
    Event event;
    event.type = (type == "peer.joined") ? Event::Type::PeerJoined
                                          : Event::Type::PeerLeft;
    event.room_code = room_code;
    event.user_id = payload_obj->getProperty("userId").toString().toStdString();
    emitEvent(std::move(event));
    return;
  }

  if (type == "signal.error") {
    Event event;
    event.type = Event::Type::Error;
    event.room_code = room_code;
    if (payload_obj != nullptr) {
      event.message = payload_obj->getProperty("message").toString();
    } else {
      event.message = "signaling error";
    }
    emitEvent(std::move(event));
  }
}

void SignalingClient::emitEvent(Event event) {
  if (!callback_) {
    return;
  }
  juce::MessageManager::callAsync([callback = callback_, event = std::move(event)]() mutable {
    callback(event);
  });
}

bool SignalingClient::sendFrame(std::uint8_t opcode,
                                const juce::MemoryBlock& payload) {
  if (!connected_) {
    return false;
  }

  std::lock_guard<std::mutex> lock(write_mutex_);

  juce::MemoryOutputStream frame;
  const std::uint8_t fin_opcode = 0x80U | (opcode & 0x0FU);
  frame.writeByte(static_cast<char>(fin_opcode));

  const std::size_t payload_size = payload.getSize();
  if (payload_size <= 125) {
    frame.writeByte(static_cast<char>(0x80U | static_cast<std::uint8_t>(payload_size)));
  } else if (payload_size <= 65535) {
    frame.writeByte(static_cast<char>(0x80U | 126U));
    const std::uint16_t len = static_cast<std::uint16_t>(payload_size);
    frame.writeByte(static_cast<char>((len >> 8) & 0xFF));
    frame.writeByte(static_cast<char>(len & 0xFF));
  } else {
    frame.writeByte(static_cast<char>(0x80U | 127U));
    const std::uint64_t len = static_cast<std::uint64_t>(payload_size);
    for (int i = 7; i >= 0; --i) {
      frame.writeByte(static_cast<char>((len >> (i * 8)) & 0xFF));
    }
  }

  std::array<std::uint8_t, 4> mask{};
  juce::Random::getSystemRandom().fillBitsRandomly(mask.data(), 32);
  frame.write(mask.data(), mask.size());

  const auto* raw = static_cast<const std::uint8_t*>(payload.getData());
  for (std::size_t i = 0; i < payload_size; ++i) {
    const std::uint8_t b = raw[i] ^ mask[i % 4];
    frame.writeByte(static_cast<char>(b));
  }

  return socket_.write(frame.getData(), static_cast<int>(frame.getDataSize())) > 0;
}

bool SignalingClient::sendPong(const juce::MemoryBlock& payload) {
  return sendFrame(0xA, payload);
}
