/**
 * @file FfiHelpers.hpp
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

#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/falcon_ffi.h"
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

// Pull in the falcon_core SP types so we can match type_name strings
#include <falcon_core/autotuner_interfaces/names/Gname.hpp>
#include <falcon_core/math/Quantity.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <falcon_core/physics/device_structures/Connections.hpp>

namespace falcon::autotuner::ffi {

// ============================================================================
// Helpers used by the ENGINE side (packing params, unpacking results)
// ============================================================================
namespace engine {

/**
 * Pack a ParameterMap into a flat array of FalconParamEntry.
 * The returned vector + all its string/key storage is valid as long as
 * the original ParameterMap is alive AND the returned `key_storage` /
 * `str_storage` vectors are alive.  Call this immediately before the
 * FFI call and discard afterwards.
 */
struct PackedParams {
  std::vector<FalconParamEntry> entries;
  // Keep string data alive for the duration of the FFI call
  std::vector<std::string> key_storage;
  std::vector<std::string> str_storage;
  // Keep shared_ptr heap allocations alive until engine reads results
  // (actually the shared_ptr heap nodes are owned by opaque.ptr — their
  //  lifetime is managed by the deleter registered in the entry)
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
            packed.str_storage.push_back(val.message);
            e.tag = FALCON_TYPE_ERROR;
            e.value.str.ptr = packed.str_storage.back().c_str();
            e.value.str.len = (int32_t)packed.str_storage.back().size();
          } else {
            // Opaque type: heap-allocate a copy of the shared_ptr so the
            // raw pointer survives past the ParameterMap scope if needed.
            // The deleter just does `delete static_cast<decltype(&val)>(ptr)`.
            using SP = T;
            auto *heap_sp = new SP(val);
            e.tag = FALCON_TYPE_OPAQUE;
            e.value.opaque.ptr = heap_sp;
            // Use mangled-free type_name convention (readable strings)
            if constexpr (std::is_same_v<SP,
                                         falcon_core::physics::
                                             device_structures::ConnectionSP>) {
              e.value.opaque.type_name = "ConnectionSP";
            } else if constexpr (std::is_same_v<
                                     SP,
                                     falcon_core::physics::device_structures::
                                         ConnectionsSP>) {
              e.value.opaque.type_name = "ConnectionsSP";
            } else if constexpr (std::is_same_v<
                                     SP, falcon_core::math::QuantitySP>) {
              e.value.opaque.type_name = "QuantitySP";
            } else if constexpr (std::is_same_v<
                                     SP, falcon_core::autotuner_interfaces::
                                             names::GnameSP>) {
              e.value.opaque.type_name = "GnameSP";
            } else if constexpr (std::is_same_v<SP,
                                                std::shared_ptr<TupleValue>>) {
              e.value.opaque.type_name = "TupleValue";
            } else if constexpr (std::is_same_v<
                                     SP, std::shared_ptr<StructInstance>>) {
              e.value.opaque.type_name = "StructInstance";
            } else {
              e.value.opaque.type_name = "unknown_opaque";
            }
            e.value.opaque.deleter = [](void *p) {
              delete static_cast<SP *>(p);
            };
          }
        },
        v);

    packed.entries.push_back(e);
  }
  return packed;
}

/**
 * Unpack FalconResultSlot[] into FunctionResult.
 * Frees string/opaque memory allocated by the wrapper.
 *
 * For opaque types: the wrapper heap-allocates a shared_ptr<T> and passes
 * its address. The engine reconstructs the RuntimeValue by reading the
 * shared_ptr via the known type_name, then calls the deleter.
 */
inline FunctionResult unpack_results(FalconResultSlot *slots, int32_t count) {
  FunctionResult result;
  result.reserve((size_t)count);

  for (int32_t i = 0; i < count; ++i) {
    FalconResultSlot &s = slots[i];
    switch (s.tag) {
    case FALCON_TYPE_NIL:
      result.push_back(nullptr);
      break;
    case FALCON_TYPE_INT:
      result.push_back(s.value.int_val);
      break;
    case FALCON_TYPE_FLOAT:
      result.push_back(s.value.float_val);
      break;
    case FALCON_TYPE_BOOL:
      result.push_back(s.value.bool_val != 0);
      break;
    case FALCON_TYPE_STRING: {
      std::string str(s.value.str.ptr, (size_t)s.value.str.len);
      ::free(s.value.str.ptr); // wrapper must use malloc/strdup
      result.push_back(std::move(str));
      break;
    }
    case FALCON_TYPE_ERROR: {
      std::string msg(s.value.error.message);
      bool fatal = s.value.error.is_fatal != 0;
      ::free(s.value.error.message);
      result.push_back(ErrorObject{std::move(msg), fatal});
      break;
    }
    case FALCON_TYPE_OPAQUE: {
      // Reconstruct the RuntimeValue from the type_name
      std::string tn = s.value.opaque.type_name ? s.value.opaque.type_name : "";
      void *ptr = s.value.opaque.ptr;
      auto del = s.value.opaque.deleter;

      RuntimeValue rv = nullptr;
      if (tn == "TupleValue") {
        using SP = std::shared_ptr<TupleValue>;
        rv = *static_cast<SP *>(ptr);
      } else if (tn == "StructInstance") {
        using SP = std::shared_ptr<StructInstance>;
        rv = *static_cast<SP *>(ptr);
      } else {
        // Unknown opaque — store as StructInstance with type_name tag
        // so it doesn't get lost entirely
        rv = nullptr;
      }
      if (del)
        del(ptr);
      result.push_back(std::move(rv));
      break;
    }
    default:
      result.push_back(nullptr);
      break;
    }
  }
  return result;
}

} // namespace engine

