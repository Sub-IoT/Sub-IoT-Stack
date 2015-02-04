include(${CMAKE_SOURCE_DIR}/cmake/app_macros.cmake)

#Add the 'BUILD_APPLICATIONS' option
OPTION(BUILD_APPLICATIONS "Build Applications" ON)
LIST_SUBDIRS(CUR_DIRS ${CMAKE_CURRENT_SOURCE_DIR})
#Add conditional options for all subdirs (which are individual applications)
FOREACH(__dir ${CUR_DIRS})
    STRING(TOUPPER ${__dir} __upper_name)
    SETOPTION_IF(APP_${__upper_name} "Build Application ${__dir}" OFF BUILD_APPLICATIONS)
    UNSET(__upper_name)
ENDFOREACH()

#Hide the parameters declared by the applications during the previous run
#Applications that are enabled will re-enable their parameters during declaration (eg by calling APP_PARAM)
HIDE_PARAMETERS(APP_PARAM_LIST)

#Finally Load the individual applications
FOREACH(__dir ${CUR_DIRS})
    GEN_PREFIX(APP_PREFIX "APP" ${__dir})
    IF(BUILD_APPLICATIONS AND ${APP_PREFIX})
	ADD_SUBDIRECTORY(${__dir})
    ENDIF()	
    UNSET(APP_PREFIX)
ENDFOREACH()