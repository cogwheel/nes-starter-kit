find_package(Python3 REQUIRED)

# This .asm file uses .incbin to compile CHR data into a library
set(CHR_ASM ${CMAKE_CURRENT_SOURCE_DIR}/chr.s)

set(CONVERTER ${CMAKE_CURRENT_SOURCE_DIR}/make_chr.py)

# Convert PNGs to CHRs
set(CHR_FILES "")
file(GLOB PNG_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.png)
foreach(PNG_FILE ${PNG_FILES})
    get_filename_component(PNG_NAME ${PNG_FILE} NAME_WE)
    set(CHR_FILE ${CMAKE_CURRENT_BINARY_DIR}/${PNG_NAME}.chr)

    add_custom_command(
        OUTPUT ${CHR_FILE}
        COMMAND ${Python3_EXECUTABLE} ${CONVERTER} ${PNG_FILE} ${CHR_FILE}
        # Ensure that `chr.s` is reassembled
        COMMAND ${CMAKE_COMMAND} -E touch ${CHR_ASM}
        DEPENDS ${CONVERTER} ${PNG_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Converting ${PNG_NAME}.png to ${PNG_NAME}.chr"
    )

    list(APPEND CHR_FILES ${CHR_FILE})
endforeach()

add_custom_target(convert_chrs DEPENDS ${CHR_FILES})

add_library(chr_data OBJECT ${CMAKE_SOURCE_DIR}/chr/chr.s)
add_dependencies(chr_data convert_chrs)
