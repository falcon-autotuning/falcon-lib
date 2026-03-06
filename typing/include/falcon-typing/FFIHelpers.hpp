/**
 * @file FFIHelpers.hpp
 * @brief C++ helpers for converting between FalconFFI C types and
 *        RuntimeValue / ParameterMap / FunctionResult.
 *
 * Two directions:
 *   - engine_to_c  : ParameterMap  → FalconParamEntry[]   (before call)
 *   - c_to_engine  : FalconResultSlot[] → FunctionResult  (after call)
 *   - wrapper_from_c : FalconParamEntry[] → ParameterMap  (in wrapper)
 *   - wrapper_to_c   : FunctionResult → FalconResultSlot[]  (in wrapper)
 */
#pragma once

#include "falcon-typing/PrimitiveTypes.hpp"
#include "falcon-typing/falcon_ffi.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace falcon::typing::ffi {

// ============================================================================
// Helpers used by the ENGINE side (packing params, unpacking results)
// ============================================================================
namespace engine {

struct PackedParams {
  std::vector<FalconParamEntry> entries;
  std::vector<std::string> key_storage;
  std::vector<std::string> str_storage;
};

inline PackedParams pack_params(const ParameterMap &params) {
  PackedParams packed;
  packed.entries.reserve(params.size());
  packed.key_storage.reserve(params.size());
  packed.str_storage.reserve(params.size());

  for (const auto &[k, v] : params) {
    packed.key_storage.push_back(k);
    FalconParamEntry e{};
    e.key = packed.key_storage.back().c_str();

    std::visit(
        [&](auto &&val) {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, int64_t>) {
            e.tag = FALCON_TYPE_INT;
            e.value.int_val = val;
          } else if constexpr (std::is_same_v<T, double>) {
            e.tag = FALCON_TYPE_FLOAT;
            e.value.float_val = val;
          } else if constexpr (std::is_same_v<T, bool>) {
            e.tag = FALCON_TYPE_BOOL;
            e.value.bool_val = val ? 1 : 0;
          } else if constexpr (std::is_same_v<T, std::string>) {
            packed.str_storage.push_back(val);
            e.tag = FALCON_TYPE_STRING;
            e.value.str.ptr = packed.str_storage.back().c_str();
            e.value.str.len = (int32_t)packed.str_storage.back().size();
          } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
            e.tag = FALCON_TYPE_NIL;
          } else if constexpr (std::is_same_v<T, ErrorObject>) {
            using SP = std::shared_ptr<ErrorObject>;
            auto *heap_sp = new SP(std::make_shared<ErrorObject>(val));
            e.tag = FALCON_TYPE_OPAQUE;
            e.value.opaque.ptr = heap_sp;
            e.value.opaque.type_name = "ErrorObject";
            e.value.opaque.deleter = [](void *ptr) {
              delete static_cast<SP *>(ptr);
            };
          } else {
            using SP = T;
            auto *heap_sp = new SP(val);
            e.tag = FALCON_TYPE_OPAQUE;
            e.value.opaque.ptr = heap_sp;
            if constexpr (std::is_same_v<SP, std::shared_ptr<TupleValue>>) {
              e.value.opaque.type_name = "TupleValue";
              e.value.opaque.deleter = [](void *p) {
                delete static_cast<SP *>(p);
              };
            } else if constexpr (std::is_same_v<SP, std::shared_ptr<ArrayValue>>) {
              e.value.opaque.type_name = "ArrayValue";
              e.value.opaque.deleter = [](void *p) {
                delete static_cast<SP *>(p);
              };
            } else if constexpr (std::is_same_v<SP, std::shared_ptr<StructInstance>>) {
              if (val && val->is_native()) {
                // Native FFI struct: forward the native_handle as a
                // shared_ptr<void>* so the wrapper can static_pointer_cast
                // it back to the concrete type.
                auto *svp = new std::shared_ptr<void>(
                    std::static_pointer_cast<void>(
                        val->native_handle.value()));
                delete heap_sp;
                e.value.opaque.ptr = svp;
                e.value.opaque.type_name = val->type_name.c_str();
                e.value.opaque.deleter = [](void *p) {
                  delete static_cast<std::shared_ptr<void> *>(p);
                };
              } else {
                // Plain FAL struct — pass as StructInstance opaque
                e.value.opaque.type_name = "StructInstance";
                e.value.opaque.deleter = [](void *p) {
                  delete static_cast<SP *>(p);
                };
              }
            } else {
              e.value.opaque.type_name = "unknown_opaque";
              e.value.opaque.deleter = [](void *p) {
                delete static_cast<SP *>(p);
              };
            }
          };
        },
        v);

    packed.entries.push_back(e);
  }
  return packed;
}

