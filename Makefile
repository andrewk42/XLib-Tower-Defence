# Makefile for SE382 Assignment 1
# By Andrew Klamut; UWID #20281315

CXXFLAGS = -lX11 -Wall

run: ajklamut_a1
	./ajklamut_a1

ajklamut_a1: main.c
	gcc $(CXXFLAGS) main.c -o ajklamut_a1

clean:
	rm ajklamut_a1

# vim:set ts=4 sw=4 sts=4 noexpandtab:
