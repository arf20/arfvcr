all: gpiod tally

gpiod: gpiod.c
	gcc -o gpiod gpiod.c -lwiringPi
	
tally: tally.c
	gcc -o tally tally.c -lwiringPi


