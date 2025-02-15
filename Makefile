all: gpiod tally

gpiod: gpiod.c
	gcc -o gpiod -g -O0 -Wall -pedantic gpiod.c -lwiringPi -lpthread
	
tally: tally.c
	gcc -o tally tally.c -lwiringPi


