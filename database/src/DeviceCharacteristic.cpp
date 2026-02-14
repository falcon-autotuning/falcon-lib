#include "falcon-database/DeviceCharacteristic.hpp"
#include <stdexcept>

namespace falcon::database {

// JSONPrimitive implementation
bool JSONPrimitive::as_bool() const {
  if (type_ != Type::Boolean) {
    throw std::runtime_error("JSONPrimitive is not a boolean");
  }
  return bool_value_;
}

int64_t JSONPrimitive::as_int() const {
  if (type_ != Type::Integer) {
    throw std::runtime_error("JSONPrimitive is not an integer");
  }
  return int_value_;
}

double JSONPrimitive::as_double() const {
  if (type_ != Type::Double) {
    throw std::runtime_error("JSONPrimitive is not a double");
  }
  return double_value_;
}

std::string JSONPrimitive::as_string() const {
  if (type_ != Type::String) {
    throw std::runtime_error("JSONPrimitive is not a string");
  }
  return string_value_;
}

json JSONPrimitive::to_json() const {
  switch (type_) {
  case Type::Null:
    return nullptr;
  case Type::Boolean:
    return bool_value_;
  case Type::Integer:
    return int_value_;
  case Type::Double:
    return double_value_;
  case Type::String:
    return string_value_;
  }
  return nullptr;
}

JSONPrimitive JSONPrimitive::from_json(const json &j) {
  if (j.is_null()) {
    return {};
  } else if (j.is_boolean()) {
    return JSONPrimitive(j.get<bool>());
  } else if (j.is_number_integer()) {
    return JSONPrimitive(j.get<int64_t>());
  } else if (j.is_number_float()) {
    return JSONPrimitive(j.get<double>());
  } else if (j.is_string()) {
    return JSONPrimitive(j.get<std::string>());
  }
  throw std::runtime_error("Unsupported JSON type for JSONPrimitive");
}

// DeviceCharacteristic implementation
json DeviceCharacteristic::to_json() const {
  return json{{"scope", scope},
              {"name", name},
              {"barrier_gate", barrier_gate},
              {"plunger_gate", plunger_gate},
              {"reservoir_gate", reservoir_gate},
              {"screening_gate", screening_gate},
              {"extra", extra},
              {"uncertainty", uncertainty},
              {"hash", hash},
              {"recordtime", time},
              {"device_state", state},
              {"unit_name", unit_name},
              {"device_characteristic", characteristic.to_json()}};
}

DeviceCharacteristic DeviceCharacteristic::from_json(const json &j) {
  DeviceCharacteristic dc;
  dc.scope = j.value("scope", "");
  dc.name = j.value("name", "");
  dc.barrier_gate = j.value("barrier_gate", "");
  dc.plunger_gate = j.value("plunger_gate", "");
  dc.reservoir_gate = j.value("reservoir_gate", "");
  dc.screening_gate = j.value("screening_gate", "");
  dc.extra = j.value("extra", "");
  dc.uncertainty = j.value("uncertainty", 0.0);
  dc.hash = j.value("hash", "");
  dc.time = j.value("recordtime", 0L);
  dc.state = j.value("device_state", "");
  dc.unit_name = j.value("unit_name", "");

  if (j.contains("device_characteristic")) {
    dc.characteristic = JSONPrimitive::from_json(j["device_characteristic"]);
  }

  return dc;
}

std::vector<DeviceCharacteristic> DatabaseConnection::get_all() {
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
    throw std::runtime_error(std::string("Get all error: ") + e.what());
  }
}

} // namespace falcon::database
