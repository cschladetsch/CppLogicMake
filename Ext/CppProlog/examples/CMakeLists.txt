include_directories(../src)

add_executable(basic_example basic_example.cpp)
target_link_libraries(basic_example prolog_core)

add_executable(family_tree family_tree.cpp)
target_link_libraries(family_tree prolog_core)

add_executable(list_processing list_processing.cpp)
target_link_libraries(list_processing prolog_core)

add_executable(arithmetic arithmetic.cpp)
target_link_libraries(arithmetic prolog_core)

add_executable(docker_example docker_example.cpp)
target_link_libraries(docker_example prolog_core)