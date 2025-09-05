#pragma once

#include "qxormqtorm/core/configuration.h"
#include "qxormqtorm/database/schemareader.h" 
#include "qxormqtorm/database/sqlserverprovider.h"
#include "qxormqtorm/codegen/qtormclassgenerator.h"
#include "qxormqtorm/codegen/sqlserverddlgenerator.h"
#include "qxormqtorm/codegen/cppparser.h"

namespace QxOrmQtOrm {

/**
 * @brief Main facade for the QxOrm+QtOrm integration library
 * 
 * This class provides the main entry points for database-to-ORM and
 * ORM-to-database code generation.
 */
class Generator
{
public:
    explicit Generator(const Configuration& config);
    
    /**
     * @brief Generate QtOrm classes from database schema
     * @param outputDir Directory to write generated C++ files
     * @return true if successful
     */
    bool generateFromDatabase(const QString& outputDir);
    
    /**
     * @brief Generate SQL DDL from QtOrm classes
     * @param inputDir Directory containing QtOrm C++ class files
     * @param outputFile SQL file to write DDL statements
     * @return true if successful
     */
    bool generateToDatabase(const QString& inputDir, const QString& outputFile);
    
private:
    Configuration m_config;
};

} // namespace QxOrmQtOrm