#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>

#define PIN	27

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid args\n");
    exit(1);
  }

  int val;
  if (argv[1][0] == '0')
    val = 0;
  else if (argv[1][0] == '1')
    val = 1;
  else {
    fprintf(stderr, "invalid args\n");
    exit(1);
  }

  wiringPiSetupGpio();
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN, val);
}

