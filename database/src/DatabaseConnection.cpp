#include "falcon-database/DatabaseConnection.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
template <typename Row>
std::optional<std::string> get_opt_str(const Row &row, const char *col) {
  return row[col].is_null() ? std::nullopt
                            : std::make_optional(row[col].c_str());
}
template <typename Row>
std::optional<double> get_opt_double(const Row &row, const char *col) {
  return row[col].is_null()
             ? std::nullopt
             : std::make_optional(row[col].template as<double>());
}
template <typename Row>
std::optional<int64_t> get_opt_int64(const Row &row, const char *col) {
  return row[col].is_null()
             ? std::nullopt
             : std::make_optional(row[col].template as<int64_t>());
}
falcon::database::DeviceCharacteristic dchar_from_row(const pqxx::row &row) {
  falcon::database::DeviceCharacteristic dchar;
  dchar.scope = row["scope"].c_str();
  dchar.name = row["name"].c_str();
  dchar.barrier_gate = get_opt_str(row, "barrier_gate");
  dchar.plunger_gate = get_opt_str(row, "plunger_gate");
  dchar.reservoir_gate = get_opt_str(row, "reservoir_gate");
  dchar.screening_gate = get_opt_str(row, "screening_gate");
  dchar.extra = get_opt_str(row, "extra");
  dchar.uncertainty = get_opt_double(row, "uncertainty");
  dchar.hash = get_opt_str(row, "hash");
  dchar.time = get_opt_int64(row, "recordtime");
  dchar.state = get_opt_str(row, "device_state");
  dchar.unit_name = get_opt_str(row, "unit_name");
  if (!row["device_characteristic"].is_null()) {
    const auto *json_str = row["device_characteristic"].c_str();
    auto json = nlohmann::json::parse(json_str);
    dchar.characteristic = falcon::database::JSONPrimitive::from_json(json);
  }
  return dchar;
}

std::string resolve_connection_string(const std::string &explicit_conn_str) {
  // Priority 1: Explicit connection string
  if (!explicit_conn_str.empty()) {
    return explicit_conn_str;
  }

  // Priority 2: FALCON_DATABASE_URL environment variable
  const char *env_url = std::getenv("FALCON_DATABASE_URL");
  if (env_url != nullptr && strlen(env_url) > 0) {
    return std::string(env_url);
  }

  // No valid connection string found
  throw std::runtime_error("No database connection string provided. "
                           "Either pass a connection string explicitly or set "
                           "FALCON_DATABASE_URL environment variable.");
}

} // namespace

