# Copyright: (C) 2013 RBCS, IIT Italy, Genova
# Authors: Ajaz Bhat
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(KEYWORD "visualizationControl")
PROJECT(${KEYWORD})

# set up to use GTK+
LINK_LIBRARIES(${GtkPlus_LIBRARIES})
LINK_DIRECTORIES(${GTKPLUS_LIBRARY_DIR})
INCLUDE_DIRECTORIES(${GtkPlus_INCLUDE_DIRS})

#search for includes
INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/include
    ${ICUB_INCLUDE_DIRS}
    ${YARP_INCLUDE_DIRS}
    ${OpenCV_INCLUDE_DIRS}    	
    ${IPP_INCLUDE_DIRS}	 
    ${GtkPlus_INCLUDE_DIRS}
)

# Search for source code.
FILE(GLOB folder_source src/*.cpp src/*.cc src/*.c)
FILE(GLOB folder_header include/*.h)
SOURCE_GROUP("Source Files" FILES ${folder_source})
SOURCE_GROUP("Header Files" FILES ${folder_header})

# Set up our main executable.
IF (folder_source)
    ADD_EXECUTABLE(${KEYWORD} 
        ${folder_source} 
        ${folder_header}
    )
    TARGET_LINK_LIBRARIES( ${KEYWORD}
      ${YARP_LIBRARIES}
      ${IPP_LIBRARIES}
      ${GTKPLUS_LINK_FLAGS}
      ${GtkPlus_LIBRARIES}
    )	
    INSTALL_TARGETS(/bin ${KEYWORD})
	
ELSE (folder_source)
    MESSAGE(FATAL_ERROR "No source code files found. Please add something")
	
ENDIF (folder_source)


	

