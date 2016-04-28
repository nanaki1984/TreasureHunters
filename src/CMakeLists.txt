set(SUB_DIRS Core Game Managers Math Network)

foreach(DIR ${SUB_DIRS})
	add_subdirectory(${DIR})
	set(LIBS ${LIBS} lib${DIR})
endforeach()

set(LIBS ${LIBS} PARENT_SCOPE)
