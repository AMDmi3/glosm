# Targets
SET(SOURCES
	src/DefaultGeometryGenerator.cc
)

SET(HEADERS
	include/glosm/DefaultGeometryGenerator.hh
)

INCLUDE_DIRECTORIES(include ../libglosm-server/include)

ADD_LIBRARY(geomgen-default SHARED ${SOURCES})
TARGET_LINK_LIBRARIES(geomgen-default glosm-server)

# Installation
# We don't have even a sight of stable API, so don't use soversion
# and don't install headers yet. May be enabled if there are any
# third party users of libglosm
#SET_TARGET_PROPERTIES(geomgen-default PROPERTIES SOVERSION 1)
#INSTALL(FILES ${HEADERS} DESTINATION include/glosm)

INSTALL(TARGETS geomgen-default LIBRARY DESTINATION lib)
