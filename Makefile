all : sobel

sobel:	sobel.c
	gcc -o sobel.exe sobel.c -Wall
