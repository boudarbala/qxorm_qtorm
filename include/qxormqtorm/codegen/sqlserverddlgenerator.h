#pragma once

#include "../database/schemareader.h"
#include <QString>

namespace QxOrmQtOrm {

/**
 * @brief Generates SQL Server DDL from database schema or C++ classes
 */
class SqlServerDdlGenerator
{
public:
    struct DdlOptions
    {
        bool includeDropStatements = false;
        bool includeIndexes = true;
        bool includeForeignKeys = true;
        bool includeComments = true;
        QString schemaName = "dbo";
        QString indent = "    ";
    };
    
    explicit SqlServerDdlGenerator(const DdlOptions& options = DdlOptions{});
    
    /**
     * @brief Generate DDL for entire database schema
     * @param schema Database schema
     * @return Complete DDL script
     */
    QString generateDdl(const DatabaseSchema& schema);
    
    /**
     * @brief Generate CREATE TABLE statement for a single table
     * @param table Table definition
     * @return CREATE TABLE DDL
     */
    QString generateCreateTable(const TableDefinition& table);
    
    /**
     * @brief Generate DROP TABLE statement for a single table
     * @param table Table definition
     * @return DROP TABLE DDL
     */
    QString generateDropTable(const TableDefinition& table);
    
    /**
     * @brief Generate ALTER TABLE statements for foreign keys
     * @param tables All tables (for foreign key references)
     * @return ALTER TABLE DDL for foreign keys
     */
    QString generateForeignKeys(const QVector<TableDefinition>& tables);
    
private:
    QString mapQtTypeToSqlServer(const QString& qtType, int maxLength = -1);
    QString generateColumnDefinition(const ColumnDefinition& column);
    QString generatePrimaryKeyConstraint(const TableDefinition& table);
    QString escapeIdentifier(const QString& identifier);
    
    DdlOptions m_options;
};

} // namespace QxOrmQtOrm