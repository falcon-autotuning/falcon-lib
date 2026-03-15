#include <cstdlib>
#include <falcon-database/DatabaseConnection.hpp>
#include <falcon-database/SnapshotManager.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <mutex>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using namespace falcon::database;
using namespace falcon_core::physics::device_structures;

namespace {
std::shared_ptr<AdminDatabaseConnection> g_db_conn;
std::once_flag g_db_init_flag;

void initialize_database() {
  const char *env_url = std::getenv("FALCON_DATABASE_URL");
  std::string connection_string = env_url ? env_url : "";
  g_db_conn = std::make_shared<AdminDatabaseConnection>(connection_string);
}

std::shared_ptr<AdminDatabaseConnection> get_global_database() {
  std::call_once(g_db_init_flag, initialize_database);
  return g_db_conn;
}

std::shared_ptr<SnapshotManager> g_snapshot_mgr;
std::once_flag g_snap_init_flag;

std::shared_ptr<SnapshotManager> get_global_snapshot_manager() {
  std::call_once(g_snap_init_flag, [] {
    g_snapshot_mgr = std::make_shared<SnapshotManager>(get_global_database());
  });
  return g_snapshot_mgr;
}
} // namespace

// Helpers to extract optional strings
static std::string opt_str(const std::optional<std::string> &opt) {
  return opt ? *opt : "";
}

static std::string json_to_str(const nlohmann::json &j) {
  if (j.is_null())
    return "";
  if (j.is_string())
    return j.get<std::string>();
  return j.dump();
}

// Helper to convert Connection opaque to string (extracts Name attribute)
static std::string connection_to_str(const FalconParamEntry *param_entries,
                                     int32_t param_count,
                                     const char *param_name) {
  try {
    auto conn_ptr = get_opaque<void>(param_entries, param_count, param_name);
    if (!conn_ptr) {
      return "";
    }
    // Access the Connection object and extract its Name attribute
    // This assumes Connection has a GetName() method or Name field
    // For now, we'll return empty string if connection is null
    return "";
  } catch (...) {
    return "";
  }
}

// Macro to extract optional double (for uncertainty)
#define GET_OPTIONAL_DOUBLE(params, key)                                       \
  (params.count(key) ? std::make_optional(std::get<double>(params.at(key)))    \
                     : std::nullopt)

// Macro to extract optional string (for extra and connections)
#define GET_OPTIONAL_STRING(params, key)                                       \
  (params.count(key)                                                           \
       ? std::make_optional(std::get<std::string>(params.at(key)))             \
       : std::nullopt)

// ────────────────────────────────────────────────────────────────────────────
// DeviceCharacteristic
// ────────────────────────────────────────────────────────────────────────────

extern "C" {
void STRUCTDeviceCharacteristicNew(const FalconParamEntry *p, int32_t pc,
                                   FalconResultSlot *out, int32_t *oc) {
  auto q = std::make_shared<DeviceCharacteristic>();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristic";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristic>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristic> *>(ptr);
  };
  *oc = 1;
}

#define IMPLEMENT_GETTER(TypeName, MethodName, FieldExpr)                      \
  void STRUCT##TypeName##Get##MethodName(                                      \
      const FalconParamEntry *param_entries, int32_t param_count,              \
      FalconResultSlot *result_slots, int32_t *out_count) {                    \
    auto obj = get_opaque<TypeName>(param_entries, param_count, "this");       \
    pack_results(FunctionResult{FieldExpr}, result_slots,                      \
                 FALCON_RESULT_SLOT_COUNT, out_count);                         \
  }

#define IMPLEMENT_GETTER_CONNECTION(TypeName, MethodName, FieldName)           \
  void STRUCT##TypeName##Get##MethodName(                                      \
      const FalconParamEntry *param_entries, int32_t param_count,              \
      FalconResultSlot *result_slots, int32_t *out_count) {                    \
    auto obj = get_opaque<TypeName>(param_entries, param_count, "this");       \
    std::string conn_str = opt_str(obj->FieldName);                            \
    pack_results(FunctionResult{conn_str}, result_slots,                       \
                 FALCON_RESULT_SLOT_COUNT, out_count);                         \
  }

constexpr int FALCON_RESULT_SLOT_COUNT = 16;

IMPLEMENT_GETTER(DeviceCharacteristic, Scope, obj->scope)
IMPLEMENT_GETTER(DeviceCharacteristic, Name, obj->name)
IMPLEMENT_GETTER(DeviceCharacteristic, Hash, opt_str(obj->hash))
IMPLEMENT_GETTER(DeviceCharacteristic, State, opt_str(obj->state))
IMPLEMENT_GETTER(DeviceCharacteristic, UnitName, opt_str(obj->unit_name))
IMPLEMENT_GETTER(DeviceCharacteristic, Value, json_to_str(obj->characteristic))
IMPLEMENT_GETTER(DeviceCharacteristic, Extra, opt_str(obj->extra))

