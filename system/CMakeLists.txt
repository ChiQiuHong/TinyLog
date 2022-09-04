set(system_SRCS
    AtFork.cc
    ThreadId.cc
    ThreadName.cc
)

add_library(system_libs ${system_SRCS})
target_link_libraries(system_libs pthread ${LIBS})

if(BUILD_EXAMPLES)
    add_subdirectory(test)
endif()