inline FunctionResult unpack_results(FalconResultSlot *slots, int32_t count) {
  FunctionResult result;
  result.reserve((size_t)count);

  for (int32_t i = 0; i < count; ++i) {
    FalconResultSlot &s = slots[i];
    switch (s.tag) {
    case FALCON_TYPE_NIL:
      result.emplace_back(nullptr);
      break;
    case FALCON_TYPE_INT:
      result.emplace_back(s.value.int_val);
      break;
    case FALCON_TYPE_FLOAT:
      result.emplace_back(s.value.float_val);
      break;
    case FALCON_TYPE_BOOL:
      result.emplace_back(s.value.bool_val != 0);
      break;
    case FALCON_TYPE_STRING: {
      std::string str(s.value.str.ptr, (size_t)s.value.str.len);
      ::free(s.value.str.ptr); // wrapper must use malloc/strdup
      result.emplace_back(std::move(str));
      break;
    }
    case FALCON_TYPE_OPAQUE: {
      std::string tn = s.value.opaque.type_name ? s.value.opaque.type_name : "";
      void *ptr = s.value.opaque.ptr;
      auto del = s.value.opaque.deleter;

      RuntimeValue rv = nullptr;
      if (tn == "TupleValue") {
        using SP = std::shared_ptr<TupleValue>;
        rv = *static_cast<SP *>(ptr);
        if (del != nullptr) del(ptr);
      } else if (tn == "ArrayValue") {
        using SP = std::shared_ptr<ArrayValue>;
        rv = *static_cast<SP *>(ptr);
        if (del != nullptr) del(ptr);
      } else if (tn == "StructInstance") {
        using SP = std::shared_ptr<StructInstance>;
        rv = *static_cast<SP *>(ptr);
        if (del != nullptr) del(ptr);
      } else if (tn == "ErrorObject") {
        using SP = std::shared_ptr<ErrorObject>;
        rv = **static_cast<SP *>(ptr);
        if (del != nullptr) del(ptr);
      } else {
        // Unknown user opaque (e.g. "InstrumentPort", "Ports", "Array", etc.)
        //
        // ptr = new shared_ptr<void>*(the_native_object) — heap-allocated.
        // Reinterpret as shared_ptr<void>* to recover shared ownership,
        // then store as StructInstance with native_handle so the interpreter
        // can pass it back into subsequent wrapper calls.
        auto inner_sp = *reinterpret_cast<std::shared_ptr<void> *>(ptr);
        auto inst = std::make_shared<StructInstance>(tn);
        inst->native_handle = inner_sp;
        if (del) del(ptr);
        rv = std::move(inst);
      }
      result.push_back(std::move(rv));
      break;
    }
    default:
      result.emplace_back(nullptr);
      break;
    }
  }
  return result;
}

} // namespace engine

// ============================================================================
// Helpers used by the WRAPPER side (unpacking params, packing results)
// ============================================================================
namespace wrapper {

inline ParameterMap unpack_params(const FalconParamEntry *entries,
                                  int32_t count) {
  ParameterMap result;
  for (int32_t i = 0; i < count; ++i) {
    const FalconParamEntry &e = entries[i];
    std::string key(e.key);
    switch (e.tag) {
    case FALCON_TYPE_NIL:
      result[key] = nullptr;
      break;
    case FALCON_TYPE_INT:
      result[key] = e.value.int_val;
      break;
    case FALCON_TYPE_FLOAT:
      result[key] = e.value.float_val;
      break;
    case FALCON_TYPE_BOOL:
      result[key] = e.value.bool_val != 0;
      break;
    case FALCON_TYPE_STRING:
      result[key] = std::string(e.value.str.ptr, (size_t)e.value.str.len);
      break;
    case FALCON_TYPE_OPAQUE: {
      std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
      void *ptr = e.value.opaque.ptr;
      if (tn == "StructInstance") {
        using SP = std::shared_ptr<StructInstance>;
        result[key] = *static_cast<SP *>(ptr);
      } else if (tn == "TupleValue") {
        using SP = std::shared_ptr<TupleValue>;
        result[key] = *static_cast<SP *>(ptr);
      } else if (tn == "ArrayValue") {
        using SP = std::shared_ptr<ArrayValue>;
        result[key] = *static_cast<SP *>(ptr);
      } else if (tn == "ErrorObject") {
        using SP = std::shared_ptr<ErrorObject>;
        result[key] = **static_cast<SP *>(ptr);
      } else {
        // ── FIX: user-defined native struct (e.g. "InstrumentPort",
        // "Connection", "Array", …) ──────────────────────────────────────────
        //
        // engine::pack_params sends these as:
        //   ptr = new shared_ptr<void>*(val->native_handle.value())
        //   type_name = val->type_name   (e.g. "InstrumentPort")
        //
        // We must reconstruct the StructInstance so downstream code (e.g.
        // STRUCTArrayPushBack storing it in arr->elements, then
        // STRUCTPortsNew reading it back) gets a proper StructInstance with
        // native_handle rather than nullptr.
        //
        // Previously this branch stored nullptr, causing any wrapper that
        // called unpack_params and then stored the result (like
        // STRUCTArrayPushBack does for the "value" param) to silently lose
        // the object and record nullptr in the array instead.
        auto inner_sp = *reinterpret_cast<std::shared_ptr<void> *>(ptr);
        auto inst = std::make_shared<StructInstance>(tn);
        inst->native_handle = inner_sp;
        result[key] = std::move(inst);
      }
      break;
    }
    default:
      result[key] = nullptr;
      break;
    }
  }
  return result;
}

/**
 * Extract an opaque (user-defined) shared_ptr from a FalconParamEntry array.
 *
 * Convention: for native structs, ptr is a heap-allocated shared_ptr<void>*
 * aliasing T.  We static_pointer_cast back to T.
 */
template <typename T>
inline std::shared_ptr<T> get_opaque(const FalconParamEntry *entries,
                                     int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) == 0 &&
        entries[i].tag == FALCON_TYPE_OPAQUE) {
      // ptr is a heap-allocated shared_ptr<void>* (alias to T).
      auto sv = *static_cast<std::shared_ptr<void> *>(entries[i].value.opaque.ptr);
      return std::static_pointer_cast<T>(sv);
    }
  }
  throw std::runtime_error(std::string("FFI: opaque param not found: ") + key);
}

