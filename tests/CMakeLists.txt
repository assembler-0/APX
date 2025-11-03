
add_executable(test_compiler_creation test_compiler_creation.cpp)
target_link_libraries(test_compiler_creation apxlib)

add_test(NAME CompilerCreationTest COMMAND test_compiler_creation)

add_executable(test_functions test_functions.cpp)
target_link_libraries(test_functions apxlib)

add_test(NAME FunctionsTest COMMAND test_functions)
