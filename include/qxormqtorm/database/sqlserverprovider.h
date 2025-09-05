#pragma once

#include "schemareader.h"
#include <QSqlDatabase>

namespace QxOrmQtOrm {

/**
 * @brief SQL Server specific implementation of SchemaReader
 */
class SqlServerProvider : public SchemaReader
{
public:
    SqlServerProvider();
    ~SqlServerProvider() override;
    
    // SchemaReader interface
    DatabaseSchema readSchema(const QString& connectionString) override;
    DatabaseSchema readTables(const QString& connectionString, const QStringList& tableNames) override;
    
protected:
    QString mapToQtType(const QString& dbType) override;
    
private:
    bool connectToDatabase(const QString& connectionString);
    void disconnectFromDatabase();
    
    QVector<TableDefinition> readTableDefinitions(const QStringList& tableFilter = QStringList());
    QVector<ColumnDefinition> readColumnDefinitions(const QString& tableName, const QString& schemaName = "dbo");
    void readForeignKeyInfo(QVector<TableDefinition>& tables);
    
    QString m_connectionName;
    QSqlDatabase m_database;
};

} // namespace QxOrmQtOrm