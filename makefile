SL2LIBDIR=/usr/include/SDL2
main:
	g++ -O3 main.cpp -o main -I${SL2LIBDIR} -lSDL2main -lSDL2