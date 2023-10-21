// https://en.wikipedia.org/wiki/Bitonic_sorter#Alternative_representation
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <random>
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
}

void bitonic_merge1(int* arr, int block_size, int n) {
// an iteration of the for loop emulates a single compute shader
#pragma omp parallel for
    for (int gpuID = 0; gpuID < n / 2; ++gpuID) {
        int block_start = (gpuID / (block_size / 2)) * block_size;
        int offset_in_block = gpuID % (block_size / 2);
        compare_and_swap(arr,
                         block_start + block_size / 2 - 1 - offset_in_block,
                         block_start + block_size / 2 + offset_in_block, n);
    }
}

void bitonic_merge2(int* arr, int block_size, int n) {
// an iteration of the for loop emulates a single compute shader
#pragma omp parallel for
    for (int gpuID = 0; gpuID < n / 2; ++gpuID) {
        int block_start = (gpuID / (block_size / 2)) * block_size;
        int offset_in_block = gpuID % (block_size / 2);
        compare_and_swap(arr, block_start + offset_in_block,
                         block_start + offset_in_block + block_size / 2, n);
    }
}

void bitonic_sort(int* arr, int n) {
    // the for loops will be done on cpu
    // the cpu will dispatch the right shaders with the right parameters
    for (int block_size = 2; block_size < n * 2; block_size *= 2) {
        bitonic_merge1(arr, block_size, n);
        for (int sub_size = block_size / 2; sub_size > 1; sub_size /= 2) {
            bitonic_merge2(arr, sub_size, n);
        }
    }
}

int main(void) {
    std::srand(std::time(nullptr));
    // int n = std::rand() % 993319;
    int n = (2 << 20) + 1;
    std::vector<int> arr;
    for (int i = 0; i < n; ++i) {
        arr.push_back(i);
    }
    std::shuffle(arr.begin(), arr.end(),
                 std::default_random_engine(std::rand()));
    bitonic_sort(&arr[0], n);
    bool sorted = true;
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i] != i) {
            printf("anomaly: idx=%d, val=%d\n", i, arr[i]);
            sorted = false;
        }
    }
    if (sorted) {
        printf("sorted\n");
        return 0;
    } else {
        printf("not sorted\n");
        return 1;
    }
}