// Getter for Uncertainty (float)
void STRUCTDeviceCharacteristicGetUncertainty(
    const FalconParamEntry *param_entries, int32_t param_count,
    FalconResultSlot *result_slots, int32_t *out_count) {
  auto device_char =
      get_opaque<DeviceCharacteristic>(param_entries, param_count, "this");
  double uncertainty_val =
      device_char->uncertainty ? *device_char->uncertainty : 0.0;
  pack_results(FunctionResult{uncertainty_val}, result_slots,
               FALCON_RESULT_SLOT_COUNT, out_count);
}

// Getters for Connections (as strings)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristic, BarrierGate, barrier_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristic, PlungerGate, plunger_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristic, ReservoirGate, reservoir_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristic, ScreeningGate, screening_gate)

// Getter for Time (int)
void STRUCTDeviceCharacteristicGetTime(const FalconParamEntry *param_entries,
                                       int32_t param_count,
                                       FalconResultSlot *result_slots,
                                       int32_t *out_count) {
  auto device_char =
      get_opaque<DeviceCharacteristic>(param_entries, param_count, "this");
  int64_t time_val = device_char->time ? *device_char->time : 0;
  pack_results(FunctionResult{time_val}, result_slots, FALCON_RESULT_SLOT_COUNT,
               out_count);
}

#define IMPLEMENT_SETTER(TypeName, MethodName, FieldName, ParamName)           \
  void STRUCT##TypeName##Set##MethodName(const FalconParamEntry *p,            \
                                         int32_t pc, FalconResultSlot *out,    \
                                         int32_t *oc) {                        \
    auto params = unpack_params(p, pc);                                        \
    auto obj = get_opaque<TypeName>(p, pc, "this");                            \
    obj->FieldName = std::get<std::string>(params.at(ParamName));              \
    out[0] = {};                                                               \
    out[0].tag = FALCON_TYPE_OPAQUE;                                           \
    out[0].value.opaque.type_name = #TypeName;                                 \
    out[0].value.opaque.ptr = new std::shared_ptr<TypeName>(obj);              \
    out[0].value.opaque.deleter = [](void *ptr) {                              \
      delete static_cast<std::shared_ptr<TypeName> *>(ptr);                    \
    };                                                                         \
    *oc = 1;                                                                   \
  }

#define IMPLEMENT_SETTER_CONNECTION(TypeName, MethodName, FieldName,           \
                                    ParamName)                                 \
  void STRUCT##TypeName##Set##MethodName(const FalconParamEntry *p,            \
                                         int32_t pc, FalconResultSlot *out,    \
                                         int32_t *oc) {                        \
    auto params = unpack_params(p, pc);                                        \
    auto obj = get_opaque<TypeName>(p, pc, "this");                            \
    auto conn_str = get_opaque<Connection>(p, pc, ParamName);                  \
    obj->FieldName = std::make_optional(conn_str->name());                     \
    out[0] = {};                                                               \
    out[0].tag = FALCON_TYPE_OPAQUE;                                           \
    out[0].value.opaque.type_name = #TypeName;                                 \
    out[0].value.opaque.ptr = new std::shared_ptr<TypeName>(obj);              \
    out[0].value.opaque.deleter = [](void *ptr) {                              \
      delete static_cast<std::shared_ptr<TypeName> *>(ptr);                    \
    };                                                                         \
    *oc = 1;                                                                   \
  }

IMPLEMENT_SETTER(DeviceCharacteristic, Scope, scope, "scope")
IMPLEMENT_SETTER(DeviceCharacteristic, Name, name, "name")
IMPLEMENT_SETTER(DeviceCharacteristic, State, state, "state_val")
IMPLEMENT_SETTER(DeviceCharacteristic, Hash, hash, "hash")
IMPLEMENT_SETTER(DeviceCharacteristic, UnitName, unit_name, "unit_name")
IMPLEMENT_SETTER(DeviceCharacteristic, Value, characteristic, "characteristic")
IMPLEMENT_SETTER(DeviceCharacteristic, Extra, extra, "extra")

// Setters for Connections (as strings)
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristic, BarrierGate, barrier_gate,
                            "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristic, PlungerGate, plunger_gate,
                            "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristic, ReservoirGate, reservoir_gate,
                            "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristic, ScreeningGate, screening_gate,
                            "conn")

