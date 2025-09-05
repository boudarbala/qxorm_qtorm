# Basic Example

This example shows how to use the QxOrm+QtOrm integration tools with a simple SQL Server database.

## Configuration

Example configuration file (`config.json`):

```json
{
    "provider": "sqlserver",
    "verbose": true,
    "namespacePrefix": "Models",
    "classPrefix": "",
    "classSuffix": "",
    "sqlserver": {
        "connectionString": "Server=localhost;Database=TestDB;Trusted_Connection=yes;",
        "schemaMode": "update"
    }
}
```

## Usage

### Generate QtOrm classes from database:

```bash
../tools/db2orm --config config.json --output ./generated/
```

### Generate SQL DDL from QtOrm classes:

```bash
../tools/orm2db --input ./src/ --output schema.sql --verbose
```

## Sample Database Schema

The example assumes a simple database with tables like:

- Users (id, username, email, created_at)
- Posts (id, title, content, user_id, created_at)
- Comments (id, content, post_id, user_id, created_at)