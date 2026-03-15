#include "falcon_core/autotuner_interfaces/interpretations/InterpretationContainer.hpp"
#include "falcon_core/autotuner_interfaces/interpretations/InterpretationContext.hpp"
#include "falcon_core/autotuner_interfaces/contexts/MeasurementContext.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include "falcon_core/generic/Map.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Connection          = falcon_core::physics::device_structures::Connection;
using ConnectionSP        = std::shared_ptr<Connection>;
using SymbolUnit          = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP        = std::shared_ptr<SymbolUnit>;
using MeasurementContext  = falcon_core::autotuner_interfaces::contexts::MeasurementContext;
using MeasurementContextSP = std::shared_ptr<MeasurementContext>;
using InterpretationContext   = falcon_core::autotuner_interfaces::interpretations::InterpretationContext;
using InterpretationContextSP = std::shared_ptr<InterpretationContext>;

// Monomorphize on double for the generic T
using IC   = falcon_core::autotuner_interfaces::interpretations::InterpretationContainer<double>;
using ICSP = std::shared_ptr<IC>;

static void pack_ic(ICSP ic, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InterpretationContainer";
  out[0].value.opaque.ptr       = new ICSP(std::move(ic));
  out[0].value.opaque.deleter   = [](void *p) { delete static_cast<ICSP *>(p); };
  *oc = 1;
}

static void pack_ic_ctx(InterpretationContextSP ctx, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InterpretationContext";
  out[0].value.opaque.ptr       = new InterpretationContextSP(std::move(ctx));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<InterpretationContextSP *>(p);
  };
  *oc = 1;
}

// Pack a list of InterpretationContext as an Array
static void pack_ctx_array(
    falcon_core::generic::ListSP<InterpretationContext> list,
    FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>();
  for (const auto &ctx : list->items()) {
    auto inst = std::make_shared<StructInstance>("InterpretationContext");
    auto sp   = std::make_shared<InterpretationContextSP>(ctx);
    inst->native_handle = std::static_pointer_cast<void>(sp);
    inst->is_native     = true;
    arr->items.push_back(inst);
  }
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

extern "C" {

// New(map: Map<InterpretationContext, T>) -> (InterpretationContainer cont)
// For simplicity, build an empty IC (map extraction from opaque Map is complex)
void STRUCTInterpretationContainerNew(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  // Build a minimal Map<InterpretationContext, double> with no entries
  // (full map extraction requires deep DSL Map internals)
  auto map_sp = std::make_shared<falcon_core::generic::Map<InterpretationContext, double>>();
  auto ic     = std::make_shared<IC>(map_sp);
  pack_ic(std::move(ic), out, oc);
}

// Unit(this) -> (SymbolUnit unit)
void STRUCTInterpretationContainerUnit(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<IC>(params, param_count, "this");
  auto unit = ic->unit();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// SelectByConnection(conn) -> Array<InterpretationContext>
void STRUCTInterpretationContainerSelectByConnection(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<IC>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto list = ic->select_by_connection(conn);
  pack_ctx_array(list, out, oc);
}

// SelectByConnections(conn) -> Array<InterpretationContext>
void STRUCTInterpretationContainerSelectByConnections(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<IC>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto list = ic->select_by_connections({conn});
  pack_ctx_array(list, out, oc);
}

// SelectByIndependentConnection(conn) -> Array<InterpretationContext>
void STRUCTInterpretationContainerSelectByIndependentConnection(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<IC>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto list = ic->select_by_independent_connection(conn);
  pack_ctx_array(list, out, oc);
}

// SelectByDependentConnection(conn) -> Array<InterpretationContext>
void STRUCTInterpretationContainerSelectByDependentConnection(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<IC>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto list = ic->select_by_dependent_connection(conn);
  pack_ctx_array(list, out, oc);
}

// SelectContexts(independent, dependent) -> Array<InterpretationContext>
void STRUCTInterpretationContainerSelectContexts(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto ic = get_opaque<IC>(params, param_count, "this");
  auto empty_list =
      std::make_shared<falcon_core::generic::List<Connection>>();
  auto list = ic->select_contexts(empty_list, empty_list);
  pack_ctx_array(list, out, oc);
}

// Equal
void STRUCTInterpretationContainerEqual(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<IC>(params, param_count, "this");
  auto other = get_opaque<IC>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual
void STRUCTInterpretationContainerNotEqual(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  auto self  = get_opaque<IC>(params, param_count, "this");
  auto other = get_opaque<IC>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON
void STRUCTInterpretationContainerToJSON(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto ic = get_opaque<IC>(params, param_count, "this");
  pack_results(FunctionResult{ic->to_json_string()}, out, 16, oc);
}

// FromJSON
void STRUCTInterpretationContainerFromJSON(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out,
                                            int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  ICSP ic   = IC::from_json_string<IC>(json);
  pack_ic(std::move(ic), out, oc);
}

} // extern "C"