// Setter for Time (int)
void STRUCTDeviceCharacteristicSetTime(const FalconParamEntry *p, int32_t pc,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto q = get_opaque<DeviceCharacteristic>(p, pc, "this");
  int64_t val = std::get<int64_t>(pm.at("time"));
  q->time = std::make_optional(val);
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristic";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristic>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristic> *>(ptr);
  };
  *oc = 1;
}

// Setter for Uncertainty (float)
void STRUCTDeviceCharacteristicSetUncertainty(const FalconParamEntry *p,
                                              int32_t pc, FalconResultSlot *out,
                                              int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto q = get_opaque<DeviceCharacteristic>(p, pc, "this");
  double val = std::get<double>(pm.at("uncertainty"));
  q->uncertainty = std::make_optional(val);
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristic";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristic>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristic> *>(ptr);
  };
  *oc = 1;
}
}

// ────────────────────────────────────────────────────────────────────────────
// DeviceCharacteristicList
// ────────────────────────────────────────────────────────────────────────────
using DeviceCharacteristicList = std::vector<DeviceCharacteristic>;

extern "C" {
void STRUCTDeviceCharacteristicListSize(const FalconParamEntry *p, int32_t pc,
                                        FalconResultSlot *out, int32_t *oc) {
  auto list = get_opaque<DeviceCharacteristicList>(p, pc, "this");
  pack_results(FunctionResult{(int64_t)list->size()}, out, 16, oc);
}
void STRUCTDeviceCharacteristicListGet(const FalconParamEntry *p, int32_t pc,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto list = get_opaque<DeviceCharacteristicList>(p, pc, "this");

  auto result_sp = std::make_shared<DeviceCharacteristic>(list->at(idx));
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristic";
  out[0].value.opaque.ptr =
      new std::shared_ptr<DeviceCharacteristic>(result_sp);
  out[0].value.opaque.deleter = [](void *q) {
    delete static_cast<std::shared_ptr<DeviceCharacteristic> *>(q);
  };
  *oc = 1;
}
}

// ────────────────────────────────────────────────────────────────────────────
// DeviceCharacteristicQuery
// ────────────────────────────────────────────────────────────────────────────
extern "C" {
void STRUCTDeviceCharacteristicQueryNew(const FalconParamEntry *p, int32_t pc,
                                        FalconResultSlot *out, int32_t *oc) {
  auto q = std::make_shared<DeviceCharacteristicQuery>();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristicQuery";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristicQuery>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristicQuery> *>(ptr);
  };
  *oc = 1;
}

IMPLEMENT_GETTER(DeviceCharacteristicQuery, Scope, opt_str(obj->scope))
IMPLEMENT_GETTER(DeviceCharacteristicQuery, Name, opt_str(obj->name))
IMPLEMENT_GETTER(DeviceCharacteristicQuery, Hash, opt_str(obj->hash))
IMPLEMENT_GETTER(DeviceCharacteristicQuery, State, opt_str(obj->state))
IMPLEMENT_GETTER(DeviceCharacteristicQuery, UnitName, opt_str(obj->unit_name))
IMPLEMENT_GETTER(DeviceCharacteristicQuery, Extra, opt_str(obj->extra))
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristicQuery, BarrierGate,
                            barrier_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristicQuery, PlungerGate,
                            plunger_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristicQuery, ReservoirGate,
                            reservoir_gate)
IMPLEMENT_GETTER_CONNECTION(DeviceCharacteristicQuery, ScreeningGate,
                            screening_gate)

// Getter for Uncertainty (float)
void STRUCTDeviceCharacteristicQueryGetUncertainty(
    const FalconParamEntry *param_entries, int32_t param_count,
    FalconResultSlot *result_slots, int32_t *out_count) {
  auto obj =
      get_opaque<DeviceCharacteristicQuery>(param_entries, param_count, "this");
  double uncertainty_val = obj->uncertainty ? *obj->uncertainty : 0.0;
  pack_results(FunctionResult{uncertainty_val}, result_slots,
               FALCON_RESULT_SLOT_COUNT, out_count);
}

IMPLEMENT_SETTER(DeviceCharacteristicQuery, Scope, scope, "scope")
IMPLEMENT_SETTER(DeviceCharacteristicQuery, Name, name, "name")
IMPLEMENT_SETTER(DeviceCharacteristicQuery, State, state, "state_val")
IMPLEMENT_SETTER(DeviceCharacteristicQuery, Hash, hash, "hash")
IMPLEMENT_SETTER(DeviceCharacteristicQuery, UnitName, unit_name, "unit_name")
IMPLEMENT_SETTER(DeviceCharacteristicQuery, Extra, extra, "extra")

IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristicQuery, BarrierGate,
                            barrier_gate, "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristicQuery, PlungerGate,
                            plunger_gate, "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristicQuery, ReservoirGate,
                            reservoir_gate, "conn")
