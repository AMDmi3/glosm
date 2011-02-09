# Targets
INCLUDE_DIRECTORIES(../libglosm-client/include ../libglosm-server/include)

ADD_EXECUTABLE(ProjectionTest ProjectionTest.cc)
TARGET_LINK_LIBRARIES(ProjectionTest glosm-server glosm-client)

ADD_EXECUTABLE(ProjectionBench ProjectionBench.cc)
TARGET_LINK_LIBRARIES(ProjectionBench glosm-server glosm-client)

ADD_EXECUTABLE(TypeTest TypeTest.cc)
TARGET_LINK_LIBRARIES(TypeTest glosm-server)

ADD_EXECUTABLE(ExceptionTest ExceptionTest.cc)
TARGET_LINK_LIBRARIES(ExceptionTest glosm-server)

# Tests
ADD_TEST(ProjectionTest ProjectionTest)
ADD_TEST(TypeTest TypeTest)
ADD_TEST(ExceptionTest ExceptionTest)
