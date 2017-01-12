CXXFLAGS+=-I../src/

TESTS= \
	geometry_test

all: ${TESTS}

clean:
	${RM} -rf ${TESTS} *.o

geometry_test: geometry_test.o ../src/mwtouch-geometry.o

