#include "qxormqtorm/core/configuration.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QDebug>

namespace QxOrmQtOrm {

Configuration::Configuration()
{
}

Configuration Configuration::fromJson(const QJsonObject& json)
{
    Configuration config;
    
    // Provider
    QString providerStr = json["provider"].toString("sqlserver");
    if (providerStr == "sqlserver") {
        config.setProvider(Provider::SqlServer);
    }
    
    // SQL Server specific settings
    if (json.contains("sqlserver")) {
        QJsonObject sqlServerObj = json["sqlserver"].toObject();
        config.setConnectionString(sqlServerObj["connectionString"].toString());
        
        QString schemaModeStr = sqlServerObj["schemaMode"].toString("update");
        if (schemaModeStr == "recreate") {
            config.setSchemaMode(SchemaMode::Recreate);
        } else if (schemaModeStr == "bypass") {
            config.setSchemaMode(SchemaMode::Bypass);
        } else if (schemaModeStr == "append") {
            config.setSchemaMode(SchemaMode::Append);
        } else {
            config.setSchemaMode(SchemaMode::Update);
        }
    }
    
    // Generation options
    config.setNamespacePrefix(json["namespacePrefix"].toString());
    config.setClassPrefix(json["classPrefix"].toString());
    config.setClassSuffix(json["classSuffix"].toString());
    config.setGenerateHeader(json["generateHeader"].toBool(true));
    config.setVerbose(json["verbose"].toBool(false));
    
    return config;
}

Configuration Configuration::fromFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open configuration file:" << filename;
        return Configuration();
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in" << filename << ":" << error.errorString();
        return Configuration();
    }
    
    return fromJson(doc.object());
}

} // namespace QxOrmQtOrm