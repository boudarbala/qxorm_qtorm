add_executable(db2orm
    db2orm.cpp
)

target_link_libraries(db2orm
    qxormqtorm
    Qt6::Core
    Qt6::Sql
)

add_executable(orm2db
    orm2db.cpp
)

target_link_libraries(orm2db
    qxormqtorm
    Qt6::Core
    Qt6::Sql
)