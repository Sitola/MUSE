LIBS= -L${SAGE_DIRECTORY}/lib  -l fsClient `pkg-config --libs libkerat`

CFLAGS = -Wall -Wno-deprecated -I${SAGE_DIRECTORY}/include -I${SAGE_DIRECTORY}/QUANTA/include -I../include `pkg-config --cflags libkerat`
CPPFLAGS = ${CFLAGS}

COMPILER = g++

all: musa_listener.o sage_client.o musa.o
	${COMPILER} ${CPPFLAGS} musa_listener.o sage_client.o musa.o ${LIBS} -o ../bin/musa  

clean:
	rm -f *.o *.orig ../include/*.o ../include/*.orig

astyle:
	astyle -A1 -T -C -S -K -Y -p -U -H -z2 *.cpp ../include/*.hpp
