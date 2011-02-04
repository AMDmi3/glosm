# Depends
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

IF(USE_GLEW)
	FIND_PACKAGE(GLEW REQUIRED)
	ADD_DEFINITIONS(-DUSE_GLEW)
ENDIF(USE_GLEW)

# Checks
IF(NOT CMAKE_CROSSCOMPILING)
	SET(CMAKE_EXTRA_INCLUDE_FILES "math.h")
	SET(CMAKE_REQUIRED_LIBRARIES m)
	CHECK_FUNCTION_EXISTS(sincos HAVE_SINCOS)
	SET(CMAKE_REQUIRED_LIBRARIES)
	SET(CMAKE_EXTRA_INCLUDE_FILES)
ENDIF(NOT CMAKE_CROSSCOMPILING)

IF(HAVE_SINCOS)
	ADD_DEFINITIONS(-DHAVE_SINCOS)
ENDIF(HAVE_SINCOS)

# Targets
SET(SOURCES
	src/FirstPersonViewer.cc
	src/GeometryLayer.cc
	src/GeometryTile.cc
	src/MercatorProjection.cc
	src/SphericalProjection.cc
	src/OrthoViewer.cc
	src/Projection.cc
	src/SimpleVertexBuffer.cc
	src/VBO.cc
	src/TileManager.cc
)

SET(HEADERS
)

INCLUDE_DIRECTORIES(include ../libglosm-server/include ${EXPAT_INCLUDE_DIR} ${GLEW_INCLUDE_DIR})

ADD_LIBRARY(glosm-client SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-client glosm-server ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${GLEW_LIBRARY})

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(glosmclient PROPERTIES SOVERSION 1)
#INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

IF(NOT CMAKE_CROSSCOMPILING)
	INSTALL(TARGETS glosm-client LIBRARY DESTINATION ${LIBDIR})
ENDIF(NOT CMAKE_CROSSCOMPILING)
