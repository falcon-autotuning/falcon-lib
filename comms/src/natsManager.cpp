#include "falcon-comms/natsManager.hpp"
#include <cstdlib>
#include <spdlog/spdlog.h>
#include <stdexcept>

namespace falcon::comms {

NatsManager &NatsManager::instance() {
  static NatsManager nm;
  return nm;
}

void NatsManager::ensure_connected() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (conn_) {
    return; // Already connected
  }
  const char *nats_url = std::getenv("NATS_URL");
  if (!nats_url || strlen(nats_url) == 0) {
    throw std::runtime_error("NATS_URL environment variable not set");
  }
  spdlog::debug("Connecting to NATS: {}", nats_url);
  natsOptions *opts = nullptr;
  natsStatus s = natsOptions_Create(&opts);
  if (s != NATS_OK) {
    throw std::runtime_error("Failed to create NATS options: " +
                             std::string(natsStatus_GetText(s)));
  }
  natsOptions_SetURL(opts, nats_url);
  natsOptions_SetTimeout(opts, 5000); // 5 second connection timeout
  s = natsConnection_Connect(&conn_, opts);
  natsOptions_Destroy(opts);
  if (s != NATS_OK) {
    throw std::runtime_error("Failed to connect to NATS: " +
                             std::string(natsStatus_GetText(s)));
  }
  spdlog::info("NATS connected ✓");
}

void NatsManager::ensure_jetstream() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (js_) {
    return; // Already initialized
  }
  if (!conn_) {
    throw std::runtime_error(
        "NATS connection must be established before JetStream");
  }
  // Initialize JetStream context with NULL options (use defaults)
  natsStatus s = natsConnection_JetStream(&js_, conn_, NULL);
  if (s != NATS_OK) {
    throw std::runtime_error("Failed to initialize JetStream: " +
                             std::string(natsStatus_GetText(s)));
  }
  spdlog::info("JetStream initialized ✓");
}

void NatsManager::publish(const std::string &subject,
                          const std::string &message) {
  ensure_connected();
  natsStatus s =
      natsConnection_PublishString(conn_, subject.c_str(), message.c_str());
  if (s != NATS_OK) {
    throw std::runtime_error("NATS publish failed: " +
                             std::string(natsStatus_GetText(s)));
  }
}

void NatsManager::publish_json(const std::string &subject,
                               const nlohmann::json &data) {
  publish(subject, data.dump());
}

std::optional<std::string> NatsManager::request(const std::string &subject,
                                                const std::string &request,
                                                int timeout_ms) {
  ensure_connected();
  natsMsg *reply = nullptr;
  natsStatus s = natsConnection_RequestString(&reply, conn_, subject.c_str(),
                                              request.c_str(), timeout_ms);
  if (s == NATS_TIMEOUT) {
    spdlog::warn("NATS request timeout: {}", subject);
    return std::nullopt;
  }
  if (s != NATS_OK) {
    throw std::runtime_error("NATS request failed: " +
                             std::string(natsStatus_GetText(s)));
  }
  if (!reply) {
    return std::nullopt;
  }
  std::string response(natsMsg_GetData(reply), natsMsg_GetDataLength(reply));
  natsMsg_Destroy(reply);
  return response;
}

std::optional<nlohmann::json>
NatsManager::request_json(const std::string &subject,
                          const nlohmann::json &request, int timeout_ms) {
  auto response = this->request(subject, request.dump(), timeout_ms);
  if (!response) {
    return std::nullopt;
  }
  try {
    return nlohmann::json::parse(*response);
  } catch (const nlohmann::json::parse_error &e) {
    spdlog::error("Failed to parse JSON response: {}", e.what());
    return std::nullopt;
  }
}

// Callback wrapper for NATS subscription
static void message_handler(natsConnection *nc, natsSubscription *sub,
                            natsMsg *msg, void *closure) {
  (void)nc;
  (void)sub;
  auto *callback =
      static_cast<std::function<void(const std::string &)> *>(closure);
  std::string data(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));
  try {
    (*callback)(data);
  } catch (const std::exception &e) {
    spdlog::error("Exception in NATS message handler: {}", e.what());
  }
  natsMsg_Destroy(msg);
}

void NatsManager::subscribe(const std::string &subject,
                            std::function<void(const std::string &)> callback) {
  ensure_connected();
  // Allocate callback on heap (will be cleaned up when subscription is
  // destroyed)
  auto *cb = new std::function<void(const std::string &)>(callback);
  natsSubscription *sub = nullptr;
  natsStatus s = natsConnection_Subscribe(&sub, conn_, subject.c_str(),
                                          message_handler, cb);
  if (s != NATS_OK) {
    delete cb;
    throw std::runtime_error("NATS subscribe failed: " +
                             std::string(natsStatus_GetText(s)));
  }
  spdlog::info("Subscribed to NATS subject: {}", subject);
}

std::vector<std::string>
NatsManager::jetstream_pull(const std::string &stream,
                            const std::string &consumer, int batch_size) {
  ensure_connected();
  ensure_jetstream();
  std::vector<std::string> messages;

  // Set up subscription options to bind to stream/consumer and enable manual
  // ack
  jsSubOptions so;
  jsSubOptions_Init(&so);
  so.Stream = stream.c_str();
  so.Consumer = consumer.c_str();
  so.ManualAck = true;

  // Synchronous JetStream subscription
  natsSubscription *sub = nullptr;
  natsStatus s = js_SubscribeSync(&sub, js_, stream.c_str(), NULL, &so, NULL);
  if (s != NATS_OK) {
    throw std::runtime_error("JetStream subscribe failed: " +
                             std::string(natsStatus_GetText(s)));
  }

  // Pull messages (up to batch_size)
  for (int i = 0; i < batch_size; ++i) {
    natsMsg *msg = nullptr;
    s = natsSubscription_NextMsg(&msg, sub, 1000); // 1000 ms timeout
    if (s == NATS_OK && msg) {
      messages.emplace_back(natsMsg_GetData(msg), natsMsg_GetDataLength(msg));
      natsMsg_Ack(msg, NULL);
      natsMsg_Destroy(msg);
    } else {
      break; // No more messages or timeout
    }
  }
  natsSubscription_Destroy(sub);

  return messages;
}

jsCtx *NatsManager::get_jetstream_context() {
  ensure_jetstream();
  return js_;
}

bool NatsManager::is_connected() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return conn_ != nullptr && natsConnection_IsClosed(conn_) == false;
}

void NatsManager::connect(const std::string &url) {
  if (!url.empty()) {
    // Override environment
    std::lock_guard<std::mutex> lock(mutex_);
    if (conn_) {
      if (js_) {
        jsCtx_Destroy(js_);
        js_ = nullptr;
      }
      natsConnection_Destroy(conn_);
      conn_ = nullptr;
    }
    natsOptions *opts = nullptr;
    natsOptions_Create(&opts);
    natsOptions_SetURL(opts, url.c_str());

    natsStatus s = natsConnection_Connect(&conn_, opts);
    natsOptions_Destroy(opts);

    if (s != NATS_OK) {
      throw std::runtime_error("Failed to connect to NATS: " +
                               std::string(natsStatus_GetText(s)));
    }
  } else {
    ensure_connected();
  }
}

void NatsManager::disconnect() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (js_) {
    jsCtx_Destroy(js_);
    js_ = nullptr;
  }
  if (conn_) {
    natsConnection_Destroy(conn_);
    conn_ = nullptr;
    spdlog::info("NATS disconnected");
  }
}

NatsManager::~NatsManager() { disconnect(); }

} // namespace falcon::comms