namespace falcon::database {

ReadOnlyDatabaseConnection::ReadOnlyDatabaseConnection(
    const std::string &connection_string)
    : conn_str_(resolve_connection_string(connection_string)),
      connected_(false) {
  // Don't connect yet - lazy initialization
}

ReadOnlyDatabaseConnection::~ReadOnlyDatabaseConnection() = default;

void ReadOnlyDatabaseConnection::ensure_connected() {
  std::lock_guard<std::mutex> lock(conn_mutex_);

  if (connected_ && conn_ && conn_->is_open()) {
    return; // Already connected and alive
  }

  try {
    conn_ = std::make_unique<pqxx::connection>(conn_str_);
    if (!conn_->is_open()) {
      throw std::runtime_error("Failed to open database connection");
    }
    connected_ = true;
  } catch (const std::exception &e) {
    connected_ = false;
    throw std::runtime_error(std::string("Database connection error: ") +
                             e.what());
  }
}

ReadWriteDatabaseConnection::ReadWriteDatabaseConnection(
    const std::string &connection_string)
    : ReadOnlyDatabaseConnection(connection_string) {}

ReadWriteDatabaseConnection::~ReadWriteDatabaseConnection() = default;

AdminDatabaseConnection::AdminDatabaseConnection(
    const std::string &connection_string)
    : ReadWriteDatabaseConnection(connection_string) {}

AdminDatabaseConnection::~AdminDatabaseConnection() = default;

void AdminDatabaseConnection::initialize_schema() {
  ensure_connected(); // Establish connection before schema operations

  try {
    pqxx::work txn(*conn_);

    txn.exec("DROP TABLE IF EXISTS device_characteristics CASCADE;");
    txn.exec(R"(
            CREATE TABLE IF NOT EXISTS device_characteristics (
                id SERIAL PRIMARY KEY,
                scope TEXT NOT NULL,
                name TEXT NOT NULL,
                barrier_gate TEXT,
                plunger_gate TEXT,
                reservoir_gate TEXT,
                screening_gate TEXT,
                extra TEXT,
                uncertainty DOUBLE PRECISION DEFAULT 0.0,
                hash TEXT,
                recordtime BIGINT,
                device_state TEXT,
                unit_name TEXT,
                device_characteristic JSONB,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");

    // Create indexes for performance
    txn.exec("CREATE INDEX IF NOT EXISTS idx_dc_name ON "
             "device_characteristics(name)");
    txn.exec("CREATE INDEX IF NOT EXISTS idx_dc_hash ON "
             "device_characteristics(hash)");
    txn.exec("CREATE INDEX IF NOT EXISTS idx_dc_time ON "
             "device_characteristics(recordtime)");

    txn.commit();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Schema initialization error: ") +
                             e.what());
  }
}

void ReadWriteDatabaseConnection::insert(const DeviceCharacteristic &dchar) {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);
    auto char_json = dchar.characteristic.to_json().dump();

    auto str_or_null =
        [](const std::optional<std::string> &var) -> const char * {
      return var ? var->c_str() : nullptr;
    };
    auto dbl_or_null =
        [](const std::optional<double> &var) -> std::optional<double> {
      return var ? var : std::nullopt;
    };
    auto int_or_null =
        [](const std::optional<int64_t> &var) -> std::optional<int64_t> {
      return var ? var : std::nullopt;
    };

    txn.exec(
        R"(
            INSERT INTO device_characteristics (
                scope, name, barrier_gate, plunger_gate, reservoir_gate,
                screening_gate, extra, uncertainty, hash, recordtime,
                device_state, unit_name, device_characteristic
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
        )",
        pqxx::params(
            dchar.scope, dchar.name, str_or_null(dchar.barrier_gate),
            str_or_null(dchar.plunger_gate), str_or_null(dchar.reservoir_gate),
            str_or_null(dchar.screening_gate), str_or_null(dchar.extra),
            dbl_or_null(dchar.uncertainty), str_or_null(dchar.hash),
            int_or_null(dchar.time), str_or_null(dchar.state),
            str_or_null(dchar.unit_name), char_json));

    txn.commit();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Insert error: ") + e.what());
  }
}

std::optional<DeviceCharacteristic>
ReadOnlyDatabaseConnection::get_by_name(const std::string &name) {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);
    auto result = txn.exec(R"(
            SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
                   screening_gate, extra, uncertainty, hash, recordtime,
                   device_state, unit_name, device_characteristic
            FROM device_characteristics
            WHERE name = $1
            ORDER BY recordtime DESC
            LIMIT 1
        )",
                           pqxx::params(name));
    if (result.empty()) {
      return std::nullopt;
    }

    auto row = result[0];
    return dchar_from_row(row);

  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get by name error: ") + e.what());
  }
}

std::vector<DeviceCharacteristic>
ReadOnlyDatabaseConnection::get_many(const std::vector<std::string> &names) {
  ensure_connected();

  std::vector<DeviceCharacteristic> results;
  if (names.empty()) {
    return results;
  }
  try {
    pqxx::work txn(*conn_);
    // Build IN clause with placeholders
    std::ostringstream query;
    query << R"(
            SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
                   screening_gate, extra, uncertainty, hash, recordtime,
                   device_state, unit_name, device_characteristic
            FROM device_characteristics
            WHERE name IN (
        )";
    for (size_t i = 0; i < names.size(); ++i) {
      if (i > 0) {
        query << ", ";
      }
      query << "$" << (i + 1);
    }
    query << ") ORDER BY recordtime DESC";

    // Build params from vector
    pqxx::params params;
    for (const auto &name : names) {
      params.append(name);
    }

    auto result = txn.exec(query.str(), params);

    for (auto row : result) {
      results.push_back(dchar_from_row(row));
    }

    return results;

  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get many error: ") + e.what());
  }
}

