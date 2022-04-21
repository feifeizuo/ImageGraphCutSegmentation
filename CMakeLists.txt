cmake_minimum_required(VERSION 3.8)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

project(ImageGraphCutSegmentation)

# ITK
FIND_PACKAGE(ITK REQUIRED ITKCommon ITKIOImageBase ITKIOPNG ITKIOMeta
                          ITKImageIntensity ITKImageFeature
                          ITKMathematicalMorphology
                          ITKBinaryMathematicalMorphology ITKDistanceMap
                          ITKTestKernel)
INCLUDE( ${ITK_USE_FILE} )

if( "${ITK_VERSION_MAJOR}" LESS 4 )
  MESSAGE(FATAL_ERROR "You must build ImageGraphCutSegmentation with ITK >= 4.0!")
endif( "${ITK_VERSION_MAJOR}" LESS 4 )

# Boost
#set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.79 COMPONENTS regex date_time system filesystem thread graph REQUIRED)
#INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})

FILE(GLOB GC_HEADERS *.h *.hpp Mask/*.h Mask/*.hpp Mask/ITKHelpers/*.h Mask/ITKHelpers/*.hpp Mask/ITKHelpers/Helpers/*.h Mask/ITKHelpers/Helpers/*.hpp)
FILE(GLOB GC_SOURCES *.cpp Mask/*.cpp Mask/ITKHelpers/*.cpp Mask/ITKHelpers/Helpers/*.cpp)

ADD_LIBRARY(ImageGraphCut SHARED ${GC_HEADERS} ${GC_SOURCES})
TARGET_LINK_LIBRARIES(ImageGraphCut ${Boost_LIBRARIES} ${ITK_LIBRARIES})

# Example
ADD_EXECUTABLE(ImageGraphCutSegmentationExample Examples/ImageGraphCutSegmentationExample.cpp)
TARGET_LINK_LIBRARIES(ImageGraphCutSegmentationExample ${ImageGraphCut} ${Boost_LIBRARIES} ${ITK_LIBRARIES})