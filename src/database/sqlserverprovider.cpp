#include "qxormqtorm/database/sqlserverprovider.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QUuid>

namespace QxOrmQtOrm {

SqlServerProvider::SqlServerProvider()
{
    m_connectionName = QUuid::createUuid().toString();
}

SqlServerProvider::~SqlServerProvider()
{
    disconnectFromDatabase();
}

DatabaseSchema SqlServerProvider::readSchema(const QString& connectionString)
{
    DatabaseSchema schema;
    
    if (!connectToDatabase(connectionString)) {
        return schema;
    }
    
    // Get database name
    QSqlQuery query(m_database);
    query.exec("SELECT DB_NAME()");
    if (query.next()) {
        schema.databaseName = query.value(0).toString();
    }
    
    // Read all tables
    schema.tables = readTableDefinitions();
    
    // Read foreign key relationships
    readForeignKeyInfo(schema.tables);
    
    disconnectFromDatabase();
    return schema;
}

DatabaseSchema SqlServerProvider::readTables(const QString& connectionString, const QStringList& tableNames)
{
    DatabaseSchema schema;
    
    if (!connectToDatabase(connectionString)) {
        return schema;
    }
    
    // Get database name
    QSqlQuery query(m_database);
    query.exec("SELECT DB_NAME()");
    if (query.next()) {
        schema.databaseName = query.value(0).toString();
    }
    
    // Read specified tables
    schema.tables = readTableDefinitions(tableNames);
    
    // Read foreign key relationships
    readForeignKeyInfo(schema.tables);
    
    disconnectFromDatabase();
    return schema;
}

QString SqlServerProvider::mapToQtType(const QString& dbType)
{
    QString lowerType = dbType.toLower();
    
    // Integer types
    if (lowerType == "tinyint") return "quint8";
    if (lowerType == "smallint") return "qint16";
    if (lowerType == "int" || lowerType == "integer") return "qint32";
    if (lowerType == "bigint") return "qint64";
    
    // Floating point types
    if (lowerType == "real") return "float";
    if (lowerType == "float" || lowerType == "double precision") return "double";
    if (lowerType == "decimal" || lowerType == "numeric" || lowerType == "money" || lowerType == "smallmoney") {
        return "double"; // Could use QDecimal in the future
    }
    
    // Boolean
    if (lowerType == "bit") return "bool";
    
    // Date/Time types
    if (lowerType == "datetime" || lowerType == "datetime2" || lowerType == "smalldatetime") return "QDateTime";
    if (lowerType == "date") return "QDate";
    if (lowerType == "time") return "QTime";
    if (lowerType == "datetimeoffset") return "QDateTime";
    if (lowerType == "timestamp") return "QByteArray";
    
    // Binary types
    if (lowerType == "binary" || lowerType == "varbinary" || lowerType == "image") return "QByteArray";
    
    // GUID
    if (lowerType == "uniqueidentifier") return "QUuid";
    
    // String types (default)
    return "QString";
}

bool SqlServerProvider::connectToDatabase(const QString& connectionString)
{
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
    
    m_database = QSqlDatabase::addDatabase("QODBC", m_connectionName);
    
    // Parse connection string or use it directly
    if (connectionString.contains("Driver=") || connectionString.contains("DRIVER=")) {
        m_database.setDatabaseName(connectionString);
    } else {
        // Simple format: "Server=server;Database=db;..."
        m_database.setDatabaseName("DRIVER={ODBC Driver 17 for SQL Server};" + connectionString);
    }
    
    if (!m_database.open()) {
        qWarning() << "Failed to connect to SQL Server:" << m_database.lastError().text();
        return false;
    }
    
    return true;
}

void SqlServerProvider::disconnectFromDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
    
    if (QSqlDatabase::contains(m_connectionName)) {
        QSqlDatabase::removeDatabase(m_connectionName);
    }
}

QVector<TableDefinition> SqlServerProvider::readTableDefinitions(const QStringList& tableFilter)
{
    QVector<TableDefinition> tables;
    
    QString sql = R"(
        SELECT 
            t.TABLE_SCHEMA,
            t.TABLE_NAME,
            CAST(ep.value AS NVARCHAR(4000)) AS TABLE_COMMENT
        FROM INFORMATION_SCHEMA.TABLES t
        LEFT JOIN sys.tables st ON st.name = t.TABLE_NAME
        LEFT JOIN sys.extended_properties ep ON ep.major_id = st.object_id 
            AND ep.minor_id = 0 AND ep.name = 'MS_Description'
        WHERE t.TABLE_TYPE = 'BASE TABLE'
    )";
    
    if (!tableFilter.isEmpty()) {
        QStringList quotedNames;
        for (const QString& name : tableFilter) {
            quotedNames << "'" + name + "'";
        }
        sql += " AND t.TABLE_NAME IN (" + quotedNames.join(",") + ")";
    }
    
    sql += " ORDER BY t.TABLE_SCHEMA, t.TABLE_NAME";
    
    QSqlQuery query(m_database);
    if (!query.exec(sql)) {
        qWarning() << "Failed to read table definitions:" << query.lastError().text();
        return tables;
    }
    
    while (query.next()) {
        TableDefinition table;
        table.schema = query.value("TABLE_SCHEMA").toString();
        table.name = query.value("TABLE_NAME").toString();
        table.comment = query.value("TABLE_COMMENT").toString();
        
        // Read column definitions
        table.columns = readColumnDefinitions(table.name, table.schema);
        
        // Extract primary key columns
        for (const auto& column : table.columns) {
            if (column.isPrimaryKey) {
                table.primaryKeyColumns << column.name;
            }
        }
        
        tables.append(table);
    }
    
    return tables;
}

