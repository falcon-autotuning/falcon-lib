#include "falcon-database/DeviceCharacteristic.hpp"
#include <optional>
#include <string>
#include <vector>

namespace falcon::routine {

/**
 * @brief Simple helper functions for database access
 *
 * These maintain a persistent connection internally for performance.
 * Connection is established on first use and reused automatically.
 *
 * Environment: DATABASE_URL must be set
 */
std::optional<std::string> get(const std::string &name);
std::optional<nlohmann::json> get_json(const std::string &name);

// Query
std::vector<std::string> query_by_scope(const std::string &scope);
bool exists(const std::string &name);

// Advanced: direct access to DeviceCharacteristic
void insert(const falcon::database::DeviceCharacteristic &dc);
std::optional<falcon::database::DeviceCharacteristic>
get_full(const std::string &name);

// Explicit connection management (optional)
void connect(const std::string &connection_string = "");
void disconnect();
bool is_connected();
} // namespace falcon::routine
