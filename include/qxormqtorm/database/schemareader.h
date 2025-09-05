#pragma once

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVector>

namespace QxOrmQtOrm {

/**
 * @brief Represents a database column definition
 */
struct ColumnDefinition
{
    QString name;
    QString dataType;
    QString qtType;          // Mapped Qt type (QString, int, etc.)
    bool isPrimaryKey = false;
    bool isAutoIncrement = false;
    bool isNullable = true;
    int maxLength = -1;
    QString defaultValue;
    QString comment;
    
    // For foreign keys
    bool isForeignKey = false;
    QString referencedTable;
    QString referencedColumn;
};

/**
 * @brief Represents a database table definition
 */
struct TableDefinition
{
    QString name;
    QString schema = "dbo";
    QString comment;
    QVector<ColumnDefinition> columns;
    QStringList primaryKeyColumns;
    
    // Find column by name
    const ColumnDefinition* findColumn(const QString& name) const;
    ColumnDefinition* findColumn(const QString& name);
};

/**
 * @brief Represents a complete database schema
 */
struct DatabaseSchema
{
    QString databaseName;
    QVector<TableDefinition> tables;
    
    // Find table by name
    const TableDefinition* findTable(const QString& name) const;
    TableDefinition* findTable(const QString& name);
};

/**
 * @brief Abstract base class for reading database schemas
 */
class SchemaReader
{
public:
    virtual ~SchemaReader() = default;
    
    /**
     * @brief Read the complete database schema
     * @param connectionString Database connection string
     * @return DatabaseSchema object with all tables and columns
     */
    virtual DatabaseSchema readSchema(const QString& connectionString) = 0;
    
    /**
     * @brief Read schema for specific tables only
     * @param connectionString Database connection string
     * @param tableNames List of table names to read
     * @return DatabaseSchema object with specified tables
     */
    virtual DatabaseSchema readTables(const QString& connectionString, const QStringList& tableNames) = 0;
    
protected:
    /**
     * @brief Map database type to Qt type
     * @param dbType Database-specific type name
     * @return Corresponding Qt type name
     */
    virtual QString mapToQtType(const QString& dbType) = 0;
};

} // namespace QxOrmQtOrm