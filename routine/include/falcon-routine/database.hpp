#pragma once
#include "falcon-database/DatabaseConnection.hpp"

namespace falcon::routine {

/**
 * @brief Alias for ReadOnlyDatabaseConnection for backward compatibility.
 *
 * The base ReadOnlyDatabaseConnection is now lazy by default,
 * so this alias maintains API compatibility with existing code.
 */
using LazyReadOnlyDatabaseConnection =
    falcon::database::ReadOnlyDatabaseConnection;

} // namespace falcon::routine
