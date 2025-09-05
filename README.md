# QxOrm + QtOrm Integration

This project combines the power of QxOrm's code generation capabilities with QtOrm's modern C++ API design to create tools for bidirectional ORM/database integration, with a focus on SQL Server support.

## Features

- **Database to ORM Generation**: Import SQL Server database schemas and generate QtOrm-style C++ classes
- **ORM to Database Generation**: Generate SQL Server DDL scripts from QtOrm C++ class definitions
- **Modern C++ API**: Based on QtOrm's clean, LINQ-like query interface
- **SQL Server Focus**: Primary support for Microsoft SQL Server with extensible architecture

## Tools

### db2orm
Command-line tool to generate QtOrm C++ classes from an existing SQL Server database.

```bash
db2orm --connection "Server=localhost;Database=MyDB;..." --output ./generated/
```

### orm2db  
Command-line tool to generate SQL Server DDL scripts from QtOrm C++ class definitions.

```bash
orm2db --input ./src/models/ --output ./schema.sql
```

## Building

Requires Qt 6.0+ and CMake 3.16+:

```bash
mkdir build && cd build
cmake ..
make
```

## Configuration

Uses JSON-based configuration similar to QtOrm:

```json
{
    "provider": "sqlserver",
    "sqlserver": {
        "connectionString": "Server=localhost;Database=MyDB;Trusted_Connection=yes;",
        "schemaMode": "update"
    }
}
```

## License

MIT License - see LICENSE file for details.