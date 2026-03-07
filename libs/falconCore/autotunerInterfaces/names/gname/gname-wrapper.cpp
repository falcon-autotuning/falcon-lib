#include "falcon_core/autotuner_interfaces/names/Gname.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using Gname   = falcon_core::autotuner_interfaces::names::Gname;
using GnameSP = std::shared_ptr<Gname>;

static void pack_opaque_gname(GnameSP gn, FalconResultSlot *out,
                              int32_t *out_count) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Gname";
  out[0].value.opaque.ptr       = new GnameSP(std::move(gn));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<GnameSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(name: string) -> (Gname gname)   [overload 1]
// New(name: int)    -> (Gname gname)   [overload 2]
// Dispatches on the runtime type of the "name" parameter.
void STRUCTGnameNew(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *out_count) {
  auto       pm       = unpack_params(params, param_count);
  const auto &name_var = pm.at("name");
  GnameSP    gn;
  if (std::holds_alternative<std::string>(name_var)) {
    gn = std::make_shared<Gname>(std::get<std::string>(name_var));
  } else {
    gn = std::make_shared<Gname>(
        static_cast<int>(std::get<int64_t>(name_var)));
  }
  pack_opaque_gname(std::move(gn), out, out_count);
}

// gname(this: Gname) -> (string out)
void STRUCTGnamegname(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *out_count) {
  GnameSP gn = get_opaque<Gname>(params, param_count, "this");
  pack_results(FunctionResult{gn->gname()}, out, 16, out_count);
}

// Equal(this: Gname, other: Gname) -> (bool equal)
void STRUCTGnameEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *out_count) {
  GnameSP gn    = get_opaque<Gname>(params, param_count, "this");
  GnameSP other = get_opaque<Gname>(params, param_count, "other");
  pack_results(FunctionResult{*gn == *other}, out, 16, out_count);
}

// NotEqual(this: Gname, other: Gname) -> (bool notequal)
void STRUCTGnameNotEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *out_count) {
  GnameSP gn    = get_opaque<Gname>(params, param_count, "this");
  GnameSP other = get_opaque<Gname>(params, param_count, "other");
  pack_results(FunctionResult{*gn != *other}, out, 16, out_count);
}

// ToJSON(this: Gname) -> (string json)
void STRUCTGnameToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  GnameSP gn = get_opaque<Gname>(params, param_count, "this");
  pack_results(FunctionResult{gn->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (Gname gname)
void STRUCTGnameFromJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *out_count) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  GnameSP     gn   = Gname::from_json_string<Gname>(json);
  pack_opaque_gname(std::move(gn), out, out_count);
}

} // extern "C"
