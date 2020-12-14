add_library(edge-log-rt-32 STATIC edge-log-rt.cpp)
add_library(edge-log-rt-64 STATIC edge-log-rt.cpp)

include(${CMAKE_ROOT}/Modules/FindZLIB.cmake)
target_include_directories(edge-log-rt-32 PUBLIC ${ZLIB_INCLUDE_DIRS})
target_include_directories(edge-log-rt-64 PUBLIC ${ZLIB_INCLUDE_DIRS})

set_target_properties(edge-log-rt-64 PROPERTIES
                      COMPILE_FLAGS "-fPIC")
set_target_properties(edge-log-rt-32 PROPERTIES
                      COMPILE_FLAGS "-m32 -fPIC"
                      LINK_FLAGS "-m32")

install(TARGETS edge-log-rt-32 DESTINATION lib)
install(TARGETS edge-log-rt-64 DESTINATION lib)
