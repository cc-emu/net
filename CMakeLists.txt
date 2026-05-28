cmake_minimum_required(VERSION 3.15)
project(ccemu-net)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_JS_INC})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/node_modules/node-addon-api)
include_directories(vendor/raknet)

# Source files
file(GLOB SOURCE_FILES src/*.cpp)

# Addon library
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES} ${CMAKE_JS_SRC})

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    ${CMAKE_JS_LIB}
)

# Include RakNet sources (static library)
file(GLOB RAKNET_SOURCES vendor/raknet/*.cpp)
add_library(raknet STATIC ${RAKNET_SOURCES})
set_target_properties(raknet PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(${PROJECT_NAME} raknet)

# Compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    target_link_libraries(${PROJECT_NAME} ws2_32.lib)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
endif()

# Rename output to .node
set_target_properties(${PROJECT_NAME} PROPERTIES 
    PREFIX "" 
    SUFFIX ".node"
)
