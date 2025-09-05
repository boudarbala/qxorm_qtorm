#include "qxormqtorm/database/schemareader.h"

namespace QxOrmQtOrm {

const ColumnDefinition* TableDefinition::findColumn(const QString& name) const
{
    for (const auto& column : columns) {
        if (column.name == name) {
            return &column;
        }
    }
    return nullptr;
}

ColumnDefinition* TableDefinition::findColumn(const QString& name)
{
    for (auto& column : columns) {
        if (column.name == name) {
            return &column;
        }
    }
    return nullptr;
}

const TableDefinition* DatabaseSchema::findTable(const QString& name) const
{
    for (const auto& table : tables) {
        if (table.name == name) {
            return &table;
        }
    }
    return nullptr;
}

TableDefinition* DatabaseSchema::findTable(const QString& name)
{
    for (auto& table : tables) {
        if (table.name == name) {
            return &table;
        }
    }
    return nullptr;
}

} // namespace QxOrmQtOrm