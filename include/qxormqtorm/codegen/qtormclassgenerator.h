#pragma once

#include "../database/schemareader.h"
#include <QString>

namespace QxOrmQtOrm {

/**
 * @brief Generates QtOrm-style C++ classes from database schema
 */
class QtOrmClassGenerator
{
public:
    struct GenerationOptions
    {
        QString namespacePrefix;
        QString classPrefix;
        QString classSuffix;
        bool generateHeader = true;
        bool generateQProperty = true;
        bool generateQInvokable = true;
        bool generateSignals = true;
        QString headerGuardPrefix = "GENERATED";
        QString indent = "    ";
    };
    
    explicit QtOrmClassGenerator(const GenerationOptions& options = GenerationOptions{});
    
    /**
     * @brief Generate C++ classes for all tables in the schema
     * @param schema Database schema to generate from
     * @param outputDir Directory to write .h and .cpp files
     * @return true if successful
     */
    bool generateClasses(const DatabaseSchema& schema, const QString& outputDir);
    
    /**
     * @brief Generate C++ class for a single table
     * @param table Table definition to generate from
     * @param outputDir Directory to write .h and .cpp files
     * @return true if successful
     */
    bool generateClass(const TableDefinition& table, const QString& outputDir);
    
    /**
     * @brief Generate header content for a table
     * @param table Table definition
     * @return Generated header file content
     */
    QString generateHeaderContent(const TableDefinition& table);
    
    /**
     * @brief Generate source content for a table
     * @param table Table definition
     * @return Generated source file content
     */
    QString generateSourceContent(const TableDefinition& table);
    
private:
    QString generateClassName(const QString& tableName);
    QString generatePropertyName(const QString& columnName);
    QString generateGetterName(const QString& columnName);
    QString generateSetterName(const QString& columnName);
    QString generateSignalName(const QString& columnName);
    QString generateMemberName(const QString& columnName);
    
    QString generateQPropertyDeclaration(const ColumnDefinition& column);
    QString generatePropertyAccessors(const ColumnDefinition& column);
    QString generatePropertySignal(const ColumnDefinition& column);
    QString generateMemberVariable(const ColumnDefinition& column);
    QString generateQOrmPropertyMacro(const ColumnDefinition& column);
    
    GenerationOptions m_options;
};

} // namespace QxOrmQtOrm