#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include <cstring>

#include <stdlib.h>
#include <stdint.h>

#include "ring.h"

ring::ring() {
}

ring::ring(unsigned int cnt, size_t size) {
    count = cnt;
    if ((count & (count - 1)) != 0) {
        unsigned int exp = (unsigned int) ceil(log2(count));
	    count = 1 << exp;
    }
    if (count < 16)
        count = 16;
    mask = count - 1;

    // obj_size = osize;

    cons_head = 0;
    cons_tail = 0;
    prod_head = 0;
    prod_tail = 0;

    ele_size = size;
    ring_buffer = (void*) malloc(count * size);
    if (ring_buffer == NULL) {
        printf("Failed to create a ring buffer!\n");
    }
}

int ring::enqueue(void* obj) {
    int ret = 0;

    uint32_t temp_ct, temp_pd, next_pd;
    do {
        temp_ct = cons_tail;
        temp_pd = prod_head;
        next_pd = (temp_pd + 1) & mask;

        if (unlikely(next_pd == temp_ct))
           return 0;

        ret = atomic32_cmpset(&prod_head, temp_pd, next_pd);
    } while (unlikely(ret == 0));

    memcpy(((char*) ring_buffer) + temp_pd * ele_size, obj, ele_size);
    // ring_buffer[temp_pd] = obj;

    do {
        ret = atomic32_cmpset(&prod_tail, temp_pd, next_pd);
    } while (unlikely(ret == 0));

    return ret;
}

int ring::dequeue(void* obj) {
    int ret = 0;

    uint32_t temp_pt, temp_ch, next_ch;
    do {
        temp_pt = prod_tail;
        temp_ch = cons_head;
        next_ch = (temp_ch + 1) & mask;

        if (unlikely(temp_ch == temp_pt))
            return 0;

        ret = atomic32_cmpset(&cons_head, temp_ch, next_ch);
    } while (unlikely(ret == 0));

    if (obj != NULL) {
        memcpy(obj, ((char*) ring_buffer) + temp_ch * ele_size, ele_size);
        // *obj = ring_buffer[temp_ch];
    }

    do {
        ret = atomic32_cmpset(&cons_tail, temp_ch, next_ch);
    } while (unlikely(ret == 0));

    return ret;
}

int ring::front(void* obj) {
    uint32_t temp_pt = prod_tail;
    uint32_t temp_ch = cons_head;

    if (unlikely(temp_ch == temp_pt))
        return 0;

    memcpy(obj, ((char*) ring_buffer) + cons_head * ele_size, ele_size);
    // *obj = ring_buffer[cons_head];

    return 1;
}

bool ring::empty() {
    uint32_t temp_pt = prod_tail;
    uint32_t temp_ch = cons_head;

    if (temp_ch == temp_pt) {
        return true;
    } else {
        return false;
    }
}

int ring::size() {
    return (prod_tail - cons_head + count) % count;
}

bool ring::full() {
    uint32_t temp_pt = prod_tail;
    uint32_t temp_ch = cons_head;
    if ((temp_pt + 1) % count == temp_ch)
        return true;
    else
        return false;
}

ring::~ring(void) {
    if (ring_buffer != NULL) {
        free(ring_buffer);
    }
}

int ring::atomic32_cmpset(
    volatile uint32_t *dst,
    uint32_t exp,
    uint32_t src
) {
    uint8_t res;

    asm volatile(
        "lock ; "
        "cmpxchgl %[src], %[dst];"
        "sete %[res];"
        : [res] "=a" (res),     /* output */
        [dst] "=m" (*dst)
        : [src] "r" (src),      /* input */
        "a" (exp),
        "m" (*dst)
        : "memory");            /* no-clobber list */
    return res;
}
