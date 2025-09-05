#include "qxormqtorm/codegen/cppparser.h"
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

namespace QxOrmQtOrm {

DatabaseSchema CppParser::parseDirectory(const QString& inputDir)
{
    DatabaseSchema schema;
    
    QStringList headerFiles = findHeaderFiles(inputDir);
    
    for (const QString& filePath : headerFiles) {
        TableDefinition table = parseFile(filePath);
        if (!table.name.isEmpty()) {
            schema.tables.append(table);
        }
    }
    
    return schema;
}

TableDefinition CppParser::parseFile(const QString& filePath)
{
    TableDefinition table;
    
    QString content = readFileContent(filePath);
    if (content.isEmpty()) {
        return table;
    }
    
    ClassInfo classInfo = parseClassInfo(content);
    
    if (classInfo.className.isEmpty()) {
        return table;
    }
    
    // Initialize table with class name
    table.name = classInfo.tableName.isEmpty() ? classInfo.className : classInfo.tableName;
    
    // Apply Q_ORM_CLASS macros
    for (const QString& macro : classInfo.qOrmClassMacros) {
        applyQOrmClassMacro(table, macro);
    }
    
    // Parse Q_PROPERTY declarations
    for (const QString& qProperty : classInfo.qProperties) {
        ColumnDefinition column = parseQProperty(qProperty);
        if (!column.name.isEmpty()) {
            table.columns.append(column);
        }
    }
    
    // Apply Q_ORM_PROPERTY macros to corresponding columns
    for (const QString& macro : classInfo.qOrmPropertyMacros) {
        // Find which property this macro applies to
        QRegularExpression macroRegex(R"(Q_ORM_PROPERTY\s*\(\s*(\w+))");
        QRegularExpressionMatch macroMatch = macroRegex.match(macro);
        if (macroMatch.hasMatch()) {
            QString propertyName = macroMatch.captured(1);
            
            // Find the corresponding column
            for (auto& column : table.columns) {
                if (column.name == propertyName) {
                    applyQOrmPropertyMacro(column, macro);
                    break;
                }
            }
        }
    }
    
    // Find primary key columns
    for (const auto& column : table.columns) {
        if (column.isPrimaryKey) {
            table.primaryKeyColumns << column.name;
        }
    }
    
    return table;
}

CppParser::ClassInfo CppParser::parseClassInfo(const QString& content)
{
    ClassInfo info;
    
    // Find class declaration
    QRegularExpression classRegex(R"(class\s+(\w+)\s*:\s*public\s+QObject)");
    QRegularExpressionMatch classMatch = classRegex.match(content);
    
    if (classMatch.hasMatch()) {
        info.className = classMatch.captured(1);
        info.baseClass = "QObject";
    }
    
    // Find Q_PROPERTY declarations
    info.qProperties = extractQProperties(content);
    
    // Find Q_ORM_CLASS macros
    info.qOrmClassMacros = extractQOrmMacros(content, "Q_ORM_CLASS");
    
    // Find Q_ORM_PROPERTY macros
    info.qOrmPropertyMacros = extractQOrmMacros(content, "Q_ORM_PROPERTY");
    
    return info;
}

QStringList CppParser::extractQProperties(const QString& content)
{
    QStringList properties;
    
    QRegularExpression regex(R"(Q_PROPERTY\s*\([^)]+\))");
    QRegularExpressionMatchIterator it = regex.globalMatch(content);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        properties << match.captured(0);
    }
    
    return properties;
}

QStringList CppParser::extractQOrmMacros(const QString& content, const QString& macroName)
{
    QStringList macros;
    
    QString pattern = QString(R"(%1\s*\([^)]+\))").arg(macroName);
    QRegularExpression regex(pattern);
    QRegularExpressionMatchIterator it = regex.globalMatch(content);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        macros << match.captured(0);
    }
    
    return macros;
}

