all:
	gcc -g -Wall -O3 -o parser main.c parser.c esp8266.c

clean:
	rm -rf parser
