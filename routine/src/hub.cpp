#include "falcon-routine/hub.hpp"
#include "falcon-routine/log.hpp"
#include <cstdlib>
#include <stdexcept>
#include <vector>

namespace falcon::routine {

Hub &Hub::instance() {
  static Hub hub;
  return hub;
}

void Hub::ensure_connected() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (conn_) {
    return; // Already connected
  }

  const char *nats_url = std::getenv("NATS_URL");
  if (!nats_url || strlen(nats_url) == 0) {
    throw std::runtime_error("NATS_URL environment variable not set");
  }

  log::debug("Connecting to NATS: " + std::string(nats_url));

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

  log::info("NATS connected ✓");
}

void Hub::publish(const std::string &subject, const std::string &message) {
  ensure_connected();

  natsStatus s =
      natsConnection_PublishString(conn_, subject.c_str(), message.c_str());

  if (s != NATS_OK) {
    throw std::runtime_error("NATS publish failed: " +
                             std::string(natsStatus_GetText(s)));
  }
}

void Hub::publish_json(const std::string &subject, const nlohmann::json &data) {
  publish(subject, data.dump());
}

std::optional<std::string> Hub::request(const std::string &subject,
                                        const std::string &request,
                                        int timeout_ms) {
  ensure_connected();

  natsMsg *reply = nullptr;
  natsStatus s = natsConnection_RequestString(&reply, conn_, subject.c_str(),
                                              request.c_str(), timeout_ms);

  if (s == NATS_TIMEOUT) {
    log::warn("NATS request timeout: " + subject);
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

std::optional<nlohmann::json> Hub::request_json(const std::string &subject,
                                                const nlohmann::json &request,
                                                int timeout_ms) {
  auto response = this->request(subject, request.dump(), timeout_ms);

  if (!response) {
    return std::nullopt;
  }

  try {
    return nlohmann::json::parse(*response);
  } catch (const nlohmann::json::parse_error &e) {
    log::error("Failed to parse JSON response: " + std::string(e.what()));
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
    log::error("Exception in NATS message handler: " + std::string(e.what()));
  }

  natsMsg_Destroy(msg);
}

void Hub::subscribe(const std::string &subject,
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

  log::info("Subscribed to NATS subject: " + subject);
}

bool Hub::is_connected() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return conn_ != nullptr && natsConnection_IsClosed(conn_) == false;
}

void Hub::connect(const std::string &url) {
  if (!url.empty()) {
    // Override environment
    std::lock_guard<std::mutex> lock(mutex_);

    if (conn_) {
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

void Hub::disconnect() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (conn_) {
    natsConnection_Destroy(conn_);
    conn_ = nullptr;
    log::info("NATS disconnected");
  }
}

Hub::~Hub() { disconnect(); }

} // namespace falcon::routine
