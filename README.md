# apx-lang

## Build

```bash
mkdir build && cd build
cmake ..
make
```

## Run

```bash
./apxc
```

## CMake Integration

To use the APX compiler in your CMake projects, you can use the `apxc.cmake` module.

First, you need to tell CMake where to find the `apxc.cmake` module. You can do this by adding the path to the `cmake` directory to the `CMAKE_MODULE_PATH`.

```cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${APX_SOURCE_DIR}/cmake")
```

Then, you can use `find_package` to find the APX compiler.

```cmake
find_package(APX REQUIRED)
```

If the APX compiler is found, you can use the `apx_add_library` function to compile your `.apx` files and create a static library.

```cmake
apx_add_library(my_apx_lib src/my_lib.apx src/my_other_lib.apx)
```

This will create a static library called `my_apx_lib`. You can then link against this library in your executable.

```cmake
add_executable(my_executable src/main.cpp)
target_link_libraries(my_executable my_apx_lib)
```

### Example CMakeLists.txt

Here is an example `CMakeLists.txt` for a project that uses an APX library.

```cmake
cmake_minimum_required(VERSION 3.20)
project(MyProject)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${APX_SOURCE_DIR}/cmake")

find_package(APX REQUIRED)

apx_add_library(my_apx_lib src/my_lib.apx)

add_executable(my_executable src/main.cpp)
target_link_libraries(my_executable my_apx_lib)
```
