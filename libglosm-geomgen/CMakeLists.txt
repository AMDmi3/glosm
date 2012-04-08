# Targets
SET(SOURCES
	GeometryGenerator.cc
	MetricBasis.cc
)

SET(HEADERS
	glosm/GeometryGenerator.hh
)

INCLUDE_DIRECTORIES(. ../libglosm-server)

ADD_LIBRARY(glosm-geomgen SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-geomgen glosm-server)

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(glosm-geomgen PROPERTIES SOVERSION 1)
#INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

IF(NOT CMAKE_CROSSCOMPILING)
	INSTALL(TARGETS glosm-geomgen LIBRARY DESTINATION ${LIBDIR})
ENDIF(NOT CMAKE_CROSSCOMPILING)
