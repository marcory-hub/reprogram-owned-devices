# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/md5/esp/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/Users/md5/esp/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader"
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix"
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/tmp"
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/src"
  "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/md5/Developer/reprogram-owned-devices/firmware/SAD00006D-ai-cam/yolo11n-detect/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
