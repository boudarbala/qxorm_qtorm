set(SOURCES
    database/sqlserverprovider.cpp
    database/schemareader.cpp
    codegen/qtormclassgenerator.cpp
    codegen/sqlserverddlgenerator.cpp
    codegen/cppparser.cpp
    core/configuration.cpp
)

set(HEADERS
    ../include/qxormqtorm/database/sqlserverprovider.h
    ../include/qxormqtorm/database/schemareader.h
    ../include/qxormqtorm/codegen/qtormclassgenerator.h
    ../include/qxormqtorm/codegen/sqlserverddlgenerator.h
    ../include/qxormqtorm/codegen/cppparser.h
    ../include/qxormqtorm/core/configuration.h
    ../include/qxormqtorm/qxormqtorm.h
)

add_library(qxormqtorm STATIC ${SOURCES} ${HEADERS})

target_link_libraries(qxormqtorm
    Qt6::Core
    Qt6::Sql
)

target_include_directories(qxormqtorm PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)