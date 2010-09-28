all:hellosphinx test
hellosphinx:hello.o
	g++ -o hellosphinx hello.o -lsphinxclient
hello.o:hello.cpp
	g++ -g -c -o hello.o hello.cpp
test:test.o
	g++ -o test test.o
test.o:test.cpp
	g++ -g -c -o test.o test.cpp

clean:
	rm test hellosphinx *.o
