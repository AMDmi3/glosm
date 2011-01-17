# Targets
INCLUDE_DIRECTORIES(../libglosm)

ADD_EXECUTABLE(ProjectionTest ProjectionTest.cc)
TARGET_LINK_LIBRARIES(ProjectionTest glosm)

ADD_EXECUTABLE(TypeTest TypeTest.cc)
TARGET_LINK_LIBRARIES(TypeTest glosm)

# Tests
ADD_TEST(ProjectionTest ProjectionTest)
ADD_TEST(TypeTest TypeTest)
