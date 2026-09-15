#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>

// time_us = 1789327119526146, fusion_time = 1789327119523086, delta_us = 3060, delta_sec = 0?
int main(void){

    uint64_t time_us = 1789327119526146;
    uint64_t fusion_time_us = 1789327119523086;
    uint64_t delta_us = time_us - fusion_time_us;
    uint64_t delta_sec = delta_us * 0.000001f;
    // printf("delta_us = %" PRIu64  "\n", delta_us);
    printf("delta_us = %llu, delta_sec = %f", delta_us, delta_sec);

    return 0;
}