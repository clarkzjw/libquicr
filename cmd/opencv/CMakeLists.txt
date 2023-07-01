# CMakeList.txt : CMake project for opencv_test, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#set(OpenCV_DIR "D:\\OpenCV\\4.8.0\\opencv\\build")
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV library status:")
message(STATUS "    config: ${OpenCV_DIR}")
message(STATUS "    version: ${OpenCV_VERSION}")
message(STATUS "    libraries: ${OpenCV_LIBS}")
message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("opencv_test")

# Add source to this project's executable.
add_executable (opencv_test "opencv_test.cpp" "opencv_test.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET opencv_test PROPERTY CXX_STANDARD 20)
endif()

include_directories(${${OpenCV_INCLUDE_DIRS}})
target_link_libraries(opencv_test ${OpenCV_LIBS})
# TODO: Add tests and install targets if needed.
