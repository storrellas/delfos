# -- CMake build rules

# Boost stuff
set(Boost_USE_STATIC_LIBS ON CACHE BOOL "Use statically-linked boost libraries")
# '- Note: By default set Boost_USE_STATIC_LIBS to ON (to link them statically
#    to avoid possible problems during execution of the application (not finding
#    the .so files)), but let the user change it (useful for example to use
#    Fedora native boost libraries, or for OSX when using Boost's "locale" to
#    avoid weird "_iconv*" missing symbols when linking")
set(Boost_USE_MULTITHREADED ON CACHE BOOL "Use multithreaded boost libraries")
# WIP ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
# find_package(Boost) # <-- Use this simple line for using only headers
set(Boost_USE_STATIC_RUNTIME ON CACHE BOOL "Use static runtime boost libraries" )
find_package(Boost
	COMPONENTS
		# lexicographically sorted
		date_time
		filesystem
		locale
		log
		program_options
		system
)
if (Boost_FOUND)
	message(STATUS "Boost found:")
	include_directories(${Boost_INCLUDE_DIRS})
	message(STATUS "  Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
	message(STATUS "  Boost_LIBRARIES: ${Boost_LIBRARIES}
	Note: Boost_LIBRARIES can be empty if no COMPONENTS are used.") # Used below
else ()
	message(FATAL_ERROR "Boost not found - Hint(?): ENV{BOOST_ROOT} is $ENV{BOOST_ROOT}")
endif ()

find_package(delfosUtils)
if (delfosUtils_FOUND)
	message(STATUS "delfosUtils found:")
	include_directories(${delfosUtils_INCLUDE_DIRS})
	message(STATUS "  delfosUtils_INCLUDE_DIRS: ${delfosUtils_INCLUDE_DIRS}")
	message(STATUS "  delfosUtils_LIBRARIES: ${delfosUtils_LIBRARIES}")
else ()
	message(FATAL_ERROR "delfosUtils not found")
endif ()

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/workflows
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)



# Generate src fake library
file(GLOB_RECURSE SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
set(SRC_FAKE_LIBRARY_LIST ${SRC_LIST})
# Remove main
list(REMOVE_ITEM SRC_FAKE_LIBRARY_LIST ${PROJECT_SOURCE_DIR}/src/enzo_main.cpp)
list(REMOVE_ITEM SRC_FAKE_LIBRARY_LIST ${PROJECT_SOURCE_DIR}/src/twilio_mockup_main.cpp)
add_library(SRC_FAKE_LIBRARY OBJECT ${SRC_FAKE_LIBRARY_LIST})

# Enzo
##################
add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:SRC_FAKE_LIBRARY> ${PROJECT_SOURCE_DIR}/src/enzo_main.cpp)

# Link the executable to the required libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
	${Boost_LIBRARIES}
#	Other required 3rd party libraries appear below (examples commented):
	${delfosUtils_LIBRARIES}
#	${CMAKE_THREAD_LIBS_INIT}
#	${OpenIGTLink_LIBRARIES}
#	${ITK_LIBRARIES}
#	${VTK_LIBRARIES}
)

set(TARGET_NAME ${PROJECT_NAME}Targets)
install(TARGETS ${PROJECT_NAME} # EXPORT ${TARGET_NAME}
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
	RUNTIME DESTINATION bin
#	INCLUDES DESTINATION include
)

# -- eof
