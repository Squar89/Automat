cmake_minimum_required (VERSION 3.0.2)
project(PW_AUTOMATON C)

set(CMAKE_C_FLAGS "-Wall -Wextra")

add_library(err err.h)
add_library(helper helper.h)
SET_TARGET_PROPERTIES(err helper PROPERTIES LINKER_LANGUAGE C)

add_executable(validator validator.c)
add_executable(tester tester.c)
#add_executable(run run.c)

install(TARGETS DESTINATION .)

#copy raport
#configure_file