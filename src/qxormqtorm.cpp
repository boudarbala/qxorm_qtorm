#include "qxormqtorm/qxormqtorm.h"
#include "qxormqtorm/database/sqlserverprovider.h"
#include "qxormqtorm/codegen/qtormclassgenerator.h"
#include "qxormqtorm/codegen/sqlserverddlgenerator.h"
#include "qxormqtorm/codegen/cppparser.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>

namespace QxOrmQtOrm {

Generator::Generator(const Configuration& config)
    : m_config(config)
{
}

bool Generator::generateFromDatabase(const QString& outputDir)
{
    if (m_config.verbose()) {
        qDebug() << "Generating QtOrm classes from database...";
        qDebug() << "Connection string:" << m_config.connectionString();
        qDebug() << "Output directory:" << outputDir;
    }
    
    // Create appropriate schema reader based on provider
    std::unique_ptr<SchemaReader> reader;
    
    switch (m_config.provider()) {
    case Configuration::Provider::SqlServer:
        reader = std::make_unique<SqlServerProvider>();
        break;
    default:
        qWarning() << "Unsupported database provider";
        return false;
    }
    
    // Read database schema
    DatabaseSchema schema = reader->readSchema(m_config.connectionString());
    
    if (schema.tables.isEmpty()) {
        qWarning() << "No tables found in database";
        return false;
    }
    
    if (m_config.verbose()) {
        qDebug() << "Found" << schema.tables.size() << "tables";
        for (const auto& table : schema.tables) {
            qDebug() << " -" << table.name << "(" << table.columns.size() << "columns)";
        }
    }
    
    // Configure class generator
    QtOrmClassGenerator::GenerationOptions genOptions;
    genOptions.namespacePrefix = m_config.namespacePrefix();
    genOptions.classPrefix = m_config.classPrefix();
    genOptions.classSuffix = m_config.classSuffix();
    genOptions.generateHeader = m_config.generateHeader();
    
    QtOrmClassGenerator generator(genOptions);
    
    // Generate classes
    bool success = generator.generateClasses(schema, outputDir);
    
    if (m_config.verbose()) {
        if (success) {
            qDebug() << "Successfully generated classes in" << outputDir;
        } else {
            qDebug() << "Failed to generate some classes";
        }
    }
    
    return success;
}

bool Generator::generateToDatabase(const QString& inputDir, const QString& outputFile)
{
    if (m_config.verbose()) {
        qDebug() << "Generating SQL DDL from QtOrm classes...";
        qDebug() << "Input directory:" << inputDir;
        qDebug() << "Output file:" << outputFile;
    }
    
    // Parse C++ files to extract schema
    CppParser parser;
    DatabaseSchema schema = parser.parseDirectory(inputDir);
    
    if (schema.tables.isEmpty()) {
        qWarning() << "No QtOrm classes found in input directory";
        return false;
    }
    
    if (m_config.verbose()) {
        qDebug() << "Found" << schema.tables.size() << "classes";
        for (const auto& table : schema.tables) {
            qDebug() << " -" << table.name << "(" << table.columns.size() << "properties)";
        }
    }
    
    // Configure DDL generator based on provider
    QString ddl;
    
    switch (m_config.provider()) {
    case Configuration::Provider::SqlServer: {
        SqlServerDdlGenerator::DdlOptions ddlOptions;
        ddlOptions.includeDropStatements = (m_config.schemaMode() == Configuration::SchemaMode::Recreate);
        ddlOptions.includeForeignKeys = true;
        ddlOptions.includeComments = true;
        
        SqlServerDdlGenerator ddlGenerator(ddlOptions);
        ddl = ddlGenerator.generateDdl(schema);
        break;
    }
    default:
        qWarning() << "Unsupported database provider for DDL generation";
        return false;
    }
    
    // Write DDL to output file
    QFile file(outputFile);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Cannot open output file:" << outputFile;
        return false;
    }
    
    QTextStream stream(&file);
    stream << ddl;
    
    if (m_config.verbose()) {
        qDebug() << "Successfully generated DDL to" << outputFile;
    }
    
    return true;
}

} // namespace QxOrmQtOrm