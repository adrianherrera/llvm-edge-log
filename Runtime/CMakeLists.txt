add_library(edge-log-rt-32 OBJECT edge-log-rt.c)
add_library(edge-log-rt-64 OBJECT edge-log-rt.c)

set_target_properties(edge-log-rt-32 PROPERTIES
                      COMPILE_FLAGS "-m32"
                      LINK_FLAGS "-m32")

install(TARGETS edge-log-rt-32 DESTINATION lib)
install(TARGETS edge-log-rt-64 DESTINATION lib)
