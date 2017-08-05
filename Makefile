all:
	gcc -g -Wall -O3 -o parser main.c parser.c

clean:
	rm -rf parser
