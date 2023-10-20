// https://en.wikipedia.org/wiki/Bitonic_sorter#Alternative_representation
#include <cstdlib>
#include <ctime>
#include <stdio.h>
#include <vector>

void bitonic_merge(int* arr, int start, int end, int n) {
    if (end - start <= 1) {
        return;
    }
    int size = end - start;
    // for (int i = start; i < (start + end) / 2; ++i) {
    //     if (i < n && i + size / 2 < n) {
    //         if (arr[i] > arr[i + (end - start) / 2]) {
    //             int tmp = arr[i];
    //             arr[i] = arr[i + (end - start) / 2];
    //             arr[i + (end - start) / 2] = tmp;
    //         }
    //     }
    // }
    // bitonic_merge(arr, start, (start + end) / 2, n);
    // bitonic_merge(arr, (start + end) / 2, end, n);
    // printf("size = %d\n", size);
    for (int a = size / 2; a > 1; a /= 2) {
        // printf("  a = %d\n", a);
        for (int b = 0; b < size / a; ++b) {
            // printf("    b = %d\n", b);
            for (int c = b * a; c < b * a + a / 2; ++c) {
                int d = c + start;
                // printf("      c = %d\n", d);
                int e = d + a / 2;
                // printf("      d = %d\n", e);
                if (d < n && e < n) {
                    if (arr[d] > arr[e]) {
                        int tmp = arr[d];
                        arr[d] = arr[e];
                        arr[e] = tmp;
                    }
                }
            }
        }
    }
}

void bitonic_sort(int* arr, int start, int end, int n) {
    if (end - start <= 1) {
        return;
    }
    // bitonic_sort(arr, start, (start + end) / 2, n);
    // bitonic_sort(arr, (start + end) / 2, end, n);
    // for (int i = 0; i < (end - start) / 2; ++i) {
    //     if (start + i < n && end - 1 - i < n) {
    //         if (arr[start + i] > arr[end - 1 - i]) {
    //             int tmp = arr[start + i];
    //             arr[start + i] = arr[end - 1 - i];
    //             arr[end - 1 - i] = tmp;
    //         }
    //     }
    // }
    for (int i = 2; i <= (end - start); i *= 2) {
        // printf("i = %d\n", i);
        for (int j = 0; j < (end - start); j += i) {
            // printf("  j = %d\n", j);
            // első merge
            for (int k = 0; k < i/2; ++k) {
                int l = j + k;
                int m = j + i - 1 - k;
                // printf("    l = %d\n", l);
                // printf("    m = %d\n", m);
                if (l < n && m < n) {
                    if (arr[l] > arr[m]) {
                        int tmp = arr[l];
                        arr[l] = arr[m];
                        arr[m] = tmp;
                    }
                }
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
