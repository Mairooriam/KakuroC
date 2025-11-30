#include <stdint.h>
#include <stdio.h>
void print_binary(unsigned int number) {
  if (number >> 1) {
    print_binary(number >> 1);
  }
  putc((number & 1) ? '1' : '0', stdout);
}
int main(int argc, char const *argv[]) {
  /* code */
  uint8_t numbers[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint8_t mask = 1;
  uint8_t sum = 18;
  uint8_t with = 4;

  for (int mask = 0; mask < (1 << 9); mask++) {

    if (__builtin_popcount(mask) == 4) {
      printf("mask has 4 numbers");
      print_binary(mask);
      printf("\n");
    }
  }
  return 0;
}
