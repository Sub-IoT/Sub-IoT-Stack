#Module specific parameters and options can be defined using
#MODULE_OPTION and MODULE_PARAMETER
#See cmake/module_macros.cmake for more information

#Export the module-specific header files to the application by using
#GLOBAL_INCLUDE_DIRECTORIES


#By convention, each module should generate a single 'static' library that can be included by the application
#eg ADD_LIBRARY(d7aoss STATIC file1.c file2.c ...)