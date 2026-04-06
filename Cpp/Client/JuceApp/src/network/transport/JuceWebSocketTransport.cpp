#include "tempolink/juce/network/transport/JuceWebSocketTransport.h"

#include <utility>

#include "tempolink/juce/logging/AppLogger.h"
#include "tempolink/juce/network/transport/WebSocketFrameCodec.h"
#include "tempolink/juce/network/transport/WebSocketHandshake.h"

namespace tempolink::juceapp::network {

JuceWebSocketTransport::JuceWebSocketTransport() = default;

JuceWebSocketTransport::~JuceWebSocketTransport() { Disconnect(); }

bool JuceWebSocketTransport::Connect(const std::string& host, int port,
                                     const std::string& path, bool use_tls) {
  Disconnect();
  use_tls_ = use_tls;

  juce::String connect_error;
  if (!stream_.Connect(host, port, use_tls_, 3000, &connect_error)) {
    tempolink::juceapp::logging::Error(
        "WebSocket connect failed: host=" + juce::String(host) +
        ", port=" + juce::String(port) +
        ", tls=" + juce::String(use_tls_ ? "true" : "false") +
        ", reason=" + connect_error);
    return false;
  }

  auto handshake = WebSocketHandshake::Perform(stream_, host, port, path);
  if (!handshake.ok) {
    juce::String detail = handshake.error_text;
    if (handshake.status_line.isNotEmpty()) {
      detail += ", status=" + handshake.status_line;
    }
    tempolink::juceapp::logging::Error(
        "WebSocket handshake failed: host=" + juce::String(host) +
        ", path=" + juce::String(path) +
        ", reason=" + detail);
    stream_.Close();
    return false;
  }

  pending_read_buffer_ = std::move(handshake.pending_bytes);
  connected_ = true;
  receive_thread_ = std::thread([this] { ReceiveLoop(); });
  return true;
}

void JuceWebSocketTransport::Disconnect() {
  const bool was_connected = connected_.exchange(false);
  if (was_connected) {
    const juce::MemoryBlock empty_payload;
    std::lock_guard<std::mutex> lock(write_mutex_);
    WebSocketFrameCodec::WriteClientFrame(stream_, Opcode::Close, empty_payload);
  }

  stream_.Close();
  if (receive_thread_.joinable()) {
    receive_thread_.join();
  }
}

bool JuceWebSocketTransport::IsConnected() const { return connected_.load(); }

bool JuceWebSocketTransport::SendFrame(Opcode opcode,
                                       const juce::MemoryBlock& payload) {
  if (!connected_) {
    return false;
  }
  std::lock_guard<std::mutex> lock(write_mutex_);
  return WebSocketFrameCodec::WriteClientFrame(stream_, opcode, payload);
}

void JuceWebSocketTransport::SetOnFrameReceived(OnFrameReceived callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  on_frame_received_ = std::move(callback);
}

void JuceWebSocketTransport::SetOnDisconnected(OnDisconnected callback) {
  std::lock_guard<std::mutex> lock(callback_mutex_);
  on_disconnected_ = std::move(callback);
}

void JuceWebSocketTransport::ReceiveLoop() {
  while (connected_) {
    const auto read =
        WebSocketFrameCodec::ReadServerFrame(stream_, pending_read_buffer_, 1000);
    if (read.status == WebSocketReadStatus::kTimeout) {
      continue;
    }
    if (read.status == WebSocketReadStatus::kClosed) {
      break;
    }
    if (read.status == WebSocketReadStatus::kError ||
        read.status == WebSocketReadStatus::kProtocolError) {
      break;
    }
    if (read.status != WebSocketReadStatus::kOk) {
      break;
    }

    if (read.is_ping) {
      SendFrame(Opcode::Pong, read.frame.payload);
      continue;
    }

    OnFrameReceived callback;
    {
      std::lock_guard<std::mutex> lock(callback_mutex_);
      callback = on_frame_received_;
    }
    if (callback) {
      callback(read.frame);
    }
  }

  connected_ = false;

  OnDisconnected dc_callback;
  {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    dc_callback = on_disconnected_;
  }
  if (dc_callback) {
    dc_callback();
  }
}

}  // namespace tempolink::juceapp::network

