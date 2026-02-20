# ===================================
# Transport Traffic Model (TTM) on C
# ===================================

cmake_minimum_required(VERSION 3.31.2)

project(Transport_Traffic_Model_TTM C)

add_executable(TTM
	src/main.c
	src/application.c
)

#Для линкус нужно модифицировать путь, т.к. opengl32 нет в ОС
target_link_libraries(TTM
	glfw3
	glew32
	opengl32
)