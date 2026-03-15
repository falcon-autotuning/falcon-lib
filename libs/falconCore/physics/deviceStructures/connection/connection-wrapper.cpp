
#include "falcon_core/physics/device_structures/Connection.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;

// Helper: pack an opaque ConnectionSP into a result slot
static void pack_opaque_connection(ConnectionSP conn, FalconResultSlot *out,
                                   int32_t *out_count) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr = new ConnectionSP(std::move(conn));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// NewPlungerGate(name: string) -> (Connection conn)
void STRUCTConnectionNewPlungerGate(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ConnectionSP conn = Connection::PlungerGate(name);
  pack_opaque_connection(std::move(conn), out, out_count);
}

// NewBarrierGate(name: string) -> (Connection conn)
void STRUCTConnectionNewBarrierGate(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ConnectionSP conn = Connection::BarrierGate(name);
  pack_opaque_connection(std::move(conn), out, out_count);
}

// NewReservoirGate(name: string) -> (Connection conn)
void STRUCTConnectionNewReservoirGate(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out,
                                      int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ConnectionSP conn = Connection::ReservoirGate(name);
  pack_opaque_connection(std::move(conn), out, out_count);
}

// NewScreeningGate(name: string) -> (Connection conn)
void STRUCTConnectionNewScreeningGate(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out,
                                      int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ConnectionSP conn = Connection::ScreeningGate(name);
  pack_opaque_connection(std::move(conn), out, out_count);
}

// NewOhmic(name: string) -> (Connection conn)
void STRUCTConnectionNewOhmic(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ConnectionSP conn = Connection::Ohmic(name);
  pack_opaque_connection(std::move(conn), out, out_count);
}

// Name(this: Connection) -> (string name)
void STRUCTConnectionName(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->name()}, out, 16, out_count);
}

// Type(this: Connection) -> (string type)
void STRUCTConnectionType(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->type()}, out, 16, out_count);
}

// IsDotGate(this: Connection) -> (bool)
void STRUCTConnectionIsDotGate(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_dot_gate()}, out, 16, out_count);
}

// IsBarrierGate(this: Connection) -> (bool)
void STRUCTConnectionIsBarrierGate(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_barrier_gate()}, out, 16, out_count);
}

// IsPlungerGate(this: Connection) -> (bool)
void STRUCTConnectionIsPlungerGate(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_plunger_gate()}, out, 16, out_count);
}

// IsScreeningGate(this: Connection) -> (bool)
void STRUCTConnectionIsScreeningGate(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_screening_gate()}, out, 16, out_count);
}

// IsReservoirGate(this: Connection) -> (bool)
void STRUCTConnectionIsReservoirGate(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_reservoir_gate()}, out, 16, out_count);
}

// IsOhmic(this: Connection) -> (bool)
void STRUCTConnectionIsOhmic(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_ohmic()}, out, 16, out_count);
}

// IsGate(this: Connection) -> (bool)
void STRUCTConnectionIsGate(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->is_gate()}, out, 16, out_count);
}

// Equal(this: Connection, other: Connection) -> (bool)
void STRUCTConnectionEqual(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  ConnectionSP other = get_opaque<Connection>(params, param_count, "other");
  pack_results(FunctionResult{*conn == *other}, out, 16, out_count);
}

// NotEqual(this: Connection, other: Connection) -> (bool)
void STRUCTConnectionNotEqual(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  ConnectionSP other = get_opaque<Connection>(params, param_count, "other");
  pack_results(FunctionResult{*conn != *other}, out, 16, out_count);
}

// ToJSON(this: Connection) -> (string)
void STRUCTConnectionToJSON(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  ConnectionSP conn = get_opaque<Connection>(params, param_count, "this");
  pack_results(FunctionResult{conn->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (Connection conn)
void STRUCTConnectionFromJSON(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  ConnectionSP conn = Connection::from_json_string<Connection>(json);
  pack_opaque_connection(std::move(conn), out, out_count);
}
} // namespace
