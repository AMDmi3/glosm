# Targets
INCLUDE_DIRECTORIES(../libglosm)

ADD_EXECUTABLE(ProjectionTest ProjectionTest.cc)
TARGET_LINK_LIBRARIES(ProjectionTest glosm)

# Tests
ADD_TEST(ProjectionTest ProjectionTest)
