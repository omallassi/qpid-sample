project(qpid-sample)
set(target qpid-sample)
include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Debug)

CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	message(STATUS "use C++11 Compiler")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

include_directories(CMAKE_CURRENT_SOURCE_DIR/../../qpid-proton/proton-c/bindings/cpp/include)

add_executable(${target} hello_world_messaging_handler.cpp)
target_link_libraries(${target} /home/omallassi/qpid-proton/build/proton-c/libqpid-proton.so /home/omallassi/qpid-proton/build/proton-c/bindings/cpp/libqpid-proton-cpp.so)