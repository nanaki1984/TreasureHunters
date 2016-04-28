CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(TreasureHunters)

# Determin the system to build for. Do that first
if(EMSCRIPTEN)
	message("++ Building for emscripten")
elseif(WIN32)
	if(NOT WINDOWS)
		set(WINDOWS TRUE)
		message("++ Building for windows")
	endif()
elseif(UNIX AND NOT APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*Linux")
		if(NOT ANDROID)
			set(LINUX TRUE)
			message("++ Building for Linux")
		else()
			message("++ Building for Android")
		endif()
	else()
		message(FATAL_ERROR "Unknown unix")
	endif()
elseif(APPLE)
	if(CMAKE_SYSTEM_NAME MATCHES ".*MacOS.*" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
		set(MACOS TRUE)
		message("++ Building for MacOS")
	else()
		message(FATAL_ERROR "Unknown apple")
	endif()
else()
	message(FATAL_ERROR "Unknown system")
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_HAS_EXCEPTIONS=0)

	string(REPLACE "/EHsc" "/EHs-c-" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

	set(CXX_FLAGS "/GR- ")
	set(COMPILER_FLAGS "")
	set(LINKER_FLAGS "")
else()
	set(CXX_FLAGS "-Wno-warn-absolute-paths -std=c++11 -stdlib=libc++ -fno-exceptions -fno-rtti ")
	set(COMPILER_FLAGS "")
	set(LINKER_FLAGS "")
endif()

# IDE/Platform related settings
if(EMSCRIPTEN)
	set(CMAKE_LINK_LIBRARY_SUFFIX "")
	set(CMAKE_STATIC_LIBRARY_PREFIX "")
	set(CMAKE_STATIC_LIBRARY_SUFFIX ".bc")
	set(CMAKE_SHARED_LIBRARY_PREFIX "")
	set(CMAKE_SHARED_LIBRARY_SUFFIX ".bc")
	set(CMAKE_EXECUTABLE_SUFFIX ".html")
	set(CMAKE_FIND_LIBRARY_PREFIXES "")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".bc")

	set(LINKER_FLAGS "${LINKER_FLAGS} -s LEGACY_GL_EMULATION=1 ")
elseif(MACOS)
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
	set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
endif()

# SSE
if(MSVC)
	# defaults to SSE2
else()
	if(LINUX OR MACOS OR WINDOWS)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -msse4 ")
	elseif(NOT EMSCRIPTEN)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -mfpu=neon ")
	endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
	add_definitions(-D_DEBUG)

	if(EMSCRIPTEN)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -O0 -s LINKABLE=1 -s VERBOSE=1 ")
		set(LINKER_FLAGS "${LINKER_FLAGS} -O0 -s LINKABLE=1 -s VERBOSE=1 ")
	elseif(MSVC)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} /MDd /Zi /Ob0 /Od /RTC1 ")
		set(LINKER_FLAGS "${LINKER_FLAGS} /debug /INCREMENTAL ")
	else()
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -g3 -O0 ")
	endif()
else() # MinSizeRel, RelWithDebInfo or Release
	if(MACOS)
		add_definitions(-D_DEBUG)
	endif()

	if(EMSCRIPTEN)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -O3 -s FORCE_ALIGNED_MEMORY=1 --closure 1 --llvm-lto 3 ")
		set(LINKER_FLAGS "${LINKER_FLAGS} -O3 -s FORCE_ALIGNED_MEMORY=1 --closure 1 --llvm-lto 3 ")
	elseif(MSVC)
		set(COMPILER_FLAGS "${COMPILER_FLAGS} /MD /O2 /Ob2 ")
		set(LINKER_FLAGS "${LINKER_FLAGS} /INCREMENTAL:NO ")
	else()
		# -flto ?
		set(COMPILER_FLAGS "${COMPILER_FLAGS} -ffast-math -O4 ")

		# Add this because Android compiler complains
		if(ANDROID)
			set(COMPILER_FLAGS "${COMPILER_FLAGS} -fno-data-sections ")
		endif()
	endif()
endif()

# Set the flags to cmake now
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILER_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

if(WIN32)
	set(SYS_LIBS wsock32 ws2_32 winmm )
else()
	set(SYS_LIBS "")
endif()
set(LIBS enet )

include_directories("src" "ext/enet/include")

add_subdirectory(ext)
add_subdirectory(src)

add_library(THShared SHARED dllmain.cc)
add_executable(THServer main.cc)

target_link_libraries(THShared ${LIBS} ${SYS_LIBS})
target_link_libraries(THServer THShared ${SYS_LIBS})
