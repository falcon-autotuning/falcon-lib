#include "falcon-database/DeviceCharacteristic.hpp"
#include <iostream>
#include <sstream>

namespace falcon::database {

DatabaseConnection::DatabaseConnection(const std::string &connection_string) {
  try {
    conn_ = std::make_unique<pqxx::connection>(connection_string);
    if (!conn_->is_open()) {
      throw std::runtime_error("Failed to open database connection");
    }
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Database connection error: ") +
                             e.what());
  }
}

DatabaseConnection::~DatabaseConnection() = default;

void DatabaseConnection::initialize_schema() {
  try {
    pqxx::work txn(*conn_);

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
                hash TEXT NOT NULL,
                recordtime BIGINT NOT NULL,
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

void DatabaseConnection::insert(const DeviceCharacteristic &dc) {
  try {
    pqxx::work txn(*conn_);

    auto char_json = dc.characteristic.to_json().dump();

    // Use new exec() API instead of deprecated exec_params()
    txn.exec(
        R"(
            INSERT INTO device_characteristics (
                scope, name, barrier_gate, plunger_gate, reservoir_gate,
                screening_gate, extra, uncertainty, hash, recordtime,
                device_state, unit_name, device_characteristic
            ) VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13)
        )",
        pqxx::params(dc.scope, dc.name, dc.barrier_gate, dc.plunger_gate,
                     dc.reservoir_gate, dc.screening_gate, dc.extra,
                     dc.uncertainty, dc.hash, dc.time, dc.state, dc.unit_name,
                     char_json));

    txn.commit();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Insert error: ") + e.what());
  }
}

std::optional<DeviceCharacteristic>
DatabaseConnection::get_by_name(const std::string &name) {
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
    DeviceCharacteristic dc;
    dc.scope = row["scope"].c_str();
    dc.name = row["name"].c_str();
    dc.barrier_gate =
        row["barrier_gate"].is_null() ? "" : row["barrier_gate"].c_str();
    dc.plunger_gate =
        row["plunger_gate"].is_null() ? "" : row["plunger_gate"].c_str();
    dc.reservoir_gate =
        row["reservoir_gate"].is_null() ? "" : row["reservoir_gate"].c_str();
    dc.screening_gate =
        row["screening_gate"].is_null() ? "" : row["screening_gate"].c_str();
    dc.extra = row["extra"].is_null() ? "" : row["extra"].c_str();
    dc.uncertainty = row["uncertainty"].as<double>();
    dc.hash = row["hash"].c_str();
    dc.time = row["recordtime"].as<int64_t>();
    dc.state = row["device_state"].is_null() ? "" : row["device_state"].c_str();
    dc.unit_name = row["unit_name"].is_null() ? "" : row["unit_name"].c_str();

    if (!row["device_characteristic"].is_null()) {
      auto json_str = row["device_characteristic"].c_str();
      auto j = json::parse(json_str);
      dc.characteristic = JSONPrimitive::from_json(j);
    }

    return dc;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get by name error: ") + e.what());
  }
}

std::vector<DeviceCharacteristic>
DatabaseConnection::get_many(const std::vector<std::string> &names) {

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
      if (i > 0)
        query << ", ";
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
      DeviceCharacteristic dc;
      dc.scope = row["scope"].c_str();
      dc.name = row["name"].c_str();
      dc.barrier_gate =
          row["barrier_gate"].is_null() ? "" : row["barrier_gate"].c_str();
      dc.plunger_gate =
          row["plunger_gate"].is_null() ? "" : row["plunger_gate"].c_str();
      dc.reservoir_gate =
          row["reservoir_gate"].is_null() ? "" : row["reservoir_gate"].c_str();
      dc.screening_gate =
          row["screening_gate"].is_null() ? "" : row["screening_gate"].c_str();
      dc.extra = row["extra"].is_null() ? "" : row["extra"].c_str();
      dc.uncertainty = row["uncertainty"].as<double>();
      dc.hash = row["hash"].c_str();
      dc.time = row["recordtime"].as<int64_t>();
      dc.state =
          row["device_state"].is_null() ? "" : row["device_state"].c_str();
      dc.unit_name = row["unit_name"].is_null() ? "" : row["unit_name"].c_str();

      if (!row["device_characteristic"].is_null()) {
        auto json_str = row["device_characteristic"].c_str();
        auto j = json::parse(json_str);
        dc.characteristic = JSONPrimitive::from_json(j);
      }

      results.push_back(std::move(dc));
    }

    return results;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get many error: ") + e.what());
  }
}

std::vector<DeviceCharacteristic>
DatabaseConnection::get_by_hash_range(const std::string &hash_start,
                                      const std::string &hash_end) {

  std::vector<DeviceCharacteristic> results;

  try {
    pqxx::work txn(*conn_);

    auto result = txn.exec(R"(
            SELECT scope, name, barrier_gate, plunger_gate, reservoir_gate,
                   screening_gate, extra, uncertainty, hash, recordtime,
                   device_state, unit_name, device_characteristic
            FROM device_characteristics
            WHERE hash >= $1 AND hash <= $2
            ORDER BY hash, recordtime DESC
        )",
                           pqxx::params(hash_start, hash_end));

    for (auto row : result) {
      DeviceCharacteristic dc;
      dc.scope = row["scope"].c_str();
      dc.name = row["name"].c_str();
      dc.barrier_gate =
          row["barrier_gate"].is_null() ? "" : row["barrier_gate"].c_str();
      dc.plunger_gate =
          row["plunger_gate"].is_null() ? "" : row["plunger_gate"].c_str();
      dc.reservoir_gate =
          row["reservoir_gate"].is_null() ? "" : row["reservoir_gate"].c_str();
      dc.screening_gate =
          row["screening_gate"].is_null() ? "" : row["screening_gate"].c_str();
      dc.extra = row["extra"].is_null() ? "" : row["extra"].c_str();
      dc.uncertainty = row["uncertainty"].as<double>();
      dc.hash = row["hash"].c_str();
      dc.time = row["recordtime"].as<int64_t>();
      dc.state =
          row["device_state"].is_null() ? "" : row["device_state"].c_str();
      dc.unit_name = row["unit_name"].is_null() ? "" : row["unit_name"].c_str();

      if (!row["device_characteristic"].is_null()) {
        auto json_str = row["device_characteristic"].c_str();
        auto j = json::parse(json_str);
        dc.characteristic = JSONPrimitive::from_json(j);
      }

      results.push_back(std::move(dc));
    }

    return results;
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Get by hash range error: ") +
                             e.what());
  }
}

bool DatabaseConnection::delete_by_name(const std::string &name) {
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

int DatabaseConnection::delete_by_hash(const std::string &hash) {
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

void DatabaseConnection::clear_all() {
  try {
    pqxx::work txn(*conn_);
    txn.exec("DELETE FROM device_characteristics");
    txn.commit();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Clear all error: ") + e.what());
  }
}

size_t DatabaseConnection::count() {
  try {
    pqxx::work txn(*conn_);
    auto result = txn.exec("SELECT COUNT(*) FROM device_characteristics");
    return result[0][0].as<size_t>();
  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Count error: ") + e.what());
  }
}

bool DatabaseConnection::test_connection() {
  try {
    return conn_ && conn_->is_open();
  } catch (...) {
    return false;
  }
}

} // namespace falcon::database
