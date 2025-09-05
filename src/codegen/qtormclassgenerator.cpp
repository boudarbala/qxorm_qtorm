#include "qxormqtorm/codegen/qtormclassgenerator.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>

namespace QxOrmQtOrm {

QtOrmClassGenerator::QtOrmClassGenerator()
{
    // Initialize default options
    m_options.generateHeader = true;
    m_options.generateQProperty = true;
    m_options.generateQInvokable = true;
    m_options.generateSignals = true;
    m_options.headerGuardPrefix = "GENERATED";
    m_options.indent = "    ";
}

QtOrmClassGenerator::QtOrmClassGenerator(const GenerationOptions& options)
    : m_options(options)
{
}

bool QtOrmClassGenerator::generateClasses(const DatabaseSchema& schema, const QString& outputDir)
{
    QDir dir(outputDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create output directory:" << outputDir;
        return false;
    }
    
    bool success = true;
    for (const auto& table : schema.tables) {
        if (!generateClass(table, outputDir)) {
            success = false;
        }
    }
    
    return success;
}

bool QtOrmClassGenerator::generateClass(const TableDefinition& table, const QString& outputDir)
{
    QString className = generateClassName(table.name);
    
    // Generate header file
    if (m_options.generateHeader) {
        QString headerPath = QDir(outputDir).filePath(className.toLower() + ".h");
        QFile headerFile(headerPath);
        
        if (!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Failed to create header file:" << headerPath;
            return false;
        }
        
        QTextStream headerStream(&headerFile);
        headerStream << generateHeaderContent(table);
    }
    
    // Generate source file
    QString sourcePath = QDir(outputDir).filePath(className.toLower() + ".cpp");
    QFile sourceFile(sourcePath);
    
    if (!sourceFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to create source file:" << sourcePath;
        return false;
    }
    
    QTextStream sourceStream(&sourceFile);
    sourceStream << generateSourceContent(table);
    
    return true;
}

QString QtOrmClassGenerator::generateHeaderContent(const TableDefinition& table)
{
    QString className = generateClassName(table.name);
    QString headerGuard = QString("%1_%2_H").arg(m_options.headerGuardPrefix, className.toUpper());
    
    QStringList lines;
    
    // Header guard and includes
    lines << QString("#pragma once");
    lines << QString("#ifndef %1").arg(headerGuard);
    lines << QString("#define %1").arg(headerGuard);
    lines << "";
    lines << "#include <QObject>";
    lines << "#include <QString>";
    lines << "#include <QDateTime>";
    lines << "#include <QDate>";
    lines << "#include <QTime>";
    lines << "#include <QByteArray>";
    lines << "#include <QUuid>";
    lines << "";
    
    // Namespace opening
    if (!m_options.namespacePrefix.isEmpty()) {
        lines << QString("namespace %1 {").arg(m_options.namespacePrefix);
        lines << "";
    }
    
    // Class declaration
    lines << QString("class %1 : public QObject").arg(className);
    lines << "{";
    lines << QString("%1Q_OBJECT").arg(m_options.indent);
    lines << "";
    
    // Q_PROPERTY declarations
    for (const auto& column : table.columns) {
        if (m_options.generateQProperty) {
            lines << QString("%1%2").arg(m_options.indent, generateQPropertyDeclaration(column));
        }
    }
    
    // Q_ORM_CLASS macro
    if (table.name != className) {
        lines << "";
        lines << QString("%1Q_ORM_CLASS(TABLE %2)").arg(m_options.indent, table.name);
    }
    
    // Q_ORM_PROPERTY macros for special columns
    for (const auto& column : table.columns) {
        QString ormProperty = generateQOrmPropertyMacro(column);
        if (!ormProperty.isEmpty()) {
            lines << QString("%1%2").arg(m_options.indent, ormProperty);
        }
    }
    
    lines << "";
    lines << "public:";
    
    // Constructor
    if (m_options.generateQInvokable) {
        lines << QString("%1Q_INVOKABLE explicit %2(QObject* parent = nullptr);").arg(m_options.indent, className);
    } else {
        lines << QString("%1explicit %2(QObject* parent = nullptr);").arg(m_options.indent, className);
    }
    lines << "";
    
    // Property accessors
    for (const auto& column : table.columns) {
        lines << generatePropertyAccessors(column);
    }
    
    // Signals
    if (m_options.generateSignals) {
        lines << "signals:";
        for (const auto& column : table.columns) {
            lines << generatePropertySignal(column);
        }
        lines << "";
    }
    
    lines << "private:";
    
    // Member variables
    for (const auto& column : table.columns) {
        lines << generateMemberVariable(column);
    }
    
    lines << "};";
    
    // Namespace closing
    if (!m_options.namespacePrefix.isEmpty()) {
        lines << "";
        lines << QString("} // namespace %1").arg(m_options.namespacePrefix);
    }
    
    lines << "";
    lines << QString("#endif // %1").arg(headerGuard);
    
    return lines.join("\n");
}

QString QtOrmClassGenerator::generateSourceContent(const TableDefinition& table)
{
    QString className = generateClassName(table.name);
    
    QStringList lines;
    
    // Include header
    lines << QString("#include \"%1.h\"").arg(className.toLower());
    lines << "";
    
    // Namespace opening
    if (!m_options.namespacePrefix.isEmpty()) {
        lines << QString("namespace %1 {").arg(m_options.namespacePrefix);
        lines << "";
    }
    
    // Constructor implementation
    lines << QString("%1::%1(QObject* parent)").arg(className);
    lines << QString("%1: QObject(parent)").arg(m_options.indent);
    
    // Initialize primitive members with default values
    QStringList memberInits;
    for (const auto& column : table.columns) {
        QString memberName = generateMemberName(column.name);
        if (column.qtType == "qint32" || column.qtType == "qint16" || column.qtType == "qint64" || column.qtType == "quint8") {
            memberInits << QString("%1{0}").arg(memberName);
        } else if (column.qtType == "bool") {
            memberInits << QString("%1{false}").arg(memberName);
        } else if (column.qtType == "float" || column.qtType == "double") {
            memberInits << QString("%1{0.0}").arg(memberName);
        }
    }
    
    if (!memberInits.isEmpty()) {
        lines << QString("%1, %2").arg(m_options.indent, memberInits.join(QString(", ")));
    }
    
    lines << "{";
    lines << "}";
    lines << "";
    
    // Property accessor implementations
    for (const auto& column : table.columns) {
        QString propertyName = generatePropertyName(column.name);
        QString getterName = generateGetterName(column.name);
        QString setterName = generateSetterName(column.name);
        QString signalName = generateSignalName(column.name);
        QString memberName = generateMemberName(column.name);
        
        // Getter
        lines << QString("%1 %2::%3() const").arg(column.qtType, className, getterName);
        lines << "{";
        lines << QString("%1return %2;").arg(m_options.indent, memberName);
        lines << "}";
        lines << "";
        
        // Setter
        lines << QString("void %1::%2(const %3& %4)").arg(className, setterName, column.qtType, propertyName);
        lines << "{";
        lines << QString("%1if (%2 != %3) {").arg(m_options.indent, memberName, propertyName);
        lines << QString("%1%1%2 = %3;").arg(m_options.indent, memberName, propertyName);
        if (m_options.generateSignals) {
            lines << QString("%1%1emit %2();").arg(m_options.indent, signalName);
        }
        lines << QString("%1}").arg(m_options.indent);
        lines << "}";
        lines << "";
    }
    
    // Namespace closing
    if (!m_options.namespacePrefix.isEmpty()) {
        lines << QString("} // namespace %1").arg(m_options.namespacePrefix);
    }
    
    return lines.join("\n");
}

QString QtOrmClassGenerator::generateClassName(const QString& tableName)
{
    QString name = tableName;
    
    // Convert to PascalCase
    name = name.replace("_", " ");
    QStringList words = name.split(" ", Qt::SkipEmptyParts);
    name = "";
    for (const QString& word : words) {
        QString capitalized = word.toLower();
        if (!capitalized.isEmpty()) {
            capitalized[0] = capitalized[0].toUpper();
        }
        name += capitalized;
    }
    
    return m_options.classPrefix + name + m_options.classSuffix;
}

QString QtOrmClassGenerator::generatePropertyName(const QString& columnName)
{
    QString name = columnName;
    
    // Convert to camelCase
    name = name.replace("_", " ");
    QStringList words = name.split(" ", Qt::SkipEmptyParts);
    if (words.isEmpty()) return columnName;
    
    QString result = words[0].toLower();
    for (int i = 1; i < words.size(); ++i) {
        QString word = words[i].toLower();
        if (!word.isEmpty()) {
            word[0] = word[0].toUpper();
        }
        result += word;
    }
    
    return result;
}

QString QtOrmClassGenerator::generateGetterName(const QString& columnName)
{
    return generatePropertyName(columnName);
}

QString QtOrmClassGenerator::generateSetterName(const QString& columnName)
{
    QString propertyName = generatePropertyName(columnName);
    if (!propertyName.isEmpty()) {
        propertyName[0] = propertyName[0].toUpper();
    }
    return "set" + propertyName;
}

QString QtOrmClassGenerator::generateSignalName(const QString& columnName)
{
    return generatePropertyName(columnName) + "Changed";
}

QString QtOrmClassGenerator::generateMemberName(const QString& columnName)
{
    return "m_" + generatePropertyName(columnName);
}

QString QtOrmClassGenerator::generateQPropertyDeclaration(const ColumnDefinition& column)
{
    QString propertyName = generatePropertyName(column.name);
    QString getterName = generateGetterName(column.name);
    QString setterName = generateSetterName(column.name);
    QString signalName = generateSignalName(column.name);
    
    QString qProperty = QString("Q_PROPERTY(%1 %2 READ %3 WRITE %4")
                        .arg(column.qtType, propertyName, getterName, setterName);
    
    if (m_options.generateSignals) {
        qProperty += QString(" NOTIFY %1").arg(signalName);
    }
    
    qProperty += ")";
    return qProperty;
}

QString QtOrmClassGenerator::generatePropertyAccessors(const ColumnDefinition& column)
{
    QStringList lines;
    QString getterName = generateGetterName(column.name);
    QString setterName = generateSetterName(column.name);
    QString propertyName = generatePropertyName(column.name);
    
    // Getter declaration
    lines << QString("%1%2 %3() const;").arg(m_options.indent, column.qtType, getterName);
    
    // Setter declaration
    lines << QString("%1void %2(const %3& %4);").arg(m_options.indent, setterName, column.qtType, propertyName);
    lines << "";
    
    return lines.join("\n");
}

QString QtOrmClassGenerator::generatePropertySignal(const ColumnDefinition& column)
{
    QString signalName = generateSignalName(column.name);
    return QString("%1void %2();").arg(m_options.indent, signalName);
}

QString QtOrmClassGenerator::generateMemberVariable(const ColumnDefinition& column)
{
    QString memberName = generateMemberName(column.name);
    return QString("%1%2 %3;").arg(m_options.indent, column.qtType, memberName);
}

QString QtOrmClassGenerator::generateQOrmPropertyMacro(const ColumnDefinition& column)
{
    QStringList parts;
    
    if (column.isPrimaryKey) {
        parts << "IDENTITY";
    }
    
    if (column.isAutoIncrement) {
        parts << "AUTOGENERATED";
    }
    
    if (column.name != generatePropertyName(column.name)) {
        parts << QString("COLUMN %1").arg(column.name);
    }
    
    if (parts.isEmpty()) {
        return QString();
    }
    
    return QString("Q_ORM_PROPERTY(%1 %2)").arg(generatePropertyName(column.name), parts.join(" "));
}

} // namespace QxOrmQtOrm