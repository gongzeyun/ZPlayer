# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp )

#add ffmpeg library start
add_library( avcodec-58 SHARED IMPORTED )
add_library( avfilter-7-lib SHARED IMPORTED )
add_library( avformat-58-lib SHARED IMPORTED )
add_library( avutil-56-lib SHARED IMPORTED )
add_library( swresample-3-lib SHARED IMPORTED )
add_library( swscale-5-lib SHARED IMPORTED )
##add ffmpeg library end

#add ffmpeg include directories
include_directories(src/main/cpp/)


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

set_target_properties(avcodec-58 PROPERTIES IMPORTED_LOCATION
                             ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavcodec-58.so)
set_target_properties(avfilter-7-lib PROPERTIES IMPORTED_LOCATION
                             ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavfilter-7.so)
set_target_properties(avformat-58-lib PROPERTIES IMPORTED_LOCATION
                              ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavformat-58.so)
set_target_properties(avutil-56-lib PROPERTIES IMPORTED_LOCATION
                             ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libavutil-56.so)
set_target_properties(swresample-3-lib PROPERTIES IMPORTED_LOCATION
                             ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswresample-3.so)
set_target_properties(swscale-5-lib PROPERTIES IMPORTED_LOCATION
                              ${CMAKE_SOURCE_DIR}/libs/armeabi-v7a/libswscale-5.so)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       android
                       avcodec-58
                       avfilter-7-lib
                       avformat-58-lib
                       avutil-56-lib
                       swresample-3-lib
                       swscale-5-lib
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )