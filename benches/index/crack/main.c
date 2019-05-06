/**
 * Copyright 2019 Marcus Pinnecke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of
 * the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <utils/time.h>
#include <utils/env.h>
#include "core/index/crack_index.h"

#define ITEM_MAX 650000
#define KEY_RAND_MAX 2048

int main(int argc, char *argv[])
{
        ng5_unused(argc);
        ng5_unused(argv);

        struct crack_index index;
        struct err default_err;
        crack_index_create(&index, &default_err, ITEM_MAX);

        u32 *values = malloc(ITEM_MAX * sizeof(u32));
        for (u32 i = 0; i < ITEM_MAX; i++) {
                values[i] = i;
        }

        for (u32 i = 0; i < ITEM_MAX; i++) {
                u32 key = 1 + (rand() % KEY_RAND_MAX);
                crack_index_push(&index, key, values + i);
        }

        u16 num_reruns = 0;
        timestamp_t acc;
        printf("rerun;alpha;num_sec;num_ops_pop;num_ops_push;duration_ms;ops_per_sec;mem_usage;mem_usage_peak\n");

        while (num_reruns++ < 1) {
                for (float alpha = 0; alpha <= 1.0f; alpha += 1.0f) {
                        for (u16 num_sec = 0; num_sec < 5; num_sec++) {
                                acc = 0;

                                u64 num_ops_pop = 0;
                                u64 num_ops_push = 0;
                                while (acc < 2000) {
                                        bool coin = (rand() % 2) == 0;
                                        timestamp_t begin = time_now_wallclock();
                                        if (coin) {

                                                for (u32 i = 0; i < alpha * 1000; i++) {
                                                        u32 key = 1 + (rand() % KEY_RAND_MAX);
                                                        u32 *data = (u32 *) crack_index_pop(&index, key);
                                                        ng5_unused(data);
                                                        num_ops_pop++;
                                                }

                                        } else {

                                                for (u32 i = 0; i < (1 - alpha) * 1000; i++) {
                                                        u32 key = 1 + (rand() % KEY_RAND_MAX);
                                                        u32 value = *(values + ((1 + rand()) % (ITEM_MAX - 10)));
                                                        crack_index_push(&index, key, &value);
                                                        num_ops_push++;
                                                }
                                        }
                                        timestamp_t end = time_now_wallclock();
                                        acc += (end - begin);
                                }

                                printf("%d;%0.2f;%d;%" PRIu64 ";%" PRIu64 ";%" PRIu64 ";%f;%zu;%zu\n",
                                        num_reruns,
                                        alpha,
                                        num_sec,
                                        num_ops_pop,
                                        num_ops_push,
                                        acc,
                                        ((num_ops_pop + num_ops_push) / (float) (acc / 1000.0f)),
                                        env_get_process_memory_usage(),
                                        env_get_process_peak_memory_usage());
                                fflush(stdout);
                        }
                }

        }


        crack_index_drop(&index);
        free(values);

        return EXIT_SUCCESS;
}