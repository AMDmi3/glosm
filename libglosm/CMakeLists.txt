# Depends
FIND_PACKAGE(EXPAT REQUIRED)
FIND_PACKAGE(OpenGL REQUIRED)

# Type size checks
INCLUDE(CheckTypeSize)

SET(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/osmtypes.h")
CHECK_TYPE_SIZE("osmid_t" OSMID_T_SIZE)
CHECK_TYPE_SIZE("osmint_t" OSMINT_T_SIZE)
CHECK_TYPE_SIZE("osmlong_t" OSMLONG_T_SIZE)
CHECK_TYPE_SIZE("float" FLOAT_SIZE)
SET(CMAKE_EXTRA_INCLUDE_FILES)

IF (${OSMID_T_SIZE} LESS 4)
	MESSAGE(FATAL_ERROR "osmid_t size too small (likely easily fixable, see osmtypes.h)")
ENDIF(${OSMID_T_SIZE} LESS 4)
IF (${OSMINT_T_SIZE} LESS 4)
	MESSAGE(FATAL_ERROR "osmint_t size too small (likely easily fixable, see osmtypes.h)")
ENDIF(${OSMINT_T_SIZE} LESS 4)
IF (${OSMLONG_T_SIZE} LESS 8)
	MESSAGE(FATAL_ERROR "osmlong_t size too small (likely easily fixable, see osmtypes.h)")
ENDIF(${OSMLONG_T_SIZE} LESS 8)

# Options
OPTION(TRUSTED_XML "Disable XML sanity checking for faster parsing" OFF)

IF(TRUSTED_XML)
	ADD_DEFINITIONS(-DTRUSTED_XML)
ENDIF(TRUSTED_XML)

# Targets
SET(glosm_SRCS
	PreloadedXmlDatasource.cc
	GeometryTile.cc
	GeometryDatasource.cc
	GeometryGenerator.cc
	GeometryLayer.cc
	Geometry.cc
	FirstPersonViewer.cc
	OrthoViewer.cc
	SimpleVertexBuffer.cc
	VBO.cc
	Math.cc
	Projection.cc
	MercatorProjection.cc
)

SET(glosm_HDRS
)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${EXPAT_INCLUDE_DIR})

ADD_LIBRARY(glosm SHARED ${glosm_SRCS})
TARGET_LINK_LIBRARIES(glosm ${OPENGL_LIBRARIES} ${EXPAT_LIBRARY})

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(glosm PROPERTIES SOVERSION 1)
#INSTALL(FILES  ${glosm_HDRS} DESTINATION include/glosm)

INSTALL(TARGETS glosm LIBRARY DESTINATION lib)
