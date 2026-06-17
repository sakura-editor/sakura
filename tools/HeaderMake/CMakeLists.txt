cmake_minimum_required(VERSION 3.10)

# define a project
project (HeaderMake)

# add test executable
add_executable(${PROJECT_NAME} HeaderMake.cpp)

# add definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC _CONSOLE UNICODE _UNICODE)

add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:${PROJECT_NAME}>
)

# add install target
install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  RUNTIME DESTINATION bin
)
