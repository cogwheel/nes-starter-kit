# We don't want any of the default optimization options for any of the
# languages.
#
# Release builds use -DNDEBUG and -Werror
# All build types except DEBUG use link-time optimization

# TODO: MinSizeRel is redundant with release. We may want to use custom variants
# that are more suited to NES.
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -flto -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-DNDEBUG -flto -Werror")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-flto")

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -flto -Werror")
set(CMAKE_C_FLAGS_DEBUG "")
set(CMAKE_C_FLAGS_MINSIZEREL "-DNDEBUG -flto -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-flto")

# Remove -DNDEBUG from asm flags (unrecognized by LLVM assembler)
# Removes other optimization options as well, to avoid confusion
set(CMAKE_ASM_FLAGS_RELEASE "")
set(CMAKE_ASM_FLAGS_DEBUG "")
set(CMAKE_ASM_FLAGS_MINSIZEREL "")
set(CMAKE_ASM_FLAGS_RELWITHDEBINFO "")