inline void pack_results(const FunctionResult &result, FalconResultSlot *slots,
                         int32_t capacity, int32_t *out_count) {
  int32_t n = (int32_t)result.size();
  n = std::min(n, capacity);
  *out_count = n;

  for (int32_t i = 0; i < n; ++i) {
    FalconResultSlot &s = slots[i];
    s = {};

    std::visit(
        [&](auto &&val) {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, int64_t>) {
            s.tag = FALCON_TYPE_INT;
            s.value.int_val = val;
          } else if constexpr (std::is_same_v<T, double>) {
            s.tag = FALCON_TYPE_FLOAT;
            s.value.float_val = val;
          } else if constexpr (std::is_same_v<T, bool>) {
            s.tag = FALCON_TYPE_BOOL;
            s.value.bool_val = val ? 1 : 0;
          } else if constexpr (std::is_same_v<T, std::string>) {
            s.tag = FALCON_TYPE_STRING;
            s.value.str.len = (int32_t)val.size();
            s.value.str.ptr = (char *)::malloc(val.size() + 1);
            std::memcpy(s.value.str.ptr, val.c_str(), val.size() + 1);
          } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
            s.tag = FALCON_TYPE_NIL;
          } else if constexpr (std::is_same_v<T, ErrorObject>) {
            using SP = std::shared_ptr<ErrorObject>;
            auto *heap_sp = new SP(std::make_shared<ErrorObject>(val));
            s.tag = FALCON_TYPE_OPAQUE;
            s.value.opaque.ptr = heap_sp;
            s.value.opaque.type_name = "ErrorObject";
            s.value.opaque.deleter = [](void *ptr) {
              delete static_cast<SP *>(ptr);
            };
          } else {
            using SP = T;
            auto *heap_sp = new SP(val);
            s.tag = FALCON_TYPE_OPAQUE;
            s.value.opaque.ptr = heap_sp;
            if constexpr (std::is_same_v<SP, std::shared_ptr<TupleValue>>) {
              s.value.opaque.type_name = "TupleValue";
            } else if constexpr (std::is_same_v<SP, std::shared_ptr<ArrayValue>>) {
              s.value.opaque.type_name = "ArrayValue";
            } else if constexpr (std::is_same_v<SP, std::shared_ptr<StructInstance>>) {
              if (val && val->is_native()) {
                // Native FFI struct: pack native_handle as shared_ptr<void>*
                // so unpack_results on the engine side can recover it.
                auto *svp = new std::shared_ptr<void>(
                    std::static_pointer_cast<void>(val->native_handle.value()));
                delete heap_sp;
                s.value.opaque.ptr = svp;
                s.value.opaque.type_name = val->type_name.c_str();
                s.value.opaque.deleter = [](void *p) {
                  delete static_cast<std::shared_ptr<void> *>(p);
                };
                return; // deleter already set, skip the generic one below
              }
              s.value.opaque.type_name = "StructInstance";
            } else {
              s.value.opaque.type_name = "unknown_opaque";
            }
            s.value.opaque.deleter = [](void *p) {
              delete static_cast<SP *>(p);
            };
          }
        },
        result[i]);
  }
}

inline void pack_single(RuntimeValue val, FalconResultSlot *slots,
                        int32_t *out_count) {
  pack_results(FunctionResult{std::move(val)}, slots, 1, out_count);
}

} // namespace wrapper
} // namespace falcon::typing::ffi
