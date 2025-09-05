add_executable(basic_example
    main.cpp
)

target_link_libraries(basic_example
    qxormqtorm
    Qt6::Core
    Qt6::Sql
)