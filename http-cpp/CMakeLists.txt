cmake_minimum_required(VERSION 2.8)

project(http CXX)
message("configure: http-cpp library")

if(MSVC)
	set(CURL_HOME ${HTTP_CPP_3RD_PARTY_DIR}/curl)
	set(CURL_INCLUDE_DIRS "${CURL_HOME}/include")
	set(CURL_LIBRARIES "${CURL_HOME}/lib/libcurl_a.lib")
	add_definitions(-DCURL_STATICLIB)
	message("-- using curl lib: ${CURL_HOME}")
else(MSVC)
	find_package(curl REQUIRED)
endif(MSVC)

include_directories(
    ${CURL_INCLUDE_DIRS}
)

add_library(
	http-cpp SHARED
	client.cpp
	client.hpp
	http-cpp.hpp
	operation.cpp
	operation.hpp
	status.cpp
	status.hpp
)

target_link_libraries(
    http-cpp
    ${CURL_LIBRARIES}
)