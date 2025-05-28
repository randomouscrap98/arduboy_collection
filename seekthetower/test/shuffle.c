#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PERIOD 65536
#define SEQCHECK 256
#define PRNG(a, c, x) (a * x + c)

int main() {
  // Each shuffle
  uint8_t *seen = (uint8_t *)malloc(PERIOD);
  // Check first 16 sequence
  uint16_t *sequences =
      (uint16_t *)malloc(sizeof(uint16_t) * PERIOD * SEQCHECK);
  // printf("Checking seed %d\n", i);
  printf("Checking seeds for period...\n");
  for (uint32_t i = 0; i < PERIOD; i++) {
    // printf("-----------------------------------------------\n");
    memset(seen, 0, PERIOD);
    // Compute a and c. This method uses ALL the bits of the seed
    uint16_t c = 2 * (i & 0xFFF) + 1;
    // uint16_t a = (((i >> 8) & 0xFF) << 3) | 0x5;
    // uint16_t a = (((i >> 12) & 0xF) << 3) | 0x5; //(i << 3) | 0x5;
    uint16_t a = (i & 0xF000) | 0x0805; //(i << 3) | 0x5;
    // Make sure full period at least
    for (uint32_t x = 0; x < PERIOD; x++) {
      uint16_t k = PRNG(a, c, x);
      if (seen[k]) {
        printf("Sequence for seed %d (a=%d, c=%d) failed at %d = %d\n", i, a, c,
               x, k);
        return 1;
      }
      seen[k] = 1;
      if (x < SEQCHECK) {
        // printf("[%d] %d\n", x, k);
        sequences[SEQCHECK * i + x] = k;
      }
    }
  }
  // return 0;
  // Now check for any matching sequences... I don't care how "bad" they are
  // though
  uint16_t failure_rate = 0;
  printf("Checking for first %d sequence match\n", SEQCHECK);
  for (uint32_t i = 0; i < PERIOD; i++) {
    for (uint32_t j = i + 1; j < PERIOD; j++) {
      for (uint32_t p = 0; p < SEQCHECK; p++) {
        if (sequences[i * SEQCHECK + p] != sequences[j * SEQCHECK + p])
          goto NEXTSEQUENCE;
      }
      // Oops, the entire sequence matched
      printf("SEQMATCH[%d]: %d = %d\n", SEQCHECK, i, j);
      failure_rate++;
      // return 1;
    NEXTSEQUENCE:;
    }
  }
  printf("Fails: %d/%d\n", failure_rate, PERIOD);
}
