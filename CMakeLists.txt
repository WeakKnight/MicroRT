cmake_minimum_required(VERSION 3.8)
project(MicroRT)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for Mac OS X" FORCE)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${PROJECT_SOURCE_DIR}/Bin/Release>)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG $<1:${PROJECT_SOURCE_DIR}/Bin/Debug>)

if (WIN32)
add_compile_definitions(MICRO_RT_OS_WINDOWS)
endif (WIN32)

if (APPLE)
add_compile_definitions(MICRO_RT_OS_MAC)
endif(APPLE)

set(ThirdPartyFolder "ThirdParty")

# tbb
add_library(tbb SHARED IMPORTED)
if (WIN32)
file(COPY ${CMAKE_SOURCE_DIR}/ThirdParty/tbb/lib/win64/tbb12.dll
    DESTINATION ${CMAKE_SOURCE_DIR}/Bin/Debug)
file(COPY ${CMAKE_SOURCE_DIR}/ThirdParty/tbb/lib/win64/tbb12.dll
    DESTINATION ${CMAKE_SOURCE_DIR}/Bin/Release)
set_property(TARGET tbb PROPERTY IMPORTED_LOCATION tbb12.dll)
set_property(TARGET tbb PROPERTY IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/ThirdParty/tbb/lib/win64/tbb12.lib)
add_compile_definitions(__TBB_NO_IMPLICIT_LINKAGE)
add_compile_definitions(__TBBMALLOC_NO_IMPLICIT_LINKAGE)
endif (WIN32)
if (APPLE)
set_property(TARGET tbb PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/tbb/lib/mac/libtbb.12.5.dylib)
endif (APPLE)

# embree
add_library(embree SHARED IMPORTED)
if (WIN32)
file(COPY ${CMAKE_SOURCE_DIR}/ThirdParty/embree/lib/win64/embree4.dll
    DESTINATION ${CMAKE_SOURCE_DIR}/Bin/Debug)
file(COPY ${CMAKE_SOURCE_DIR}/ThirdParty/embree/lib/win64/embree4.dll
    DESTINATION ${CMAKE_SOURCE_DIR}/Bin/Release)
set_property(TARGET embree PROPERTY IMPORTED_LOCATION embree4.dll)
set_property(TARGET embree PROPERTY IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/ThirdParty/embree/lib/win64/embree4.lib)
endif(WIN32)
if (APPLE)
set_property(TARGET embree PROPERTY IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/ThirdParty/embree/lib/mac/libembree4.dylib)
endif(APPLE)

file(GLOB LIBRARY_PRIVATE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.m
    ${CMAKE_CURRENT_SOURCE_DIR}/*.mm)

file(GLOB LIBRARY_PUBLIC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(micro_rt SHARED ${LIBRARY_PUBLIC_FILES} ${LIBRARY_PRIVATE_FILES})

target_include_directories(micro_rt PUBLIC
    ${CMAKE_SOURCE_DIR}/ThirdParty/tbb/include/
    ${CMAKE_SOURCE_DIR}/ThirdParty/embree/include/
)

target_link_libraries(micro_rt tbb embree)

if (APPLE)
set_target_properties(micro_rt PROPERTIES LINK_FLAGS "-framework Foundation -framework QuartzCore -framework Metal")
endif()

if(MSVC)
  target_compile_options(micro_rt PRIVATE /W3 /permissive)
else()
  target_compile_options(micro_rt PRIVATE -Wno-gnu-anonymous-struct)
endif()