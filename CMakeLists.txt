cmake_minimum_required(VERSION 4.3.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20 REQUIRED)

set(PROJECT_NAME minimal_renderer)
set(INCLUDE_DIR include)
set(SRC_DIR src)
set(SHADERS_DIR shaders)
set(BINARY_DIR bin)
set(GLAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glad)
set(MEMORY_READER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/memory_reader)

Project(${PROJECT_NAME})

list(APPEND SOURCES
    ${GLAD_DIR}/${SRC_DIR}/glad.c
    ${SRC_DIR}/platform_arch_linux.cpp
    ${SRC_DIR}/renderer.cpp
    ${SRC_DIR}/renderer_opengl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/minimal_renderer.cpp)

add_executable(${PROJECT_NAME} ${SOURCES})

add_custom_target(Assets ALL)

find_package(glfw3 3.4 REQUIRED)
find_package(glm 1.0 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INCLUDE_DIR}
    "${MEMORY_READER_DIR}/${INCLUDE_DIR}"
    "${GLAD_DIR}/${INCLUDE_DIR}")

target_link_libraries(${PROJECT_NAME} PRIVATE uring glfw GL glm::glm)

add_custom_command(TARGET Assets POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/${SHADERS_DIR}" $<TARGET_FILE_DIR:${PROJECT_NAME}>/${SHADERS_DIR}
    VERBATIM)

add_dependencies(${PROJECT_NAME} Assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/${BINARY_DIR}/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${SHADERS_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${BINARY_DIR}/$<CONFIG>/${SHADERS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${PROJECT_NAME}>" "${CMAKE_CURRENT_SOURCE_DIR}/${BINARY_DIR}/$<CONFIG>/"
    VERBATIM)