std::vector<DeviceCharacteristic>
ReadOnlyDatabaseConnection::get_by_hash_range(const std::string &hash_start,
                                              const std::string &hash_end) {
  ensure_connected();

  std::vector<DeviceCharacteristic> results;
  try {
    pqxx::work txn(*conn_);

    auto result = txn.exec(R"(
            SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
                   screening_gate, extra, uncertainty, hash, recordtime,
                   device_state, unit_name, device_characteristic
            FROM device_characteristics
            WHERE string_to_array(hash, '.')::int[] >= string_to_array($1, '.')::int[]
              AND string_to_array(hash, '.')::int[] <= string_to_array($2, '.')::int[]
            ORDER BY string_to_array(hash, '.')::int[], recordtime DESC
        )",
                           pqxx::params(hash_start, hash_end));

    for (auto row : result) {
      results.push_back(dchar_from_row(row));
    }

    return results;

  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get by hash range error: ") +
                             e.what());
  }
}

bool ReadWriteDatabaseConnection::delete_by_name(const std::string &name) {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);

    auto result = txn.exec("DELETE FROM device_characteristics WHERE name = $1",
                           pqxx::params(name));

    txn.commit();
    return result.affected_rows() > 0;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Delete by name error: ") + e.what());
  }
}

int ReadWriteDatabaseConnection::delete_by_hash(const std::string &hash) {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);

    auto result = txn.exec("DELETE FROM device_characteristics WHERE hash = $1",
                           pqxx::params(hash));

    txn.commit();
    return static_cast<int>(result.affected_rows());
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Delete by hash error: ") + e.what());
  }
}

void AdminDatabaseConnection::clear_all() {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);
    txn.exec("DELETE FROM device_characteristics");
    txn.commit();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Clear all error: ") + e.what());
  }
}

size_t ReadOnlyDatabaseConnection::count() {
  ensure_connected();

  try {
    pqxx::work txn(*conn_);
    auto result = txn.exec("SELECT COUNT(*) FROM device_characteristics");
    return result[0][0].as<size_t>();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Count error: ") + e.what());
  }
}

bool ReadOnlyDatabaseConnection::test_connection() {
  try {
    ensure_connected();
    return conn_ && conn_->is_open();
  } catch (...) {
    return false;
  }
}

std::vector<DeviceCharacteristic> ReadOnlyDatabaseConnection::get_all() {
  ensure_connected();

  std::vector<DeviceCharacteristic> results;
  try {
    pqxx::work txn(*conn_);
    auto result = txn.exec(R"(
            SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
                   screening_gate, extra, uncertainty, hash, recordtime,
                   device_state, unit_name, device_characteristic
            FROM device_characteristics
            ORDER BY name, recordtime DESC
        )");

    for (auto row : result) {
      results.push_back(dchar_from_row(row));
    }
    return results;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get all error: ") + e.what());
  }
}

std::vector<DeviceCharacteristic> ReadOnlyDatabaseConnection::get_by_query(
    const DeviceCharacteristicQuery &query) {
  ensure_connected();

  std::vector<DeviceCharacteristic> results;
  try {
    pqxx::work txn(*conn_);
    std::ostringstream sql;
    sql << R"(
      SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
             screening_gate, extra, uncertainty, hash, recordtime,
             device_state, unit_name, device_characteristic
      FROM device_characteristics
    )";

    std::vector<std::string> conditions;
    pqxx::params params;

    auto add_condition = [&](const auto &opt, const std::string &field) {
      if (opt) {
        conditions.push_back(field + " = $" +
                             std::to_string(params.size() + 1));
        params.append(*opt);
      }
    };

    add_condition(query.scope, "scope");
    add_condition(query.name, "name");
    add_condition(query.barrier_gate, "barrier_gate");
    add_condition(query.plunger_gate, "plunger_gate");
    add_condition(query.reservoir_gate, "reservoir_gate");
    add_condition(query.screening_gate, "screening_gate");
    add_condition(query.extra, "extra");
    add_condition(query.uncertainty, "uncertainty");
    add_condition(query.hash, "hash");
    add_condition(query.time, "recordtime");
    add_condition(query.state, "device_state");
    add_condition(query.unit_name, "unit_name");

    if (!conditions.empty()) {
      sql << " WHERE ";
      for (size_t i = 0; i < conditions.size(); ++i) {
        if (i > 0) {
          sql << " AND ";
        }
        sql << conditions[i];
      }
    }
    sql << " ORDER BY recordtime DESC";

    auto result = txn.exec(sql.str(), params);

    for (auto row : result) {
      results.push_back(dchar_from_row(row));
    }
    return results;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get by query error: ") + e.what());
  }
}

} // namespace falcon::database
