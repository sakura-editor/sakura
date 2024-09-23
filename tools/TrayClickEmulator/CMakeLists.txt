cmake_minimum_required(VERSION 3.7)

# define a project
project (TrayClickEmulator)

# add an executable
add_executable(${PROJECT_NAME} WIN32 TrayClickEmulator.cpp)

# set C++ Standard Level
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# add definitions
target_compile_definitions(${PROJECT_NAME} PUBLIC _CONSOLE _UNICODE UNICODE)

# add include directories
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/../../sakura_core")

# add compiler options
target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

find_package(cxxopts CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

target_link_libraries(${PROJECT_NAME}
    PRIVATE cxxopts::cxxopts
    PRIVATE spdlog::spdlog
)

# add install target
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    RUNTIME DESTINATION tools/click-emu
)
