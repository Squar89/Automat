cmake_minimum_required (VERSION 3.0.2)
project(PW_AUTOMATON C)

set(CMAKE_C_FLAGS "-Wall -Wextra")

add_library(err err.h)
add_library(helper helper.h)
SET_TARGET_PROPERTIES(err helper PROPERTIES LINKER_LANGUAGE C)

add_executable(validator validator.c)
add_executable(tester tester.c)
#add_executable(run run.c)

target_link_libraries(validator err)
target_link_libraries(validator helper)
target_link_libraries(tester err)
target_link_libraries(tester helper)
#tartet_link_libraries(run err)
#target_link_libraries(run helper)


install(TARGETS DESTINATION .)

#copy raport
#configure_file