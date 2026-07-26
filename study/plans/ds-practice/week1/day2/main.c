/* day2/main.c
   Implements:
   - insertion_sort
   - merge_sort (recursive)
   - quick_sort (Lomuto partition)
   Compile: gcc -std=c11 -O2 -Wall day2/main.c -o day2/main
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- Helpers ---------- */
/*用两个函数防止运用不同排序方法时把原数组释放，
  防止后续程序无法正常进行*/
void print_array(int *a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("%d", a[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]\n");
}

int *copy_array(int *a, size_t n) {
    int *b = malloc(n * sizeof(int));
    if (!b) return NULL;
    memcpy(b, a, n * sizeof(int)); // memcpy函数为复制地址函数，形式为memcpy(目的地址，目标地址，复制字节大小)
    return b;
}

/* ---------- Insertion Sort ---------- */
void insertion_sort(int *a, size_t n) {
    for (size_t i = 1; i < n; ++i) { // 大循环遍历key
        int key = a[i];
        ssize_t j = (ssize_t)i - 1; // ssize_t是有符号类型定义，可以存放负值
        while (j >= 0 && a[j] > key) { //小循环遍历key的左值，并递减至数组第一位
            a[j+1] = a[j];
            --j;
        }
        a[j+1] = key; //最后把key值还给数组a[]，防止数据丢失
    }
}

/* ---------- Merge Sort with line-by-line comments ---------- */

/*
 merge 函数：把两个相邻的有序区间 [l..m] 和 [m+1..r] 合并成一个有序区间
 参数：
   a   - 原数组
   l   - 左区间起始索引
   m   - 左区间结束索引（右区间从 m+1 开始）
   r   - 右区间结束索引
   tmp - 临时数组（大小至少为数组总长度），用于暂存合并结果
*/
void merge(int *a, size_t l, size_t m, size_t r, int *tmp) {
    size_t i = l;       // i 指向左区间当前要读取的位置，初始为 l
    size_t j = m + 1;   // j 指向右区间当前要读取的位置，初始为 m+1
    size_t k = l;       // k 指向临时数组写入位置，初始也从 l 开始

    // 当左右两边都有剩余元素时，比较大小把较小的放入 tmp
    while (i <= m && j <= r) {
        if (a[i] <= a[j])    // 如果左边的元素 <= 右边的（使用 <= 可保证稳定性）
            tmp[k++] = a[i++]; // 把左边元素放入 tmp，然后 i++（读取左区下一个）
        else
            tmp[k++] = a[j++]; // 否则把右边元素放入 tmp，然后 j++
    }

    // 如果左区还有剩余（右区先消耗完），把左区剩余全部复制到 tmp
    while (i <= m) {
        tmp[k++] = a[i++];
    }

    // 如果右区还有剩余（左区先消耗完），把右区剩余全部复制到 tmp
    while (j <= r) {
        tmp[k++] = a[j++];
    }

    // 把 tmp 中合并好的有序区间拷回到原数组 a 的 [l..r] 区间
    for (size_t x = l; x <= r; ++x) {
        a[x] = tmp[x]; // 注意 tmp 中数据是从索引 l 开始存放的，因此直接按索引拷回
    }
}

/*
 merge_sort_rec 函数：对区间 [l..r] 递归进行归并排序
 参数：
   a   - 原数组
   l   - 当前区间左端索引
   r   - 当前区间右端索引
   tmp - 临时数组，用于合并时暂存
*/
void merge_sort_rec(int *a, size_t l, size_t r, int *tmp) {
    if (l >= r) return;           // 基线条件：区间长度为 0 或 1 时已排序，直接返回
    size_t m = l + (r - l) / 2;   // 计算中点 m，避免直接 (l+r)/2 可能的溢出（虽然 size_t 溢出不常见）
    merge_sort_rec(a, l, m, tmp); // 递归排序左半部分 [l..m]
    merge_sort_rec(a, m+1, r, tmp); // 递归排序右半部分 [m+1..r]
    merge(a, l, m, r, tmp);       // 合并左右两个已排序的子区间
}

/*
 merge_sort 函数：对整个数组进行归并排序的入口
 参数：
   a - 要排序的数组
   n - 数组长度
 说明：
   分配一个大小为 n 的临时数组 tmp，然后调用递归函数；完成后释放 tmp。
*/
void merge_sort(int *a, size_t n) {
    if (n == 0) return;           // 空数组直接返回，避免分配 0 大小内存（虽然 malloc(0) 可返回 NULL 或非 NULL）
    int *tmp = malloc(n * sizeof(int)); // 分配临时数组，用于合并时暂存数据
    if (!tmp) return;             // 如果分配失败，直接返回（真实项目应处理错误）
    merge_sort_rec(a, 0, n-1, tmp); // 对整个区间 [0..n-1] 做递归归并排序
    free(tmp);                    // 释放临时数组内存
}

/* ---------- Quick Sort (Lomuto partition) with line-by-line comments ---------- */

/*
 lomuto_partition:
 - 把区间 [lo..hi] 以 a[hi] 作为枢轴(pivot)分区
 - 返回枢轴最后放置的位置索引（pivot index）
 - 约定：lo 和 hi 都是包含边界（inclusive），使用 size_t 类型
*/

void swap(int arr[],size_t x,size_t y){
    if(x == 0 && y == 0) return;
    int temp;
    temp = arr[x];
    arr[x] = arr[y];
    arr[y] = temp;
}

void randomswaptohi(int arr[],size_t lo,size_t hi){
    if(lo == 0) return;
    int random = lo + rand()%(hi - lo + 1);
    swap(arr,random,hi);
}

size_t lomuto_partition(int *a, size_t lo, size_t hi) {
    randomswaptohi(*a,lo,hi);
    int pivot = a[hi];            // 选取区间最后一个元素作为枢轴值
    size_t i = lo;                // i 指向下一个“比 pivot 小”的元素应当放置的位置
    // j 用于遍历从 lo 到 hi-1 的元素，检查哪些元素小于枢轴
    for (size_t j = lo; j < hi; ++j) {
        if (a[j] < pivot) {      // 如果当前元素小于枢轴
            // 交换 a[i] 和 a[j]，把较小元素放到左边的“已收集”区
            int t = a[i];
            a[i] = a[j];
            a[j] = t;
            ++i;                 // 已收集元素区扩大一位
        }
    }
    // 将枢轴移动到中间位置 i（这样左边都 < pivot，右边 >= pivot）
    int t = a[i];
    a[i] = a[hi];
    a[hi] = t;
    return i;                    // 返回枢轴的最终位置
}

/*
 quick_sort_rec:
 - 对区间 [lo..hi] 递归地进行快速排序
 - 注意 size_t 是无符号的，做 p-1 的递归调用时要避免下溢（因此判断 p>0）
*/
void quick_sort_rec(int *a, size_t lo, size_t hi) {
    if (lo >= hi) return;        // 区间长度为 0 或 1，已排序，结束递归
    size_t p = lomuto_partition(a, lo, hi); // 分区，p 为枢轴新位置

    // 递归排序左半部分 [lo .. p-1]
    if (p > 0) {                 // 必须检查 p>0，防止 size_t 下溢（p-1 会变成很大）
        quick_sort_rec(a, lo, p - 1);
    }
    // 递归排序右半部分 [p+1 .. hi]
    // 当 p == SIZE_MAX（不可能在正常用法中），p+1 会溢出；一般情形下这行安全
    quick_sort_rec(a, p + 1, hi);
}

/*
 quick_sort:
 - 对整个数组进行快速排序的入口函数
 - 当 n 为 0 时直接返回，避免对空区间调用递归
*/
void quick_sort(int *a, size_t n) {
    if (n == 0) return;
    quick_sort_rec(a, 0, n - 1);
}

/* ---------- 可选：随机枢轴版本（减少最坏情况概率） ---------- */
/*
 快速排序的最坏情况发生在枢轴总是选择不好的情况下（例如总选最后一个而数组已接近有序）。
 一个简单办法是随机选枢轴：把随机选出的元素与 hi 交换，然后按 Lomuto 分区处理。
*/
#include <stdlib.h> // 为 rand()/srand()

size_t lomuto_partition_random_pivot(int *a, size_t lo, size_t hi) {
    // 生成一个在 [lo, hi] 的随机索引 r，并把 a[r] 交换到 hi 位置作为枢轴
    size_t r = lo + (size_t)(rand() % (hi - lo + 1)); // 注意：hi-lo+1 非0前提 lo<=hi
    int tmp = a[r];
    a[r] = a[hi];
    a[hi] = tmp;
    // 然后调用普通 lomuto_partition（现在 a[hi] 是随机枢轴）
    return lomuto_partition(a, lo, hi);
}

/* 使用随机化分区的递归版本（示例） */
void quick_sort_rec_random(int *a, size_t lo, size_t hi) {
    if (lo >= hi) return;
    size_t p = lomuto_partition_random_pivot(a, lo, hi);
    if (p > 0) quick_sort_rec_random(a, lo, p - 1);
    quick_sort_rec_random(a, p + 1, hi);
}

/* 使用随机枢轴的入口（需要事先调用 srand(time(NULL))） */
void quick_sort_random(int *a, size_t n) {
    if (n == 0) return;
    quick_sort_rec_random(a, 0, n - 1);
}
/* ---------- Main: tests ---------- */
int main(void) {
    int data[] = {1,3,4,2,6};
    size_t n = sizeof(data)/sizeof(data[0]);

    printf("orig: ");
    print_array(data, n);

    int *b;

    b = copy_array(data, n);
    insertion_sort(b, n);
    printf("insertion_sort: ");
    print_array(b, n);
    free(b);

    b = copy_array(data, n);
    merge_sort(b, n);
    printf("merge_sort:     ");
    print_array(b, n);
    free(b);

    b = copy_array(data, n);
    quick_sort(b, n);
    printf("quick_sort:     ");
    print_array(b, n);
    free(b);

    /* edge cases */
    /*释放子函数和工具函数内存，优化内存管理*/
    int empty[] = {};
    quick_sort(empty, 0); /* should be fine */


    int one[] = {42};
    merge_sort(one, 1);
    print_array(one, 1);

    return 0;
}
