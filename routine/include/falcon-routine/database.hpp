#pragma once
#include "falcon-database/DatabaseConnection.hpp"

namespace falcon::routine {

/**
 * @brief Alias for ReadOnlyDatabaseConnection but exposed within the routine
 * library.
 */
using LazyReadOnlyDatabaseConnection =
    falcon::database::ReadOnlyDatabaseConnection;

} // namespace falcon::routine
