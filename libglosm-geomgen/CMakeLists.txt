# Targets
SET(SOURCES
	GeometryGenerator.cc
	MetricBasis.cc
)

SET(HEADERS
	glosm/GeometryGenerator.hh
	glosm/MetricBasis.hh
)

INCLUDE_DIRECTORIES(. ../libglosm-server)

ADD_LIBRARY(glosm-geomgen SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-geomgen glosm-server)

# Installation
# SET_TARGET_PROPERTIES(glosm-geomgen PROPERTIES SOVERSION 1)
INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

IF(NOT CMAKE_CROSSCOMPILING)
	INSTALL(TARGETS glosm-geomgen LIBRARY DESTINATION ${LIBDIR})
ENDIF(NOT CMAKE_CROSSCOMPILING)
