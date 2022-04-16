cmake_minimum_required(VERSION 3.14)

project(Cerberus LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
)

add_library(Cerberus SHARED
  Cerberus_global.h
  cerberus.cpp
  cerberus.h
  cpp.astylerc
  src/thread/thread.h
  src/thread/thread.cpp
  src/time/time.h
  src/time/time.cpp
  src/mutex/mutex.h
  src/mutex/mutex.cpp
  src/mutex/mutexlocker.h
  src/mutex/mutexlocker.cpp
  src/message/slot/baseslot.h
  src/message/slot/baseslot.cpp
  src/message/messagequeue.h
  src/message/messagequeue.cpp
  src/message/message.h
  src/message/message.cpp
  src/exception/exception.h
  src/exception/exception.cpp
  src/exception/exceptioncatalog.h
  src/define.h
  src/message/slot/charslot.h
  src/message/slot/charslot.cpp
)

target_compile_definitions(Cerberus PRIVATE CERBERUS_LIBRARY)

add_executable(
  test/test main.cpp 
  test/threadtest.cpp
)

find_library(GTEST_LIBRARY NAMES gtest PATHS "/usr/lib" PATH_SUFFIXES lib)
target_link_libraries(test LINK_PUBLIC ${GTEST_LIBRARY})

find_library(GTEST_MAIN_LIBRARY NAMES gtest_main PATHS "/usr/lib" PATH_SUFFIXES lib)
target_link_libraries(test LINK_PUBLIC ${GTEST_MAIN_LIBRARY})
