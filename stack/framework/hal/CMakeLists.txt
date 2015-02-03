#Make sure that the HAL interface is reacheable by the entire source tree

#GET_PROPERTY(__includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE(AUTHOR_WARNING "Includes@${CMAKE_CURRENT_SOURCE_DIR}: ${__includes}")

GLOBAL_INCLUDE_DIRECTORIES("inc")

#GET_PROPERTY(__includes DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#MESSAGE(AUTHOR_WARNING "Includes@${CMAKE_CURRENT_SOURCE_DIR}: ${__includes}")

#note: this does not include any chip code. 
#see note in 'chips' directory in the CMakeLists.txt in the 'chips' directory
ADD_SUBDIRECTORY("chips")
ADD_SUBDIRECTORY("platforms")
