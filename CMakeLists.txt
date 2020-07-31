project(signal_slot_test)
cmake_minimum_required(VERSION 3.10)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GoogleTest)
add_executable(sigslot_test ${CMAKE_CURRENT_SOURCE_DIR}/sigslot_test.cpp)
