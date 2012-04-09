# Depends
FIND_PACKAGE(EXPAT REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

# Type size checks
INCLUDE(CheckTypeSize)

SET(CMAKE_EXTRA_INCLUDE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/glosm/osmtypes.h")
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

IF(WITH_GLES)
	ADD_DEFINITIONS(-DWITH_GLES)
ENDIF(WITH_GLES)

# Targets
SET(SOURCES
	BBox.cc
	DummyHeightmap.cc
	Exception.cc
	Geometry.cc
	GeometryOperations.cc
	Guard.cc
	ParsingHelpers.cc
	PreloadedGPXDatasource.cc
	PreloadedXmlDatasource.cc
	SRTMDatasource.cc
	Timer.cc
	XMLParser.cc
	WayMerger.cc
)

SET(HEADERS
)

INCLUDE_DIRECTORIES(. ${EXPAT_INCLUDE_DIR})

ADD_LIBRARY(glosm-server SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-server ${EXPAT_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(glosmserver PROPERTIES SOVERSION 1)
#INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

IF(NOT CMAKE_CROSSCOMPILING)
	INSTALL(TARGETS glosm-server LIBRARY DESTINATION ${LIBDIR})
ENDIF(NOT CMAKE_CROSSCOMPILING)
