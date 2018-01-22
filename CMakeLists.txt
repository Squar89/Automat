cmake_minimum_required (VERSION 3.0.2)
project(PW_AUTOMATON C)

set(CMAKE_C_FLAGS "-Wall -Wextra")

add_library(err err.c)
add_library(helper helper.c)
add_library(dArray dArray.c)
SET_TARGET_PROPERTIES(err helper dArray PROPERTIES LINKER_LANGUAGE C)

add_executable(validator validator.c)
add_executable(tester tester.c)
add_executable(run run.c)
add_executable(clear clear.c)

target_link_libraries(validator err helper dArray -lrt)
target_link_libraries(tester err helper -lrt)
target_link_libraries(run err helper dArray -lrt)
target_link_libraries(clear -lrt)

install(TARGETS DESTINATION .)