QVector<ColumnDefinition> SqlServerProvider::readColumnDefinitions(const QString& tableName, const QString& schemaName)
{
    QVector<ColumnDefinition> columns;
    
    QString sql = R"(
        SELECT 
            c.COLUMN_NAME,
            c.DATA_TYPE,
            c.CHARACTER_MAXIMUM_LENGTH,
            c.IS_NULLABLE,
            c.COLUMN_DEFAULT,
            CAST(ep.value AS NVARCHAR(4000)) AS COLUMN_COMMENT,
            CASE WHEN tc.CONSTRAINT_TYPE = 'PRIMARY KEY' THEN 1 ELSE 0 END AS IS_PRIMARY_KEY,
            CASE WHEN c.COLUMNPROPERTY(OBJECT_ID(c.TABLE_SCHEMA + '.' + c.TABLE_NAME), c.COLUMN_NAME, 'IsIdentity') = 1 
                 THEN 1 ELSE 0 END AS IS_IDENTITY
        FROM INFORMATION_SCHEMA.COLUMNS c
        LEFT JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE kcu ON 
            c.TABLE_SCHEMA = kcu.TABLE_SCHEMA AND 
            c.TABLE_NAME = kcu.TABLE_NAME AND 
            c.COLUMN_NAME = kcu.COLUMN_NAME
        LEFT JOIN INFORMATION_SCHEMA.TABLE_CONSTRAINTS tc ON 
            kcu.CONSTRAINT_SCHEMA = tc.CONSTRAINT_SCHEMA AND 
            kcu.CONSTRAINT_NAME = tc.CONSTRAINT_NAME
        LEFT JOIN sys.columns sc ON sc.object_id = OBJECT_ID(c.TABLE_SCHEMA + '.' + c.TABLE_NAME) 
            AND sc.name = c.COLUMN_NAME
        LEFT JOIN sys.extended_properties ep ON ep.major_id = sc.object_id 
            AND ep.minor_id = sc.column_id AND ep.name = 'MS_Description'
        WHERE c.TABLE_SCHEMA = ? AND c.TABLE_NAME = ?
        ORDER BY c.ORDINAL_POSITION
    )";
    
    QSqlQuery query(m_database);
    query.prepare(sql);
    query.addBindValue(schemaName);
    query.addBindValue(tableName);
    
    if (!query.exec()) {
        qWarning() << "Failed to read column definitions for" << schemaName + "." + tableName << ":" << query.lastError().text();
        return columns;
    }
    
    while (query.next()) {
        ColumnDefinition column;
        column.name = query.value("COLUMN_NAME").toString();
        column.dataType = query.value("DATA_TYPE").toString();
        column.qtType = mapToQtType(column.dataType);
        column.isPrimaryKey = query.value("IS_PRIMARY_KEY").toBool();
        column.isAutoIncrement = query.value("IS_IDENTITY").toBool();
        column.isNullable = (query.value("IS_NULLABLE").toString() == "YES");
        column.maxLength = query.value("CHARACTER_MAXIMUM_LENGTH").toInt();
        column.defaultValue = query.value("COLUMN_DEFAULT").toString();
        column.comment = query.value("COLUMN_COMMENT").toString();
        
        columns.append(column);
    }
    
    return columns;
}

void SqlServerProvider::readForeignKeyInfo(QVector<TableDefinition>& tables)
{
    QString sql = R"(
        SELECT 
            fk.name AS FK_NAME,
            tp.name AS PARENT_TABLE,
            cp.name AS PARENT_COLUMN,
            tr.name AS REFERENCED_TABLE,
            cr.name AS REFERENCED_COLUMN
        FROM sys.foreign_keys fk
        INNER JOIN sys.foreign_key_columns fkc ON fkc.constraint_object_id = fk.object_id
        INNER JOIN sys.tables tp ON tp.object_id = fk.parent_object_id
        INNER JOIN sys.columns cp ON cp.object_id = tp.object_id AND cp.column_id = fkc.parent_column_id
        INNER JOIN sys.tables tr ON tr.object_id = fk.referenced_object_id
        INNER JOIN sys.columns cr ON cr.object_id = tr.object_id AND cr.column_id = fkc.referenced_column_id
    )";
    
    QSqlQuery query(m_database);
    if (!query.exec(sql)) {
        qWarning() << "Failed to read foreign key information:" << query.lastError().text();
        return;
    }
    
    while (query.next()) {
        QString parentTable = query.value("PARENT_TABLE").toString();
        QString parentColumn = query.value("PARENT_COLUMN").toString();
        QString referencedTable = query.value("REFERENCED_TABLE").toString();
        QString referencedColumn = query.value("REFERENCED_COLUMN").toString();
        
        // Find the parent table and column
        for (auto& table : tables) {
            if (table.name == parentTable) {
                auto* column = table.findColumn(parentColumn);
                if (column) {
                    column->isForeignKey = true;
                    column->referencedTable = referencedTable;
                    column->referencedColumn = referencedColumn;
                }
                break;
            }
        }
    }
}

} // namespace QxOrmQtOrm