#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include "qxormqtorm/qxormqtorm.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("db2orm");
    app.setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Generate QtOrm C++ classes from SQL Server database schema");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Connection string option
    QCommandLineOption connectionOption(QStringList() << "c" << "connection",
                                      "Database connection string",
                                      "connectionString");
    parser.addOption(connectionOption);
    
    // Output directory option
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                  "Output directory for generated classes",
                                  "outputDir", ".");
    parser.addOption(outputOption);
    
    // Configuration file option
    QCommandLineOption configOption("config",
                                   "Configuration file (JSON format)",
                                   "configFile");
    parser.addOption(configOption);
    
    // Namespace option
    QCommandLineOption namespaceOption("namespace",
                                     "Namespace prefix for generated classes",
                                     "namespace");
    parser.addOption(namespaceOption);
    
    // Class prefix option
    QCommandLineOption prefixOption("prefix",
                                  "Prefix for generated class names",
                                  "prefix");
    parser.addOption(prefixOption);
    
    // Class suffix option
    QCommandLineOption suffixOption("suffix",
                                  "Suffix for generated class names",
                                  "suffix");
    parser.addOption(suffixOption);
    
    // Verbose option
    QCommandLineOption verboseOption("verbose",
                                   "Enable verbose output");
    parser.addOption(verboseOption);
    
    // Parse command line
    parser.process(app);
    
    QxOrmQtOrm::Configuration config;
    
    // Load configuration from file if specified
    if (parser.isSet(configOption)) {
        config = QxOrmQtOrm::Configuration::fromFile(parser.value(configOption));
    }
    
    // Override with command line options
    if (parser.isSet(connectionOption)) {
        config.setConnectionString(parser.value(connectionOption));
    }
    
    if (parser.isSet(namespaceOption)) {
        config.setNamespacePrefix(parser.value(namespaceOption));
    }
    
    if (parser.isSet(prefixOption)) {
        config.setClassPrefix(parser.value(prefixOption));
    }
    
    if (parser.isSet(suffixOption)) {
        config.setClassSuffix(parser.value(suffixOption));
    }
    
    if (parser.isSet(verboseOption)) {
        config.setVerbose(true);
    }
    
    // Validate required parameters
    if (config.connectionString().isEmpty()) {
        qCritical() << "Connection string is required. Use --connection or provide a config file.";
        return 1;
    }
    
    QString outputDir = parser.value(outputOption);
    
    // Ensure output directory exists
    QDir dir(outputDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        qCritical() << "Failed to create output directory:" << outputDir;
        return 1;
    }
    
    // Generate classes
    QxOrmQtOrm::Generator generator(config);
    
    if (!generator.generateFromDatabase(outputDir)) {
        qCritical() << "Failed to generate classes";
        return 1;
    }
    
    if (config.verbose()) {
        qDebug() << "Classes generated successfully in:" << QDir(outputDir).absolutePath();
    }
    
    return 0;
}