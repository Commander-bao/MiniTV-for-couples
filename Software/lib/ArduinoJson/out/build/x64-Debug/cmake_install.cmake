# Install script for directory: D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson/out/build/x64-Debug/src/cmake_install.cmake")
  include("D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson/out/build/x64-Debug/extras/tests/cmake_install.cmake")
  include("D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson/out/build/x64-Debug/extras/fuzzing/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "D:/OneDrive/作品/Arduino ESP32太空人天气时钟/程序/4M Flash版本-2021-5-21正式版/4M_Flash_CLOCK/4M_Flash_CLOCK/libraries/ArduinoJson/out/build/x64-Debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
