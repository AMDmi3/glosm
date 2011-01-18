# Depends
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(X11 REQUIRED)
FIND_PACKAGE(PNG REQUIRED)

# Targets
SET(SOURCES
	Main.cc
	PBuffer.cc
	PngWriter.cc
	PixelBuffer.cc
)

INCLUDE_DIRECTORIES(${OPENGL_INCLUDE_DIR} ${X11_INCLUDE_DIR} ${PNG_INCLUD_DIR} ../libglosm-client/include ../libglosm-server/include ../libgeomgen-default/include)

ADD_EXECUTABLE(glosm-tiler ${SOURCES})
TARGET_LINK_LIBRARIES(glosm-tiler glosm-client glosm-server geomgen-default ${X11_X11_LIB} ${OPENGL_gl_LIBRARY} ${PNG_LIBRARIES})

# Installation
INSTALL(TARGETS glosm-tiler RUNTIME DESTINATION bin)
