#pragma once

#include <QObject>
#include <QString>
#include <QJsonObject>

namespace QxOrmQtOrm {

/**
 * @brief Configuration for database connection and generation options
 */
class Configuration
{
public:
    enum class Provider {
        SqlServer,
        // Future: MySQL, PostgreSQL, etc.
    };
    
    enum class SchemaMode {
        Recreate,
        Update,
        Bypass,
        Append
    };
    
    Configuration();
    
    // Static factory methods
    static Configuration fromJson(const QJsonObject& json);
    static Configuration fromFile(const QString& filename);
    
    // Provider settings
    Provider provider() const { return m_provider; }
    void setProvider(Provider provider) { m_provider = provider; }
    
    // Connection settings
    QString connectionString() const { return m_connectionString; }
    void setConnectionString(const QString& connectionString) { m_connectionString = connectionString; }
    
    // Schema mode
    SchemaMode schemaMode() const { return m_schemaMode; }
    void setSchemaMode(SchemaMode mode) { m_schemaMode = mode; }
    
    // Generation options
    QString namespacePrefix() const { return m_namespacePrefix; }
    void setNamespacePrefix(const QString& prefix) { m_namespacePrefix = prefix; }
    
    QString classPrefix() const { return m_classPrefix; }
    void setClassPrefix(const QString& prefix) { m_classPrefix = prefix; }
    
    QString classSuffix() const { return m_classSuffix; }
    void setClassSuffix(const QString& suffix) { m_classSuffix = suffix; }
    
    bool generateHeader() const { return m_generateHeader; }
    void setGenerateHeader(bool generate) { m_generateHeader = generate; }
    
    bool verbose() const { return m_verbose; }
    void setVerbose(bool verbose) { m_verbose = verbose; }
    
private:
    Provider m_provider = Provider::SqlServer;
    QString m_connectionString;
    SchemaMode m_schemaMode = SchemaMode::Update;
    QString m_namespacePrefix;
    QString m_classPrefix;
    QString m_classSuffix;
    bool m_generateHeader = true;
    bool m_verbose = false;
};

} // namespace QxOrmQtOrm