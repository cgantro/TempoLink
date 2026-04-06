#include "tempolink/juce/network/transport/TlsSocketStream.h"

#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#endif

namespace tempolink::juceapp::network {

TlsSocketStream::TlsSocketStream(juce::StreamingSocket& socket) : socket_(socket) {}

TlsSocketStream::~TlsSocketStream() { Shutdown(); }

bool TlsSocketStream::Initialize(const std::string& host, juce::String* error_text) {
#if !defined(CPPHTTPLIB_OPENSSL_SUPPORT)
  juce::ignoreUnused(host);
  if (error_text != nullptr) {
    *error_text = "tls requested but openssl is disabled";
  }
  return false;
#else
  auto* ctx = SSL_CTX_new(TLS_client_method());
  if (ctx == nullptr) {
    if (error_text != nullptr) {
      *error_text = "SSL_CTX_new failed";
    }
    return false;
  }

  SSL_CTX_set_default_verify_paths(ctx);
  SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, nullptr);

  auto* ssl = SSL_new(ctx);
  if (ssl == nullptr) {
    SSL_CTX_free(ctx);
    if (error_text != nullptr) {
      *error_text = "SSL_new failed";
    }
    return false;
  }

  if (!host.empty()) {
    SSL_set_tlsext_host_name(ssl, host.c_str());
    X509_VERIFY_PARAM* verify_param = SSL_get0_param(ssl);
    X509_VERIFY_PARAM_set1_host(verify_param, host.c_str(), 0);
  }

  SSL_set_fd(ssl, socket_.getRawSocketHandle());
  const int connect_result = SSL_connect(ssl);
  if (connect_result <= 0) {
    const unsigned long err = ERR_get_error();
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    if (error_text != nullptr) {
      *error_text =
          "SSL_connect failed: " + juce::String(ERR_error_string(err, nullptr));
    }
    return false;
  }

  if (SSL_get_verify_result(ssl) != X509_V_OK) {
    const long verify_error = SSL_get_verify_result(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    if (error_text != nullptr) {
      *error_text =
          "TLS cert verify failed: " + juce::String(static_cast<int>(verify_error));
    }
    return false;
  }

  ssl_ctx_ = ctx;
  ssl_ = ssl;
  return true;
#endif
}

void TlsSocketStream::Shutdown() {
#if defined(CPPHTTPLIB_OPENSSL_SUPPORT)
  if (ssl_ != nullptr) {
    SSL_free(static_cast<SSL*>(ssl_));
    ssl_ = nullptr;
  }
  if (ssl_ctx_ != nullptr) {
    SSL_CTX_free(static_cast<SSL_CTX*>(ssl_ctx_));
    ssl_ctx_ = nullptr;
  }
#else
  ssl_ = nullptr;
  ssl_ctx_ = nullptr;
#endif
}

int TlsSocketStream::ReadSome(void* out, int bytes, int timeout_ms) {
  if (bytes <= 0 || out == nullptr) {
    return 0;
  }
#if !defined(CPPHTTPLIB_OPENSSL_SUPPORT)
  juce::ignoreUnused(out, bytes, timeout_ms);
  return -1;
#else
  auto* ssl = static_cast<SSL*>(ssl_);
  if (ssl == nullptr) {
    return -1;
  }
  for (;;) {
    const int n = SSL_read(ssl, out, bytes);
    if (n > 0) {
      return n;
    }
    const int ssl_error = SSL_get_error(ssl, n);
    if (ssl_error == SSL_ERROR_WANT_READ) {
      if (socket_.waitUntilReady(true, timeout_ms) <= 0) {
        return 0;
      }
      continue;
    }
    if (ssl_error == SSL_ERROR_WANT_WRITE) {
      if (socket_.waitUntilReady(false, timeout_ms) <= 0) {
        return 0;
      }
      continue;
    }
    return -1;
  }
#endif
}

int TlsSocketStream::WriteSome(const void* in, int bytes, int timeout_ms) {
  if (bytes <= 0 || in == nullptr) {
    return 0;
  }
#if !defined(CPPHTTPLIB_OPENSSL_SUPPORT)
  juce::ignoreUnused(in, bytes, timeout_ms);
  return -1;
#else
  auto* ssl = static_cast<SSL*>(ssl_);
  if (ssl == nullptr) {
    return -1;
  }
  for (;;) {
    const int n = SSL_write(ssl, in, bytes);
    if (n > 0) {
      return n;
    }
    const int ssl_error = SSL_get_error(ssl, n);
    if (ssl_error == SSL_ERROR_WANT_WRITE) {
      if (socket_.waitUntilReady(false, timeout_ms) <= 0) {
        return 0;
      }
      continue;
    }
    if (ssl_error == SSL_ERROR_WANT_READ) {
      if (socket_.waitUntilReady(true, timeout_ms) <= 0) {
        return 0;
      }
      continue;
    }
    return -1;
  }
#endif
}

}  // namespace tempolink::juceapp::network

