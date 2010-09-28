all:hellosphinx test
hellosphinx:hello.cpp
	g++ -g -o hellosphinx hello.cpp -lsphinxclient
test:test.cpp
	g++ -g -o test test.cpp 
clean:
	rm test hellosphinx *.o
