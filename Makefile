all: hello

hello: main.o user.o
	clang++ main.o user.o -o hello -std=c++17

main.o: main.cpp
	clang++ -c main.cpp -std=c++17

user.o: user.cpp
	clang++ -c user.cpp -std=c++17

clean:
	rm -rf *.o hello