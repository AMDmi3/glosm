# Depends
FIND_PACKAGE(GLUT REQUIRED)

# Targets
INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ../libglosm-client/include ../libglosm-server/include ../libglosm-geomgen/include)
ADD_DEFINITIONS(-DTESTDATA=\"${PROJECT_SOURCE_DIR}/testdata/glosm.osm\")

ADD_EXECUTABLE(SimpleViewer SimpleViewer.cc)
TARGET_LINK_LIBRARIES(SimpleViewer glosm-client glosm-server glosm-geomgen ${GLUT_LIBRARY})