IMPLEMENT_SETTER_CONNECTION(DeviceCharacteristicQuery, ScreeningGate,
                            screening_gate, "conn")

// Setter for Uncertainty (float)
void STRUCTDeviceCharacteristicQuerySetUncertainty(const FalconParamEntry *p,
                                                   int32_t pc,
                                                   FalconResultSlot *out,
                                                   int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto q = get_opaque<DeviceCharacteristicQuery>(p, pc, "this");
  double val = std::get<double>(pm.at("uncertainty"));
  q->uncertainty = std::make_optional(val);
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristicQuery";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristicQuery>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristicQuery> *>(ptr);
  };
  *oc = 1;
}
}

// ────────────────────────────────────────────────────────────────────────────
// Database
// ────────────────────────────────────────────────────────────────────────────

static void pack_dchar_opt(std::optional<DeviceCharacteristic> opt,
                           FalconResultSlot *out, int32_t *oc) {
  if (opt.has_value()) {
    auto dchar = std::make_shared<DeviceCharacteristic>(*opt);
    out[0] = {};
    out[0].tag = FALCON_TYPE_OPAQUE;
    out[0].value.opaque.type_name = "DeviceCharacteristic";
    out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristic>(dchar);
    out[0].value.opaque.deleter = [](void *q) {
      delete static_cast<std::shared_ptr<DeviceCharacteristic> *>(q);
    };
    out[1] = {};
    out[1].tag = FALCON_TYPE_BOOL;
    out[1].value.bool_val = 1;
    *oc = 2;
  } else {
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    out[1] = {};
    out[1].tag = FALCON_TYPE_BOOL;
    out[1].value.bool_val = 0;
    *oc = 2;
  }
}

static void pack_dchar_list(std::vector<DeviceCharacteristic> list,
                            FalconResultSlot *out, int32_t *oc) {
  auto l = std::make_shared<DeviceCharacteristicList>(std::move(list));
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristicList";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristicList>(l);
  out[0].value.opaque.deleter = [](void *q) {
    delete static_cast<std::shared_ptr<DeviceCharacteristicList> *>(q);
  };
  *oc = 1;
}

extern "C" {
void GetByName(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
               int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto name = std::get<std::string>(pm.at("name"));
  pack_dchar_opt(get_global_database()->get_by_name(name), out, oc);
}

void GetAll(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
            int32_t *oc) {
  pack_dchar_list(get_global_database()->get_all(), out, oc);
}

void GetByHashRange(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto h1 = std::get<std::string>(pm.at("hash_start"));
  auto h2 = std::get<std::string>(pm.at("hash_end"));
  pack_dchar_list(get_global_database()->get_by_hash_range(h1, h2), out, oc);
}

void GetByQuery(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                int32_t *oc) {
  auto q = get_opaque<DeviceCharacteristicQuery>(p, pc, "query");
  pack_dchar_list(get_global_database()->get_by_query(*q), out, oc);
}

void Count(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
           int32_t *oc) {
  pack_results(FunctionResult{(int64_t)get_global_database()->count()}, out, 16,
               oc);
}

void IsConnected(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                 int32_t *oc) {
  pack_results(FunctionResult{get_global_database()->is_connected()}, out, 16,
               oc);
}

// Write methods
void Insert(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
            int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "dchar");
  get_global_database()->insert(*dchar);
  pack_results(FunctionResult{}, out, 16, oc);
}

void DeleteByName(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                  int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto name = std::get<std::string>(pm.at("name"));
  pack_results(FunctionResult{get_global_database()->delete_by_name(name)}, out,
               16, oc);
}

void DeleteByHash(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                  int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto hash = std::get<std::string>(pm.at("hash"));
  pack_results(
      FunctionResult{(int64_t)get_global_database()->delete_by_hash(hash)}, out,
      16, oc);
}

// Admin methods
void InitializeSchema(const FalconParamEntry *p, int32_t pc,
                      FalconResultSlot *out, int32_t *oc) {
  get_global_database()->initialize_schema();
  pack_results(FunctionResult{}, out, 16, oc);
}

void ClearAll(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
              int32_t *oc) {
  get_global_database()->clear_all();
  pack_results(FunctionResult{}, out, 16, oc);
}

// Snapshot methods
void ExportToJson(const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,
                  int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto filename = std::get<std::string>(pm.at("filename"));
  get_global_snapshot_manager()->export_to_json(filename);
  pack_results(FunctionResult{}, out, 16, oc);
}

void ImportFromJson(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto filename = std::get<std::string>(pm.at("filename"));
  bool clear_existing = std::get<bool>(pm.at("clear_existing"));
  get_global_snapshot_manager()->import_from_json(filename, clear_existing);
  pack_results(FunctionResult{}, out, 16, oc);
}
}
