all: gpiod.c
	gcc -o gpiod gpiod.c -lwiringPi
