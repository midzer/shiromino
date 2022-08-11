install(CODE "

set(PRE_EXCLUDES
\"^api-ms-win\"
)

set(POST_EXCLUDES
\"system32\"
)

file(GET_RUNTIME_DEPENDENCIES
	EXECUTABLES \$<TARGET_FILE:${GAME_EXECUTABLE}>
	PRE_EXCLUDE_REGEXES
		\${PRE_EXCLUDES}
	POST_EXCLUDE_REGEXES
		\${POST_EXCLUDES}
	RESOLVED_DEPENDENCIES_VAR RESOLVED_DEPENDENCIES
	UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
)
file(INSTALL
	DESTINATION \"\${CMAKE_INSTALL_PREFIX}\"
	TYPE SHARED_LIBRARY
	FOLLOW_SYMLINK_CHAIN
	FILES \${RESOLVED_DEPENDENCIES}
)

")
