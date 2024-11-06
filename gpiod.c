#include <stdio.h>
#include <wiringPi.h>

int main(void) {
  wiringPiSetupGpio();

  // pin mode ..(INPUT, OUTPUT, PWM_OUTPUT, GPIO_CLOCK)
  // set pin 17 to input
  pinMode(17, INPUT);

  // pull up/down mode (PUD_OFF, PUD_UP, PUD_DOWN) => down
  pullUpDnControl(17, PUD_DOWN);

  // get state of pin 17
  int value = digitalRead(17);

  printf("%d\n", value);
}

