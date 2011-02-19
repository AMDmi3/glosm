IF(BUILD_VIEWER_SDL)
	# Depends
	FIND_PACKAGE(SDL REQUIRED)

	# Targets
	SET(SOURCES
		SDLMain.cc
		GlosmViewer.cc
	)

	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${SDL_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ../libglosm-client/include ../libglosm-server/include ../libglosm-geomgen/include)

	ADD_EXECUTABLE(glosm-viewer-sdl ${SOURCES})
	TARGET_LINK_LIBRARIES(glosm-viewer-sdl glosm-client glosm-server glosm-geomgen ${SDL_LIBRARY} ${GLEW_LIBRARY})

	# Installation
	INSTALL(TARGETS glosm-viewer-sdl RUNTIME DESTINATION ${BINDIR})
ENDIF(BUILD_VIEWER_SDL)

IF(BUILD_VIEWER_GLUT)
	# Depends
	FIND_PACKAGE(GLUT REQUIRED)

	# Targets
	SET(SOURCES
		GLUTMain.cc
		GlosmViewer.cc
	)

	INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ../libglosm-client/include ../libglosm-server/include ../libglosm-geomgen/include)

	ADD_EXECUTABLE(glosm-viewer-glut ${SOURCES})
	TARGET_LINK_LIBRARIES(glosm-viewer-glut glosm-client glosm-server glosm-geomgen ${GLUT_LIBRARIES} ${GLEW_LIBRARY})

	# Installation
	INSTALL(TARGETS glosm-viewer-glut RUNTIME DESTINATION ${BINDIR})
ENDIF(BUILD_VIEWER_GLUT)
