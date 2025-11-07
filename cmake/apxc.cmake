
# - Try to find the APX compiler
#
# Once done this will define
#
#  APX_FOUND - system has APX
#  APX_EXECUTABLE - the APX compiler
#  APX_VERSION_STRING - the version of APX found

find_program(APX_EXECUTABLE apxc)

if(APX_EXECUTABLE)
    execute_process(
        COMMAND ${APX_EXECUTABLE} --version
        OUTPUT_VARIABLE APX_VERSION_STRING
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(APX
    REQUIRED_VARS APX_EXECUTABLE
    VERSION_VAR APX_VERSION_STRING)

mark_as_advanced(APX_EXECUTABLE)

if(APX_FOUND)
    function(apx_compile_file input_file output_file)
        add_custom_command(
            OUTPUT ${output_file}
            COMMAND ${APX_EXECUTABLE} -i ${input_file} -o ${output_file}
            DEPENDS ${input_file}
            COMMENT "Compiling APX file ${input_file}"
        )
    endfunction()

    function(apx_add_library target_name)
        set(output_files "")
        set(asm_files "")
        foreach(source_file ${ARGN})
            get_filename_component(basename ${source_file} NAME_WE)
            set(generated_asm "${CMAKE_CURRENT_BINARY_DIR}/${basename}.s")
            set(generated_obj "${CMAKE_CURRENT_BINARY_DIR}/${basename}.o")

            apx_compile_file(${CMAKE_CURRENT_SOURCE_DIR}/${source_file} ${generated_asm})

            add_custom_command(
                OUTPUT ${generated_obj}
                COMMAND nasm -f elf64 ${generated_asm} -o ${generated_obj}
                DEPENDS ${generated_asm}
                COMMENT "Assembling ${generated_asm}"
            )
            list(APPEND output_files ${generated_obj})
            list(APPEND asm_files ${generated_asm})
        endforeach()

        add_library(${target_name} STATIC ${output_files})
        
        # Add a custom target to clean the generated assembly files
        add_custom_target(Clean${target_name}
            COMMAND ${CMAKE_COMMAND} -E remove ${asm_files} ${output_files}
            COMMENT "Cleaning generated files for ${target_name}"
        )
        add_dependencies(${target_name} Clean${target_name})
    endfunction()
endif()
