# Depends
FIND_PACKAGE(OpenGL REQUIRED)

# Targets
SET(SOURCES
	src/FirstPersonViewer.cc
	src/GeometryLayer.cc
	src/GeometryTile.cc
	src/MercatorProjection.cc
	src/OrthoViewer.cc
	src/Projection.cc
	src/SimpleVertexBuffer.cc
	src/VBO.cc
	src/StaticQuadtree.cc
)

SET(HEADERS
)

INCLUDE_DIRECTORIES(include ../libglosm-server/include ${EXPAT_INCLUDE_DIR})

ADD_LIBRARY(glosm-client SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-client glosm-server ${OPENGL_LIBRARIES})

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(glosmclient PROPERTIES SOVERSION 1)
#INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

INSTALL(TARGETS glosm-client LIBRARY DESTINATION ${LIBDIR})
