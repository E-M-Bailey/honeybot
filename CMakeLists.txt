cmake_minimum_required(VERSION 3.15)
project(honeybot VERSION 1.0 DESCRIPTION "Codeburners' secretary")

add_subdirectory(lib/DPP)

add_executable(${PROJECT_NAME}
	src/main.cpp
)
	
target_link_libraries(${PROJECT_NAME}
	dpp
)
	
target_include_directories(${PROJECT_NAME} PRIVATE
	lib/DPP/include
)

set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)
