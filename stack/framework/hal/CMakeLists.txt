#Make sure that the HAL interface is reacheable by the entire source tree
GLOBAL_INCLUDE_DIRECTORIES("inc")

#note: this does not include any chip code. 
#see note in 'chips' directory in the CMakeLists.txt in the 'chips' directory
ADD_SUBDIRECTORY("chips")

ADD_SUBDIRECTORY("platforms")
