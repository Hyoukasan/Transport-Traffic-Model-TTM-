# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

cmake_minimum_required(VERSION 3.31.2)

project(Transport_Traffic_Model_TTM C)

include_directories(${PROJECT_SOURCE_DIR}/third_party/include)
link_directories(${PROJECT_SOURCE_DIR}/third_party/lib)

file(GLOB SOURCES "src/*.c")

add_executable(TTM ${SOURCES})

set_target_properties(TTM PROPERTIES
	WIN32_EXECUTABLE TRUE
)

#Для линкус нужно модифицировать путь, т.к. opengl32 нет в ОС
target_link_libraries(TTM
	libglew32.dll.a
	libglfw3dll.a
	opengl32
)