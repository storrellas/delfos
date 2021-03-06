# -- CMake build rules

cmake_minimum_required(VERSION 2.8)

set(PROJECT_NAME "enzo")
set(PROJECT_TYPE "CXX")
project(${PROJECT_NAME} ${PROJECT_TYPE})

message(STATUS "CMAKE_C_PLATFORM_ID is ${CMAKE_C_PLATFORM_ID}")
message(STATUS "CMAKE_SYSTEM_NAME is ${CMAKE_SYSTEM_NAME}")

if(CMAKE_CONFIGURATION_TYPES) # multiconfig generator?
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Profile" CACHE String "" FORCE)
	set(CMAKE_CONFIGURATION_TYPES Release CACHE String "Release" FORCE)
	message(STATUS "CMAKE_CONFIGURATION_TYPES is ${CMAKE_CONFIGURATION_TYPES}")
else()
  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE: Defaulting to Release")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
  endif()
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
  # set the valid options for cmake-gui drop-down list
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;Profile")
endif()

# set(BUILD_TESTING OFF CACHE BOOL OFF FORCE)
# message(STATUS "BUILD_TESTING is ${BUILD_TESTING}")

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	set(MyUsername $ENV{username})
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	set(MyUsername $ENV{USER})
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	set(MyUsername $ENV{USER})
else()
	set(MyUsername "Unknown")
endif()
message(STATUS "Your username (MyUsername variable) is ${MyUsername}")
set(def_CMAKE_MYUSERNAME CMAKE_MYUSERNAME=${MyUsername})
message(STATUS "> def_CMAKE_MYUSERNAME is ${def_CMAKE_MYUSERNAME}")
set(MyUserActualFullName "N.N.")
if(${MyUsername} MATCHES "frede" OR "frederic" OR "frederic.perez")
	set(MyUserActualFullName "Frederic-Perez")
endif()
message(STATUS "MyUserActualFullName is ${MyUserActualFullName}")

if(CMAKE_MAJOR_VERSION LESS 3)
	SITE_NAME(MyHostname)
else()
	cmake_host_system_information(RESULT MyHostname QUERY HOSTNAME)
endif()
message(STATUS "Your host name (MyHostname variable) is ${MyHostname}")
set(def_CMAKE_MYHOSTNAME CMAKE_MYHOSTNAME=${MyHostname}) 
message(STATUS "> def_CMAKE_MYHOSTNAME is ${def_CMAKE_MYHOSTNAME}")

if("${CMAKE_SYSTEM_NAME}" MATCHES "Windows")
	if(MSVC)
		message(STATUS "Entering Windows/MSVC branch.")
		string(CONCAT CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS "
			"/W4 /WX /EHa /wd4127 /wd4351 "
			"/D${def_CMAKE_MYUSERNAME} /D${def_CMAKE_MYHOSTNAME}")
		# '- http://msdn.microsoft.com/en-US/library/thxezb7y.aspx:
		#		  /wd n  Disables the compiler warning that is specified in n.
		#			For example, /wd4326 disables compiler warning C4351.
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:MULTIPLE")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")
		message(STATUS "CMAKE_EXE_LINKER_FLAGS are: " ${CMAKE_EXE_LINKER_FLAGS})
	else()
		message(ERROR "Uncontemplated case.")
	endif()
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	if(CMAKE_COMPILER_IS_GNUCXX)
		message(STATUS "Entering Linux/CMAKE_COMPILER_IS_GNUCXX branch.")
# TODO: Use string(CONCAT CMAKE_CXX_FLAGS after updating cmake-gui!
		string(CONCAT CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -g3 -Wall -Wextra "
			# "-Weffc++ " 
			# "-Werror=shadow "
			#"-Werror=unused-value -Werror=unused-variable "
			# "-Werror=unused-parameter "
			#"-Werror=return-type "
			#"-Werror=return-type "
			"-std=c++11 -D${def_CMAKE_MYUSERNAME} -D${def_CMAKE_MYHOSTNAME}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	else()
		message(STATUS "Entering Linux/else branch. We assume clang++.")
# TODO: Use string(CONCAT CMAKE_CXX_FLAGS after updating cmake-gui!
		string(CONCAT CMAKE_CXX_FLAGS
			"${CMAKE_CXX_FLAGS} -Wall -Weffc++ -Wcomment "
			"-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic "
			"-Wno-global-constructors -Wno-exit-time-destructors "
			"-std=c++11 -D ${def_CMAKE_MYUSERNAME} -D ${def_CMAKE_MYHOSTNAME}")
		set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread")
	endif()
elseif("${CMAKE_SYSTEM_NAME}" MATCHES "Darwin")
	message(STATUS "Entering Darwin branch. We assume clang++.")
	string(CONCAT CMAKE_CXX_FLAGS
		"${CMAKE_CXX_FLAGS} -Wall -std=c++11 "
		"-D${def_CMAKE_MYUSERNAME} -D${def_CMAKE_MYHOSTNAME}")
else()
	message(FATAL_ERROR "Unknown system")
endif()

message(STATUS "CMAKE_CXX_FLAGS are:" ${CMAKE_CXX_FLAGS})

option(BUILD_TESTING "Build the testing tree" ON)
if(BUILD_TESTING)
	set(SITE ${MyUserActualFullName}-at-${MyHostname})
	message(STATUS "SITE is ${SITE}")

	message(STATUS "CMAKE_CXX_COMPILER is ${CMAKE_CXX_COMPILER}")
	message(STATUS "CMAKE_CXX_COMPILER_ID is ${CMAKE_CXX_COMPILER_ID}")
	message(STATUS "CMAKE_CXX_COMPILER_VERSION is ${CMAKE_CXX_COMPILER_VERSION}")
	if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
		# See https://en.wikipedia.org/wiki/Visual_C%2B%2B for the "conversion"
		if (MSVC10)
			set(MyCxx "VC++-2010")
		elseif(MSVC11)
			set(MyCxx "VC++-2012")
		elseif(MSVC12)
			set(MyCxx "VC++-2013")
		elseif(MSVC14)
			set(MyCxx "VC++-2015")
		else()
			set(MyCxx "VC++-????")
		endif()
	else()
		set(MyCxx "${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}")
	endif()
	set(BUILDNAME
		${PROJECT_NAME}-${CMAKE_SYSTEM_NAME}-${MyCxx}-${CMAKE_BUILD_TYPE})
	message(STATUS "BUILDNAME is ${BUILDNAME}")
	
	include(CTest)
	# '- To build Continuous, Experimental, etc., and enables submission to
	#    dashboards. 
	#    From http://www.cmake.org/Wiki/CMake/Testing_With_CTest:
	#    "This module will automatically call enable_testing() ... so you no
	#    longer have to do so ..."
endif(BUILD_TESTING)

#set(CMAKE_VERBOSE_MAKEFILE ON)
# '- to get more verbose output from Makefile builds and show each command line
#    as it is launched (for example, to display the compiler flags used to
#    compile!)

include(GNUInstallDirs)

add_subdirectory(src)
#add_subdirectory(twilio_mockup)
add_subdirectory(resources)

# -- eof
