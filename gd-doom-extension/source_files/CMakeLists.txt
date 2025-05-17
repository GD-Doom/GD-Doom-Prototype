
set(GDEXTENSION_NAME gd-doom)

set(OUTPUT_NAME ${GDEXTENSION_NAME})
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Windows>:.windows>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Linux>:.linux>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Android>:.android>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<PLATFORM_ID:Darwin>:.macos>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<CONFIG:Debug>:.debug>)
set(OUTPUT_NAME ${OUTPUT_NAME}$<$<CONFIG:Release>:.release>)

if (CMAKE_SIZE_OF_VOID_P EQUAL 4)
    set(OUTPUT_NAME ${OUTPUT_NAME}.x86_32)
else()
    set(OUTPUT_NAME ${OUTPUT_NAME}.x86_64)
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Add a dynamic library named ${GDEXTENSION_NAME}
add_library(${GDEXTENSION_NAME} SHARED ${SOURCES})

#---------------------------------------------------------------------------
# Setup TARGET.
#---------------------------------------------------------------------------

set_target_properties(${GDEXTENSION_NAME} PROPERTIES OUTPUT_NAME "${OUTPUT_NAME}")
target_include_directories(${GDEXTENSION_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(${GDEXTENSION_NAME} PUBLIC godot::cpp)
