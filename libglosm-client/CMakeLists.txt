# Depends
FIND_PACKAGE(Threads REQUIRED)

# Checks
IF(NOT CMAKE_CROSSCOMPILING)
	INCLUDE(CheckFunctionExists)
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
	CheckGL.cc
	FirstPersonViewer.cc
	GeometryLayer.cc
	GeometryTile.cc
	GPXLayer.cc
	GPXTile.cc
	MercatorProjection.cc
	mglu.cc
	OrthoViewer.cc
	Projection.cc
	SphericalProjection.cc
	TerrainLayer.cc
	TerrainTile.cc
	TileManager.cc
)

# We check whether GL functions != NULL there. These funcions may
# really be either function pointers (which are perfectly OK to
# compare with NULL) or normal functions (for which warning will
# be generated, while comparison still gives correct result).
# We have to ignore the warning because there can be mix of both
SET_SOURCE_FILES_PROPERTIES(
	CheckGL.cc
	PROPERTIES
	COMPILE_FLAGS "-Wno-address"
)

SET(HEADERS
	glosm/CheckGL.hh
	glosm/FirstPersonViewer.hh
	glosm/GeometryLayer.hh
	glosm/GeometryTile.hh
	glosm/GPXLayer.hh
	glosm/GPXTile.hh
	glosm/Layer.hh
	glosm/MercatorProjection.hh
	glosm/OrthoViewer.hh
	glosm/Projection.hh
	glosm/Renderable.hh
	glosm/SphericalProjection.hh
	glosm/TerrainLayer.hh
	glosm/TerrainTile.hh
	glosm/Tile.hh
	glosm/TileManager.hh
	glosm/VertexBuffer.hh
	glosm/Viewer.hh
)

INCLUDE_DIRECTORIES(. ../libglosm-server ${EXPAT_INCLUDE_DIR} ${GL_INCLUDE_DIRS})

ADD_LIBRARY(glosm-client SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-client glosm-server ${GL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Installation
# SET_TARGET_PROPERTIES(glosm-client PROPERTIES SOVERSION 1)
INSTALL(FILES ${HEADERS} DESTINATION include/glosm)
INSTALL(FILES glosm/util/gl.h DESTINATION include/glosm/util)

IF(NOT CMAKE_CROSSCOMPILING)
	INSTALL(TARGETS glosm-client LIBRARY DESTINATION ${LIBDIR})
ENDIF(NOT CMAKE_CROSSCOMPILING)
