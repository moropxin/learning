/* day1/main.c
   Demonstrates:
   - array_reverse_inplace
   - remove_element (in-place)
   - two_sum_bruteforce
   - two_sum_hash (simple open-addressing hashmap)
   Compile: gcc -std=c11 -O2 -Wall day1/main.c -o day1/main
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* ---------- Array reverse (in-place) ---------- */
/*
   array_reverse(a, start, end)
   - start: 起始索引（包含）
   - end: 结束索引（包含）
*/
void array_reverse(int *a, size_t start, size_t end) {
    if (!a) return;
    if (start >= end) return;
    size_t i = start, j = end; // end 被视为包含的最后一个索引
    while (i < j) {
        int tmp = a[i];
        a[i] = a[j];
        a[j] = tmp;
        i++; j--;
    }
}

/* ---------- Remove element in-place ----------
   Removes all occurrences of val. Returns new length.
   Classic fast/slow pointer approach.
*/
size_t remove_element(int *a, size_t n, int val) {
    if (!a) return 0;
    size_t write = 0;
    for (size_t read = 0; read < n; ++read) {
        if (a[read] != val) {
            a[write++] = a[read];
        }
    }
    return write;
}

/* ---------- Simple hashmap for int -> int (index) ----------
   Open addressing with linear probing.
   - capacity must be > 2*expected_count for reasonable behavior.
   - Does not implement resizing or deletion (OK for this two-sum usage).
*/
typedef struct {
    int key;
    int value;
    bool used;
} Entry;

typedef struct {
    Entry *entries;
    size_t capacity; // 表示数组范围
} IntMap;

// size_t 为无符号整数类型，用于存储内存大小/数组长度
static size_t imap_next_power_of_two(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1; // 等价于 p *= 2
    return p;
}

IntMap *imap_create(size_t capacity) {
    capacity = imap_next_power_of_two(capacity * 2); // reduce load
    IntMap *m = malloc(sizeof(*m));
    if (!m) return NULL;
    m->capacity = capacity ? capacity : 16;
    m->entries = calloc(m->capacity, sizeof(Entry));
    if (!m->entries) { free(m); return NULL; }
    return m;
}

void imap_free(IntMap *m) {
    if (!m) return;
    free(m->entries);
    free(m);
}

static size_t imap_hash(int key, size_t cap) {
    // simple mix for int keys
    uint32_t k = (uint32_t)key;
    k = (k ^ (k >> 16)) * 0x45d9f3b;
    k = (k ^ (k >> 16)) * 0x45d9f3b;
    k = k ^ (k >> 16);
    return (size_t)k & (cap - 1); // cap is power of two
}

bool imap_put(IntMap *m, int key, int value) {
    if (!m) return false;
    size_t idx = imap_hash(key, m->capacity);
    for (size_t i = 0; i < m->capacity; ++i) {
        size_t pos = (idx + i) & (m->capacity - 1);
        if (!m->entries[pos].used) {
            m->entries[pos].used = true;
            m->entries[pos].key = key;
            m->entries[pos].value = value;
            return true;
        }
        if (m->entries[pos].used && m->entries[pos].key == key) {
            // overwrite index if key exists
            m->entries[pos].value = value;
            return true;
        }
    }
    return false; // map full
}

bool imap_get(IntMap *m, int key, int *out_value) {
    if (!m) return false;
    size_t idx = imap_hash(key, m->capacity);
    for (size_t i = 0; i < m->capacity; ++i) {
        size_t pos = (idx + i) & (m->capacity - 1);
        if (!m->entries[pos].used) {
            return false; // not found
        }
        if (m->entries[pos].used && m->entries[pos].key == key) {
            if (out_value) *out_value = m->entries[pos].value;
            return true;
        }
    }
    return false;
}

/* ---------- Two-sum brute force ----------
   Returns 1 if found and set i_out/j_out to indices, else 0.
*/
int two_sum_bruteforce(int *a, size_t n, int target, int *i_out, int *j_out) {
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            if (a[i] + a[j] == target) {
                if (i_out) *i_out = (int)i; // 把 size_t 转为 int 便于输出
                if (j_out) *j_out = (int)j; // 判断指针是否为空，防止写入非法地址
                return 1;
            }
        }
    }
    return 0;
}

/* ---------- Two-sum using hashmap (one-pass) ----------
   Returns 1 if found, with indices in i_out, j_out (i is earlier index).
*/
int two_sum_hash(int *a, size_t n, int target, int *i_out, int *j_out) {
    IntMap *m = imap_create(n + 4);
    if (!m) return 0;
    for (size_t i = 0; i < n; ++i) {
        int need = target - a[i];
        int found_index;
        if (imap_get(m, need, &found_index)) {
            if (i_out) *i_out = found_index;
            if (j_out) *j_out = (int)i;
            imap_free(m);
            return 1;
        }
        // store current value -> index
        imap_put(m, a[i], (int)i);
    }
    imap_free(m);
    return 0;
}

/* ---------- Helpers for printing arrays ---------- */
void print_array(int *a, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; ++i) {
        printf("%d", a[i]);
        if (i + 1 < n) printf(", ");
    }
    printf("]");
}

/* ---------- Main: simple tests ---------- */
int main(void) {
    puts("Test array_reverse:");
    int arr1[] = {1,2,3,4,5};
    size_t n1 = sizeof(arr1)/sizeof(arr1[0]);
    size_t st = 0, en = n1 - 1; // 反转整个数组
    printf("orig: "); print_array(arr1, n1); printf("\n");
    array_reverse(arr1, st, en);
    printf("reversed: "); print_array(arr1, n1); printf("\n\n");

    puts("Test remove_element (remove all 2s):");
    int arr2[] = {0,2,2,3,2,4,2,5};
    size_t n2 = sizeof(arr2)/sizeof(arr2[0]);
    printf("orig: "); print_array(arr2, n2); printf("\n");
    size_t newlen = remove_element(arr2, n2, 2);
    printf("after remove: "); print_array(arr2, newlen); printf(" newlen=%zu\n\n", newlen);

    puts("Test two_sum_bruteforce:");
    int a3[] = {2,7,11,15};
    size_t n3 = sizeof(a3)/sizeof(a3[0]);
    int i,j;
    if (two_sum_bruteforce(a3, n3, 9, &i, &j)) {
        printf("found indices %d, %d -> values %d + %d = 9\n\n", i, j, a3[i], a3[j]);
    } else {
        printf("not found\n\n");
    }

    puts("Test two_sum_hash:");
    int a4[] = {3,3};
    size_t n4 = sizeof(a4)/sizeof(a4[0]);
    if (two_sum_hash(a4, n4, 6, &i, &j)) {
        printf("found indices %d, %d -> values %d + %d = 6\n", i, j, a4[i], a4[j]);
    } else {
        printf("not found\n");
    }

    return 0;
}
