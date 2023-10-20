// https://en.wikipedia.org/wiki/Bitonic_sorter#Alternative_representation
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <vector>

void compare_and_swap(int* arr, int i, int j, int n) {
    if (i < n && j < n) {
        if (arr[i] > arr[j]) {
            int tmp = arr[i];
            arr[i] = arr[j];
            arr[j] = tmp;
        }
    }
    /* parallel on GPU:
     * memoryBarrier();
     * barrier();
     */
}

void bitonic_merge(int* arr, int start, int end, int n) {
    // start = j = block_start
    // end = j + i = block_end
    // end - start = i = block_size
    if (end - start <= 1) {
        return;
    }
    int size = end - start;
    for (int a = size / 2; a > 1; a /= 2) {
        /* parallel on GPU:
         * i / 2 GPU cores are here
         * // a = sub_block_size
         */
        // printf("  a = %d\n", a);
        for (int b = 0; b < size; b += a) {
            /* parallel on GPU:
             * a / 2 GPU cores should be here
             * // b = sub_block_start_offset =
             * // = sub_block_index_offset * sub_block_size
             * b = (offset_in_block / (a / 2)) * a
             */
            // printf("    b = %d\n", b);
            for (int c = 0; c < a / 2; ++c) {
                /* parallel on GPU:
                 * 1 GPU core should be here
                 * // c = offset_in_sub_block
                 * c = offset_in_block % (a / 2)
                 */
                int d = b + c + start;
                // printf("      c = %d\n", d);
                int e = b + c + start + a / 2;
                // printf("      d = %d\n", e);
                compare_and_swap(arr, d, e, n);
            }
        }
    }
}

void bitonic_sort(int* arr, int start, int end, int n) {
    if (end - start <= 1) {
        return;
    }
    for (int i = 2; i <= (end - start); i *= 2) {
        /* parallel on GPU:
         * // i = block_size
         */
        // printf("i = %d\n", i);
        for (int j = 0; j < (end - start); j += i) {
            /* parallel on GPU:
             * i / 2 GPU cores should be here
             * // j = block_start = block_index * block_size
             * j = ((gl_GlobalInvocationID.x / (i / 2)) * i
             */
            // printf("  j = %d\n", j);
            // első merge
            for (int k = 0; k < i / 2; ++k) {
                /* parallel on GPU:
                 * 1 GPU core should be here
                 * // k = offset_in_block
                 * k = (gl_GlobalInvacationID.x) % (i/2)
                 */
                int l = j + k;
                int m = j + i - 1 - k;
                // printf("    l = %d\n", l);
                // printf("    m = %d\n", m);
                compare_and_swap(arr, l, m, n);
            }
            // további merge
            bitonic_merge(arr, j, j + i, n);
        }
    }
    bitonic_merge(arr, start, end, n);
}

int main(void) {
    std::srand(std::time(nullptr));
    int n = std::rand() % 993319;
    // int n = (2 << 20) + 1;
    // int n = 8;
    std::vector<int> arr;
    for (int i = 0; i < n; ++i) {
        // arr.push_back(i%4);
        arr.push_back((std::rand() << 1) + 993319);
        // arr.push_back(std::rand() % 8);
    }
    // for (int i = 0; i < n; ++i) {
    //     printf("%d, ", arr[i]);
    // }
    // printf("\n");
    printf("n = %d\n", n);
    int k = 1;
    while (k < n) {
        k <<= 1;
    }
    printf("k = %d\n", k);
    bitonic_sort(&arr[0], 0, k, n);
    // for (int i = 0; i < n; ++i) {
    //     printf("%d, ", arr[i]);
    // }
    // printf("\n");
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i] > arr[i + 1]) {
            printf("not sorted!\n");
            return 1;
        }
    }
    printf("sorted\n");
    return 0;
}
