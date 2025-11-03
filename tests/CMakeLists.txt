add_executable(test_functions test_functions.cpp)
target_link_libraries(test_functions apxlib)

add_test(NAME FunctionsTest COMMAND test_functions)