// ============================================================================
// Helpers used by the WRAPPER side (unpacking params, packing results)
// These are included by wrapper .cpp files.
// ============================================================================
namespace wrapper {

/**
 * Reconstruct a ParameterMap from the flat C array.
 * For opaque entries, this re-wraps the heap shared_ptr WITHOUT taking
 * ownership (the engine's deleter will still fire after we return).
 *
 * NOTE: For user-defined struct types (not known to the engine's variant),
 * you must use get_opaque<T>(params, "key") instead (see below).
 */
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
    case FALCON_TYPE_ERROR:
      result[key] = ErrorObject{
          std::string(e.value.str.ptr, (size_t)e.value.str.len), false};
      break;
    case FALCON_TYPE_OPAQUE: {
      std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
      void *ptr = e.value.opaque.ptr;
      // Re-wrap known engine types only; unknown opaque (user structs)
      // stay as opaque — use get_opaque<T>() to extract them.
      if (tn == "StructInstance") {
        using SP = std::shared_ptr<StructInstance>;
        result[key] = *static_cast<SP *>(ptr);
      } else {
        // Unknown user type — leave as nil; use get_opaque<T>() directly
        result[key] = nullptr;
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
 * Use this when the struct type is not known to the engine's RuntimeValue
 * variant (e.g. a local `struct Quantity`).
 *
 * The engine heap-allocated a shared_ptr<T>* and put its address in
 * entry.value.opaque.ptr.  We dereference to get the shared_ptr<T>.
 */
template <typename T>
inline std::shared_ptr<T> get_opaque(const FalconParamEntry *entries,
                                     int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) == 0 &&
        entries[i].tag == FALCON_TYPE_OPAQUE) {
      using SP = std::shared_ptr<T>;
      return *static_cast<SP *>(entries[i].value.opaque.ptr);
    }
  }
  throw std::runtime_error(std::string("FFI: opaque param not found: ") + key);
}

/**
 * Pack a FunctionResult into pre-allocated FalconResultSlot[].
 * Strings are heap-allocated via malloc (engine will free() them).
 * Opaque shared_ptrs are heap-allocated via new SP(...) with a matching
 * deleter (engine will call it).
 */
inline void pack_results(const FunctionResult &result, FalconResultSlot *slots,
                         int32_t capacity, int32_t *out_count) {
  int32_t n = (int32_t)result.size();
  if (n > capacity)
    n = capacity;
  *out_count = n;

  for (int32_t i = 0; i < n; ++i) {
    FalconResultSlot &s = slots[i];
    s = {}; // zero-initialize

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
            s.tag = FALCON_TYPE_ERROR;
            s.value.error.is_fatal = val.is_fatal ? 1 : 0;
            s.value.error.message = (char *)::malloc(val.message.size() + 1);
            std::memcpy(s.value.error.message, val.message.c_str(),
                        val.message.size() + 1);
          } else {
            // Opaque shared_ptr type
            using SP = T;
            auto *heap_sp = new SP(val);
            s.tag = FALCON_TYPE_OPAQUE;
            s.value.opaque.ptr = heap_sp;
            if constexpr (std::is_same_v<SP,
                                         falcon_core::physics::
                                             device_structures::ConnectionSP>) {
              s.value.opaque.type_name = "ConnectionSP";
            } else if constexpr (std::is_same_v<
                                     SP,
                                     falcon_core::physics::device_structures::
                                         ConnectionsSP>) {
              s.value.opaque.type_name = "ConnectionsSP";
            } else if constexpr (std::is_same_v<
                                     SP, falcon_core::math::QuantitySP>) {
              s.value.opaque.type_name = "QuantitySP";
            } else if constexpr (std::is_same_v<
                                     SP, falcon_core::autotuner_interfaces::
                                             names::GnameSP>) {
              s.value.opaque.type_name = "GnameSP";
            } else if constexpr (std::is_same_v<SP,
                                                std::shared_ptr<TupleValue>>) {
              s.value.opaque.type_name = "TupleValue";
            } else if constexpr (std::is_same_v<
                                     SP, std::shared_ptr<StructInstance>>) {
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

/**
 * Convenience: pack a single-value result.
 */
inline void pack_single(RuntimeValue val, FalconResultSlot *slots,
                        int32_t *out_count) {
  pack_results(FunctionResult{std::move(val)}, slots, 1, out_count);
}

} // namespace wrapper
} // namespace falcon::autotuner::ffi
