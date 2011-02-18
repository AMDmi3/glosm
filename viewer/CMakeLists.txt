# Depends
FIND_PACKAGE(GLUT REQUIRED)

IF(USE_GLEW)
	FIND_PACKAGE(GLEW REQUIRED)
	ADD_DEFINITIONS(-DUSE_GLEW)
ENDIF(USE_GLEW)

# Targets
SET(SOURCES
	Main.cc
	GlosmViewer.cc
)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLEW_INCLUDE_DIR} ../libglosm-client/include ../libglosm-server/include ../libglosm-geomgen/include)

ADD_EXECUTABLE(glosm-viewer ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-viewer glosm-client glosm-server glosm-geomgen ${GLUT_LIBRARY} ${GLEW_LIBRARY})

# Installation
INSTALL(TARGETS glosm-viewer RUNTIME DESTINATION ${BINDIR})
