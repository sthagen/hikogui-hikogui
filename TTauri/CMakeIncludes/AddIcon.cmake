
include(CMakeIncludes/GetRelativePath.cmake)

function(add_icon RET)
    find_package(python COMPONENTS Interpreter)
    set(SVG2TTICON_PY ${CMAKE_SOURCE_DIR}/Utilities/SVG2TTIcon.py)

    foreach(SOURCE_FILE IN LISTS ARGN)
        get_filename_component(INPUT_PATH ${SOURCE_FILE} ABSOLUTE)
        get_filename_component(INPUT_FILENAME ${SOURCE_FILE} NAME) 
        get_filename_component(INPUT_BASENAME ${SOURCE_FILE} NAME_WLE)
        get_relative_path(INPUT_RELPATH ${INPUT_PATH})

        get_filename_component(OUTPUT_RELDIR ${INPUT_RELPATH} DIRECTORY)
        set(OUTPUT_FILENAME "${INPUT_BASENAME}.tticon")
        set(OUTPUT_RELPATH "${OUTPUT_RELDIR}/${OUTPUT_FILENAME}")
        get_filename_component(OUTPUT_PATH ${OUTPUT_RELPATH} ABSOLUTE BASE_DIR ${CMAKE_CURRENT_BINARY_DIR})

	    # Add a custom command to compile GLSL to SPIR-V.
	    add_custom_command(
		    OUTPUT ${OUTPUT_PATH}
		    COMMAND ${Python_EXECUTABLE} ${SVG2TTICON_PY} -o ${OUTPUT_PATH} ${INPUT_PATH}
		    DEPENDS ${INPUT_PATH} ${SVG2TTICON_PY}
		    VERBATIM)

        set(OUTPUT_PATHS ${OUTPUT_PATHS} ${OUTPUT_PATH})
    endforeach()

    set(${RET} ${OUTPUT_PATHS} PARENT_SCOPE)
endfunction()
