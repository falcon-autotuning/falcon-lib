#include <cstdlib>
#include <falcon-database/DatabaseConnection.hpp>
#include <falcon-database/SnapshotManager.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <mutex>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using namespace falcon::database;

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

// ----------------------------------------------------------------------------
// DeviceCharacteristic
// ----------------------------------------------------------------------------

extern "C" {
void STRUCTDeviceCharacteristicGetScope(const FalconParamEntry *p, int32_t pc,
                                        FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{dchar->scope}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetName(const FalconParamEntry *p, int32_t pc,
                                       FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{dchar->name}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetHash(const FalconParamEntry *p, int32_t pc,
                                       FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{opt_str(dchar->hash)}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetTime(const FalconParamEntry *p, int32_t pc,
                                       FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  int64_t t = dchar->time ? *dchar->time : 0;
  pack_results(FunctionResult{t}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetState(const FalconParamEntry *p, int32_t pc,
                                        FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{opt_str(dchar->state)}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetUnitName(const FalconParamEntry *p,
                                           int32_t pc, FalconResultSlot *out,
                                           int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{opt_str(dchar->unit_name)}, out, 16, oc);
}
void STRUCTDeviceCharacteristicGetValue(const FalconParamEntry *p, int32_t pc,
                                        FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{opt_str(dchar->characteristic)}, out, 16, oc);
}
void STRUCTDeviceCharacteristicToJson(const FalconParamEntry *p, int32_t pc,
                                      FalconResultSlot *out, int32_t *oc) {
  auto dchar = get_opaque<DeviceCharacteristic>(p, pc, "this");
  pack_results(FunctionResult{dchar->to_json().dump()}, out, 16, oc);
}
}

// ----------------------------------------------------------------------------
// DeviceCharacteristicList
// ----------------------------------------------------------------------------
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

// ----------------------------------------------------------------------------
// DeviceCharacteristicQuery
// ----------------------------------------------------------------------------
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

void STRUCTDeviceCharacteristicQuerySetScope(const FalconParamEntry *p,
                                             int32_t pc, FalconResultSlot *out,
                                             int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto q = get_opaque<DeviceCharacteristicQuery>(p, pc, "this");
  q->scope = std::get<std::string>(pm.at("scope"));
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceCharacteristicQuery";
  out[0].value.opaque.ptr = new std::shared_ptr<DeviceCharacteristicQuery>(q);
  out[0].value.opaque.deleter = [](void *ptr) {
    delete static_cast<std::shared_ptr<DeviceCharacteristicQuery> *>(ptr);
  };
  *oc = 1;
}

#define IMPLEMENT_SETTER(MethodName, FieldName, ParamName)                     \
  void STRUCTDeviceCharacteristicQuerySet##MethodName(                         \
      const FalconParamEntry *p, int32_t pc, FalconResultSlot *out,            \
      int32_t *oc) {                                                           \
    auto pm = unpack_params(p, pc);                                            \
    auto q = get_opaque<DeviceCharacteristicQuery>(p, pc, "this");             \
    q->FieldName = std::get<std::string>(pm.at(ParamName));                    \
    out[0] = {};                                                               \
    out[0].tag = FALCON_TYPE_OPAQUE;                                           \
    out[0].value.opaque.type_name = "DeviceCharacteristicQuery";               \
    out[0].value.opaque.ptr =                                                  \
        new std::shared_ptr<DeviceCharacteristicQuery>(q);                     \
    out[0].value.opaque.deleter = [](void *ptr) {                              \
      delete static_cast<std::shared_ptr<DeviceCharacteristicQuery> *>(ptr);   \
    };                                                                         \
    *oc = 1;                                                                   \
  }

IMPLEMENT_SETTER(Name, name, "name")
IMPLEMENT_SETTER(State, state, "state_val")
IMPLEMENT_SETTER(Hash, hash, "hash")
IMPLEMENT_SETTER(UnitName, unit_name, "unit_name")
}

// ----------------------------------------------------------------------------
// Database
// ----------------------------------------------------------------------------

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

void TestConnection(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{get_global_database()->test_connection()}, out,
               16, oc);
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
