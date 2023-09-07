# Makefile for pong program
pong: pong.c
	gcc -Wall -w -g pong.c draw.c inputs.c screens.c -o pong -lncurses