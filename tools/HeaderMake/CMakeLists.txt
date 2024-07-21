cmake_minimum_required(VERSION 3.7)

# define a project
project (HeaderMake)

# add test executable
add_executable(${PROJECT_NAME} ../../HeaderMake/HeaderMake.cpp)

# add definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC _CONSOLE UNICODE _UNICODE)

# add install target
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION bin
)