ColumnDefinition CppParser::parseQProperty(const QString& qPropertyLine)
{
    ColumnDefinition column;
    
    // Extract type and property name from Q_PROPERTY
    QRegularExpression regex(R"(Q_PROPERTY\s*\(\s*(\w+(?:\s*\*)?)\s+(\w+)\s+READ\s+(\w+)(?:\s+WRITE\s+(\w+))?(?:\s+NOTIFY\s+(\w+))?\s*(?:STORED\s+(false|true))?\s*\))");
    QRegularExpressionMatch match = regex.match(qPropertyLine);
    
    if (match.hasMatch()) {
        QString propertyType = match.captured(1).trimmed();
        column.name = match.captured(2);
        column.qtType = propertyType;
        
        // Set default nullability - most columns should be NOT NULL unless specified
        column.isNullable = false;
        
        // Check if it's stored (STORED false means transient)
        QString stored = match.captured(6);
        if (stored == "false") {
            // This is a transient property, skip it
            column.name.clear();
            return column;
        }
    }
    
    return column;
}

void CppParser::applyQOrmPropertyMacro(ColumnDefinition& column, const QString& macroLine)
{
    // Extract property name and options from Q_ORM_PROPERTY
    QRegularExpression regex(R"(Q_ORM_PROPERTY\s*\(\s*(\w+)\s+([^)]+)\s*\))");
    QRegularExpressionMatch match = regex.match(macroLine);
    
    if (!match.hasMatch()) {
        return;
    }
    
    QString propertyName = match.captured(1);
    QString options = match.captured(2);
    
    // Parse options
    if (options.contains("IDENTITY")) {
        column.isPrimaryKey = true;
        column.isNullable = false;  // Primary keys are NOT NULL
    }
    
    if (options.contains("AUTOGENERATED")) {
        column.isAutoIncrement = true;
    }
    
    if (options.contains("TRANSIENT")) {
        // Mark as transient - should be excluded from table
        column.name.clear();
        return;
    }
    
    // Extract COLUMN name
    QRegularExpression columnRegex(R"(COLUMN\s+(\w+))");
    QRegularExpressionMatch columnMatch = columnRegex.match(options);
    if (columnMatch.hasMatch()) {
        column.name = columnMatch.captured(1);
    }
}

void CppParser::applyQOrmClassMacro(TableDefinition& table, const QString& macroLine)
{
    // Extract options from Q_ORM_CLASS
    QRegularExpression regex(R"(Q_ORM_CLASS\s*\(\s*([^)]+)\s*\))");
    QRegularExpressionMatch match = regex.match(macroLine);
    
    if (!match.hasMatch()) {
        return;
    }
    
    QString options = match.captured(1);
    
    // Extract TABLE name
    QRegularExpression tableRegex(R"(TABLE\s+(\w+))");
    QRegularExpressionMatch tableMatch = tableRegex.match(options);
    if (tableMatch.hasMatch()) {
        table.name = tableMatch.captured(1);
    }
    
    // Extract SCHEMA
    QRegularExpression schemaRegex(R"(SCHEMA\s+(\w+))");
    QRegularExpressionMatch schemaMatch = schemaRegex.match(options);
    if (schemaMatch.hasMatch()) {
        table.schema = schemaMatch.captured(1);
    }
}

QString CppParser::extractPropertyType(const QString& qPropertyLine)
{
    QRegularExpression regex(R"(Q_PROPERTY\s*\(\s*(\w+(?:\s*\*)?))");
    QRegularExpressionMatch match = regex.match(qPropertyLine);
    
    if (match.hasMatch()) {
        return match.captured(1).trimmed();
    }
    
    return QString();
}

QString CppParser::extractPropertyName(const QString& qPropertyLine)
{
    QRegularExpression regex(R"(Q_PROPERTY\s*\(\s*\w+(?:\s*\*)?\s+(\w+))");
    QRegularExpressionMatch match = regex.match(qPropertyLine);
    
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    return QString();
}

QString CppParser::mapCppTypeToQt(const QString& cppType)
{
    // This method could be used to map custom C++ types to Qt types
    // For now, assume the types are already Qt types
    return cppType;
}

QStringList CppParser::findHeaderFiles(const QString& directory)
{
    QStringList headerFiles;
    
    QDir dir(directory);
    if (!dir.exists()) {
        qWarning() << "Directory does not exist:" << directory;
        return headerFiles;
    }
    
    // Find .h files recursively
    QStringList nameFilters;
    nameFilters << "*.h" << "*.hpp";
    
    QDirIterator it(directory, nameFilters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        headerFiles << it.next();
    }
    
    return headerFiles;
}

QString CppParser::readFileContent(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open file:" << filePath;
        return QString();
    }
    
    QTextStream stream(&file);
    return stream.readAll();
}

} // namespace QxOrmQtOrm