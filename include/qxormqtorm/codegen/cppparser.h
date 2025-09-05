#pragma once

#include "../database/schemareader.h"
#include <QString>
#include <QStringList>

namespace QxOrmQtOrm {

/**
 * @brief Parses QtOrm-style C++ classes to extract schema information
 */
class CppParser
{
public:
    /**
     * @brief Parse C++ files in a directory to extract table definitions
     * @param inputDir Directory containing C++ header files
     * @return DatabaseSchema with extracted table definitions
     */
    DatabaseSchema parseDirectory(const QString& inputDir);
    
    /**
     * @brief Parse a single C++ header file to extract table definition
     * @param filePath Path to C++ header file
     * @return TableDefinition extracted from the file
     */
    TableDefinition parseFile(const QString& filePath);
    
private:
    struct ClassInfo
    {
        QString className;
        QString tableName;
        QString baseClass;
        QStringList qProperties;
        QStringList qOrmClassMacros;
        QStringList qOrmPropertyMacros;
    };
    
    ClassInfo parseClassInfo(const QString& content);
    QStringList extractQProperties(const QString& content);
    QStringList extractQOrmMacros(const QString& content, const QString& macroName);
    
    ColumnDefinition parseQProperty(const QString& qPropertyLine);
    void applyQOrmPropertyMacro(ColumnDefinition& column, const QString& macroLine);
    void applyQOrmClassMacro(TableDefinition& table, const QString& macroLine);
    
    QString extractPropertyType(const QString& qPropertyLine);
    QString extractPropertyName(const QString& qPropertyLine);
    QString mapCppTypeToQt(const QString& cppType);
    
    QStringList findHeaderFiles(const QString& directory);
    QString readFileContent(const QString& filePath);
};

} // namespace QxOrmQtOrm