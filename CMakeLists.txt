
cmake_minimum_required(VERSION 3.12)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJ_NAME vkdemo)
project(${PROJ_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)


find_package(Vulkan REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

aux_source_directory(src SRC_DIR)

add_executable(${PROJ_NAME})
target_sources(${PROJ_NAME} PRIVATE ${SRC_DIR})
target_include_directories(${PROJ_NAME} PRIVATE  "include")

target_link_libraries(${PROJ_NAME} ${Vulkan_LIBRARIES} glfw glm)

install(TARGETS ${PROJ_NAME} LIBRARY DESTINATION "lib" RUNTIME DESTINATION "bin" ARCHIVE DESTINATION "lib")

