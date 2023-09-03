cmake_minimum_required(VERSION 3.18)

# This is a helper script for the `run` target.

# CMake bakes environment variables into commands it generates, so any commands
# using the $EMULATOR variable were fixed at the time of configuration. Changing
# $EMULATOR made no difference.

# Instead, we create a command that runs this script, and this script fetches
# what it needs from the environment.

# Pick a shell
#
# I'm not exactly sure why, but this is needed for execute_process to run the
# emulator command correctly.
if(WIN32)
    set(shell_command cmd /c)
else()
    set(shell_command sh -c)
endif()

# Use EMULATOR environment var if available, otherwise use the system launcher
set(emulator_command "$ENV{EMULATOR}")
if("${emulator_command}" STREQUAL "")
    if(WIN32)
        set(emulator_command start)
    elseif(APPLE)
        set(emulator_command open)
    else()
        set(emulator_command xdg-open)
    endif()
    message("EMULATOR environment variable not set. Using `${emulator_command}`")
endif()

# Run the emulator on the rom
# ARGV contains: <cmake_script> -P -- <rom_file>
set(ROM ${CMAKE_ARGV4})
get_filename_component(ROM_DIR ${ROM} DIRECTORY)
set(full_command ${shell_command} "${emulator_command} ${ROM}")
execute_process(
    COMMAND ${full_command}
    WORKING_DIRECTORY ${ROM_DIR}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
)

if(result)
    message(FATAL_ERROR "Failed to run ${full_command}:\n${output}\n${error}")
endif()
