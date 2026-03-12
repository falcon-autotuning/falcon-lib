# falcon/database

Database connectivity and query interface for the Falcon autotuning system. Provides access to PostgreSQL-backed device characteristic storage with full CRUD operations and snapshot management.

---

## ⚠️ Prerequisites - IMPORTANT

**This library requires an external PostgreSQL database to function.** Unlike other Falcon libraries, the database package connects to a live database service that must be running before you can use this library or run its tests.

### Required Setup Before Testing

1. **Install PostgreSQL** (if not already installed)
2. **Start the Falcon database service** using the `falcon-db-cli` tool
3. **Initialize the database schema**
4. **Set the connection environment variable**

See the [Database Setup](#database-setup) section below for detailed instructions.

---

## Installation

```fal
import "libs/database/database.fal";
```

---

## Database Setup

The Falcon database uses PostgreSQL as its backend. Before running tests or using the database library, you must:

### 1. Start PostgreSQL Service

Ensure PostgreSQL is installed and running on your system:

```bash
# On Ubuntu/Debian
sudo systemctl start postgresql

# On macOS with Homebrew
brew services start postgresql

# Or using Docker
docker run -d --name falcon-postgres -p 5432:5432 \
  -e POSTGRES_PASSWORD=falcon \
  -e POSTGRES_DB=falcon_db \
  postgres:latest
```

### 2. Set Connection Environment Variable

The library reads the database URL from the `FALCON_DATABASE_URL` environment variable:

```bash
export FALCON_DATABASE_URL="postgresql://username:password@localhost:5432/falcon_db"
```

Replace `username`, `password`, and `falcon_db` with your actual PostgreSQL credentials and database name.

### 3. Initialize Database Schema

Use the `falcon-db-cli` tool to create the required database tables:

```bash
falcon-db-cli init
```

This command is **idempotent** — it's safe to run multiple times. It will create the `device_characteristics` table and any required indexes.

### 4. Verify Connection

Test that everything is working:

```bash
falcon-db-cli help
```

For more information about the CLI tool, see `/database/docs/CLI.md` in the repository.

---

## Running Tests

**⚠️ CRITICAL:** The test suite assumes:

1. A PostgreSQL database is running and accessible
2. `FALCON_DATABASE_URL` environment variable is set correctly
3. The database schema has been initialized with `falcon-db-cli init`
4. **Data persists between test runs** — tests are designed to be non-destructive

### Test Execution

```bash
# Make sure database is running and initialized first!
export FALCON_DATABASE_URL="postgresql://username:password@localhost:5432/falcon_db"
falcon-db-cli init

# Now run the tests
cd libs/database/tests
falcon-test run_tests.fal
```

### Test Data Persistence

Unlike in-memory tests, **database tests have side effects**:

- **INSERT operations persist** — data added by tests remains in the database
- **DELETE operations are permanent** — removed data does not automatically restore
- **CLEAR operations are destructive** — some admin tests are skipped to prevent data loss

The test suite is designed to be **non-destructive by default**:

- Uses queries that don't rely on specific pre-existing data
- Tests read operations extensively without requiring specific fixtures
- Skips dangerous operations like `ClearAll()` that would delete all data
- Uses unique identifiers for test data to avoid conflicts

### Cleaning Up Test Data

If you need to reset the database to a clean state:

```bash
# Export a backup first (recommended)
falcon-db-cli snapshot export backup_$(date +%Y%m%d).json

# Clear all data (DESTRUCTIVE - cannot be undone!)
falcon-db-cli snapshot import empty.json --clear

# Or drop and recreate the schema
falcon-db-cli init
```

---

## API Overview

### Database Struct

Main database connection and query interface.

```fal
Database db = Database.New();
```

#### Connection Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `New()` | `Database` | Create a new database connection (lazy initialization) |
| `TestConnection()` | `bool ok` | Test if database is reachable |
| `IsConnected()` | `bool connected` | Check if connection has been established |

#### Read Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `GetByName(string name)` | `(DeviceCharacteristic, bool found)` | Retrieve single entry by name |
| `GetAll()` | `DeviceCharacteristicList` | Retrieve all device characteristics |
| `GetByHashRange(string start, string end)` | `DeviceCharacteristicList` | Query by hash range |
| `GetByQuery(DeviceCharacteristicQuery q)` | `DeviceCharacteristicList` | Query with custom filters |
| `Count()` | `int count` | Get total number of records |

#### Write Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `Insert(DeviceCharacteristic dchar)` | `()` | Insert a new device characteristic |
| `DeleteByName(string name)` | `bool deleted` | Delete by name (returns true if found) |
| `DeleteByHash(string hash)` | `int count` | Delete all matching hash (returns count deleted) |

#### Admin Methods

| Method | Returns | Description |
|--------|---------|-------------|
| `InitializeSchema()` | `()` | Create database tables (idempotent) |
| `ClearAll()` | `()` | **DANGEROUS:** Delete all records |
| `ExportToJson(string filename)` | `()` | Export all data to JSON snapshot |
| `ImportFromJson(string filename, bool clear)` | `()` | Import from JSON snapshot |

---

### DeviceCharacteristic Struct

Represents a single device characteristic record.

```fal
string scope = dchar.GetScope();
string name = dchar.GetName();
string hash = dchar.GetHash();
int time = dchar.GetTime();
string state_val = dchar.GetState();
string unit_name = dchar.GetUnitName();
string json_str = dchar.ToJson();
```

---

### DeviceCharacteristicList Struct

Collection of device characteristics returned from queries.

```fal
int count = list.Size();
DeviceCharacteristic first = list.Get(0);
```

---

### DeviceCharacteristicQuery Struct

Builder pattern for constructing database queries.

```fal
DeviceCharacteristicQuery q = DeviceCharacteristicQuery.New();
q = q.SetScope("gpu")
     .SetName("device_01")
     .SetState("active")
     .SetHash("abc123")
     .SetUnitName("unit_0");

DeviceCharacteristicList results = db.GetByQuery(q);
```

All `Set*` methods return the query object for method chaining.

---

## Example Usage

### Basic Connection and Query

```fal
import "libs/database/database.fal";

Database db = Database.New();

// Test connection
bool ok = db.TestConnection();
if (ok) {
    int total = db.Count();
    // ... use database
}
```

### Query by Name

```fal
Database db = Database.New();
string target_name = "my_device";

DeviceCharacteristic dchar;
bool found;
(dchar, found) = db.GetByName(target_name);

if (found) {
    string scope = dchar.GetScope();
    string hash = dchar.GetHash();
}
```

### Complex Query with Filters

```fal
Database db = Database.New();

DeviceCharacteristicQuery q = DeviceCharacteristicQuery.New();
q = q.SetScope("gpu_compute")
     .SetState("calibrated");

DeviceCharacteristicList results = db.GetByQuery(q);
int count = results.Size();

int i = 0;
while (i < count) {
    DeviceCharacteristic item = results.Get(i);
    string name = item.GetName();
    // Process item...
    i = i + 1;
}
```

### Snapshot Export/Import

```fal
Database db = Database.New();

// Export current state
db.ExportToJson("backup_2026_03_12.json");

// Import (without clearing existing data)
db.ImportFromJson("previous_backup.json", false);

// Import and replace all data (DESTRUCTIVE!)
db.ImportFromJson("clean_slate.json", true);
```

---

## Troubleshooting

### "Connection failed" errors

- Verify PostgreSQL is running: `pg_isready`
- Check `FALCON_DATABASE_URL` is set correctly
- Ensure database exists: `psql -l`
- Check firewall settings for port 5432

### "Table does not exist" errors

- Run `falcon-db-cli init` to create the schema

### Tests failing with "no data found"

- The test suite expects an empty or minimal database
- Some tests skip if no data is present
- Consider running with a test database separate from production

### Permission errors

- Ensure the database user has `CREATE`, `INSERT`, `SELECT`, `DELETE` privileges
- Check PostgreSQL `pg_hba.conf` for connection authentication rules

---

## Environment Variables

| Variable | Required | Description |
|----------|----------|-------------|
| `FALCON_DATABASE_URL` | **Yes** | PostgreSQL connection string (see above) |

---

## See Also

- **Database CLI Documentation:** `/database/docs/CLI.md`
- **Testing Framework:** `libs/testing/README.md`
- **Connection Implementation:** `libs/database/database-wrapper.cpp`

---

## Maintainer

`falcon-devs`

For issues related to database connectivity or schema, refer to the main Falcon issue tracker.
