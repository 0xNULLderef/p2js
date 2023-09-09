cmake_minimum_required(VERSION 3.5)

project(p2js LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(SUBHOOK_FORCE_32BIT ON CACHE BOOL "Builds subhook as a 32-bit library on 64-bit hosts")
set(SUBHOOK_STATIC ON CACHE BOOL "Builds subhook as a static library")
add_subdirectory(subhook)

set(V8_INCLUDE
	"${CMAKE_SOURCE_DIR}/../v8"
	"${CMAKE_SOURCE_DIR}/../v8/include"
)

set(V8_LIB
	"${CMAKE_SOURCE_DIR}/../v8/out/"
)

add_library(v8 STATIC IMPORTED)
set_target_properties(v8 PROPERTIES
	IMPORTED_LOCATION "${V8_LIB}"
	INTERFACE_INCLUDE_DIRECTORIES "${V8_INCLUDE}"
)

set(SOURCES_P2JS
	"p2js/hooks.cpp"
	"p2js/javascript.cpp"
	"p2js/javascriptconsole.cpp"
	"p2js/logger.cpp"
	"p2js/main.cpp"
	"p2js/memory.cpp"
	"p2js/plugin.cpp"
)

add_library(p2js SHARED ${SOURCES_P2JS})
set_target_properties(p2js PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
target_link_libraries(p2js subhook v8)

