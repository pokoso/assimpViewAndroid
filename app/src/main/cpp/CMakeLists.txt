# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

include_directories(
        .
        opencv2
)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
        NativeAssimpDrawer

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        assimpDrawer.cpp
        ModelObject.cpp
        misc.cpp
        modelAssimp.cpp
        myJNIHelper.cpp
        myShader.cpp
        myGLCamera.cpp
        myGLFunctions.cpp
        myGLRenderer.cpp
        gestureClass.cpp
        Animator.cpp
        AnimationLoader.cpp
        Animation.cpp
        Utils.cpp
)

find_library(
        log-lib
        log
)

find_library(
        android-lib
        android
)

add_library(
        opencv-lib
        SHARED
        IMPORTED )

set_target_properties(
        opencv-lib
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI}/libopencv_java3.so
)

add_library(
        assimp-lib
        SHARED
        IMPORTED )

set_target_properties(
        assimp-lib
        PROPERTIES IMPORTED_LOCATION
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/${ANDROID_ABI}/libassimp.so
)

target_link_libraries(
        NativeAssimpDrawer
        ${log-lib}
        ${android-lib}
        GLESv2
        opencv-lib
        assimp-lib
)