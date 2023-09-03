# Makefile for pong program
pong: pong.c
	gcc -Wall -w -g -o pong pong.c -lncurses