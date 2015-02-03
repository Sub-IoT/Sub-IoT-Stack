#Add the 'BUILD_MODULES' option
OPTION(BUILD_MODULES "Build Modules" ON)
LIST_SUBDIRS(CUR_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
#Add conditional options for all subdirs (which are individual modules)
FOREACH(__dir ${CUR_DIRS})
    STRING(TOUPPER ${__dir} __upper_name)
    SETOPTION_IF(MODULE_${__upper_name} "Build Module ${__dir}" OFF BUILD_MODULES)
    UNSET(__upper_name)
ENDFOREACH()

#Define MACRO's that allows individual modules to add conditional (cache) parameters and options
MACRO(MODULE_PARAM var value type doc)
    SET(MODULE_PARAM_LIST "${MODULE_PARAM_LIST};${var}" CACHE INTERNAL "")
    #Set entry in cache if the module is being loaded
    SETCACHE_IF(${var} ${value} ${type} ${doc} ${MODULE_PREFIX})
ENDMACRO()
MACRO(MODULE_OPTION option doc default)
    MODULE_PARAM( ${option} ${default} BOOL ${doc})
ENDMACRO()

#Hide the parameters declared by the modules during the previous run
#Modules that are enabled will re-enable their parameters during declaration (eg by calling MODULE_PARAM)
HIDE_PARAMETERS(MODULE_PARAM_LIST)

#Finally Load the individual modules
FOREACH(__dir ${CUR_DIRS})
    GEN_PREFIX(MODULE_PREFIX "MODULE" ${__dir})
    IF(BUILD_MODULES AND ${MODULE_PREFIX})
	ADD_SUBDIRECTORY(${__dir})
    ENDIF()	
    UNSET(MODULE_PREFIX)
ENDFOREACH()