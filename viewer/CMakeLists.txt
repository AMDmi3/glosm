# Depends
FIND_PACKAGE(GLUT REQUIRED)

# Targets
SET(SOURCES
	Main.cc
)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ../libglosm-client/include ../libglosm-server/include ../libgeomgen-default/include)

ADD_EXECUTABLE(glosm-viewer ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-viewer glosm-client glosm-server geomgen-default ${GLUT_LIBRARY})

# Installation
INSTALL(TARGETS glosm-viewer RUNTIME DESTINATION ${BINDIR})
