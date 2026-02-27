#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
namespace {
falcon::autotuner::FunctionResult
STRUCTConnectionNewPlungerGate(falcon::autotuner::ParameterMap params) {
  std::string a = std::get<std::string>(params.at("name"));
  ConnectionSP conn = Connection::PlungerGate(a);
  return falcon::autotuner::FunctionResult{conn};
}

falcon::autotuner::FunctionResult
STRUCTConnectionNewBarrierGate(falcon::autotuner::ParameterMap params) {
  std::string a = std::get<std::string>(params.at("name"));
  ConnectionSP conn = Connection::BarrierGate(a);
  return falcon::autotuner::FunctionResult{conn};
}

falcon::autotuner::FunctionResult
STRUCTConnectionNewReservoirGate(falcon::autotuner::ParameterMap params) {
  std::string a = std::get<std::string>(params.at("name"));
  ConnectionSP conn = Connection::ReservoirGate(a);
  return falcon::autotuner::FunctionResult{conn};
}

falcon::autotuner::FunctionResult
STRUCTConnectionNewScreeningGate(falcon::autotuner::ParameterMap params) {
  std::string a = std::get<std::string>(params.at("name"));
  ConnectionSP conn = Connection::ScreeningGate(a);
  return falcon::autotuner::FunctionResult{conn};
}

falcon::autotuner::FunctionResult
STRUCTConnectionNewOhmic(falcon::autotuner::ParameterMap params) {
  std::string a = std::get<std::string>(params.at("name"));
  ConnectionSP conn = Connection::Ohmic(a);
  return falcon::autotuner::FunctionResult{conn};
}

falcon::autotuner::FunctionResult
STRUCTConnectionName(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->name()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionType(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->type()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsDotGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_dot_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsBarrierGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_barrier_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsPlungerGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_plunger_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsScreeningGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_screening_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsReservoirGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_reservoir_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsOhmic(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_ohmic()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionIsGate(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  return falcon::autotuner::FunctionResult{conn->is_gate()};
}

falcon::autotuner::FunctionResult
STRUCTConnectionEqual(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  ConnectionSP other = std::get<ConnectionSP>(params.at("other"));
  return falcon::autotuner::FunctionResult{conn == other};
}

falcon::autotuner::FunctionResult
STRUCTConnectionNotEqual(falcon::autotuner::ParameterMap params) {
  ConnectionSP conn = std::get<ConnectionSP>(params.at("this"));
  ConnectionSP other = std::get<ConnectionSP>(params.at("other"));
  return falcon::autotuner::FunctionResult{conn != other};
}
} // namespace
