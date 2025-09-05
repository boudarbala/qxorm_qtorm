#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "qxormqtorm/qxormqtorm.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("orm2db");
    app.setApplicationVersion("1.0");
    
    QCommandLineParser parser;
    parser.setApplicationDescription("Generate SQL Server DDL from QtOrm C++ classes");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // Input directory option
    QCommandLineOption inputOption(QStringList() << "i" << "input",
                                 "Input directory containing QtOrm C++ classes",
                                 "inputDir", ".");
    parser.addOption(inputOption);
    
    // Output file option
    QCommandLineOption outputOption(QStringList() << "o" << "output",
                                  "Output SQL file",
                                  "outputFile", "schema.sql");
    parser.addOption(outputOption);
    
    // Configuration file option
    QCommandLineOption configOption("config",
                                   "Configuration file (JSON format)",
                                   "configFile");
    parser.addOption(configOption);
    
    // Schema mode option
    QCommandLineOption schemaModeOption("schema-mode",
                                      "Schema generation mode (recreate, update, bypass, append)",
                                      "mode", "update");
    parser.addOption(schemaModeOption);
    
    // Include drop statements option
    QCommandLineOption dropOption("include-drop",
                                "Include DROP TABLE statements");
    parser.addOption(dropOption);
    
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
    QString schemaModeStr = parser.value(schemaModeOption);
    if (schemaModeStr == "recreate") {
        config.setSchemaMode(QxOrmQtOrm::Configuration::SchemaMode::Recreate);
    } else if (schemaModeStr == "bypass") {
        config.setSchemaMode(QxOrmQtOrm::Configuration::SchemaMode::Bypass);
    } else if (schemaModeStr == "append") {
        config.setSchemaMode(QxOrmQtOrm::Configuration::SchemaMode::Append);
    } else {
        config.setSchemaMode(QxOrmQtOrm::Configuration::SchemaMode::Update);
    }
    
    if (parser.isSet(verboseOption)) {
        config.setVerbose(true);
    }
    
    QString inputDir = parser.value(inputOption);
    QString outputFile = parser.value(outputOption);
    
    // Validate input directory
    QDir dir(inputDir);
    if (!dir.exists()) {
        qCritical() << "Input directory does not exist:" << inputDir;
        return 1;
    }
    
    // Ensure output directory exists
    QFileInfo outputInfo(outputFile);
    QDir outputDir = outputInfo.dir();
    if (!outputDir.exists() && !outputDir.mkpath(".")) {
        qCritical() << "Failed to create output directory:" << outputDir.path();
        return 1;
    }
    
    // Generate DDL
    QxOrmQtOrm::Generator generator(config);
    
    if (!generator.generateToDatabase(inputDir, outputFile)) {
        qCritical() << "Failed to generate DDL";
        return 1;
    }
    
    if (config.verbose()) {
        qDebug() << "DDL generated successfully to:" << QFileInfo(outputFile).absoluteFilePath();
    }
    
    return 0;
}