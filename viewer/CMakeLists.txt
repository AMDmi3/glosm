# Depends
FIND_PACKAGE(GLUT REQUIRED)

# Targets
SET(viewer_SRCS
	Main.cc
)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ../libglosm)

ADD_EXECUTABLE(glosm-viewer ${viewer_SRCS})
TARGET_LINK_LIBRARIES(glosm-viewer glosm ${GLUT_LIBRARY})

# Installation
INSTALL(TARGETS glosm-viewer RUNTIME DESTINATION bin)
