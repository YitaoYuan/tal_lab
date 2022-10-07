#ifndef _RING_H_
#define _RING_H_

#include <stdint.h>

#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

/**
 * Usage:
 *
 *   step 1: declare a ring
 *     ring myring(16)
 *   step 2: enqueue an object
 *     struct xxx_t xxx;
 *     myring.enqueue((void*) &xxx);
 *   step 3:
 *     struct xxx_t* xxx;
 *     myring.dequeue((void**) &xxx);
 */
class ring {
public:
    ring();
    /**
     * Create a ring buffer
     *
     * @para count
     *   The size of the ring (must be a power of 2).
     *   Each element of the ring is a pointer
     * @para size
     *  The size of each element
     */
    ring(unsigned int count, size_t size);
    /**
     * Do enqueue operation
     *
     * @para obj
     *   A pointer to the object to be added.
     *
     * @return
     *   Non-zero on success;
     *   0 on failure (only buffer is full).
     */
    int enqueue(void* obj);
    /**
     * Do dequeue operation
     *
     * @para obj
     *   A pointer to a void * pointer (object) that will be filled.
     * @return
     *   Non-zero on success;
     *   0 on failure (only buffer is empty).
     */
    int dequeue(void* obj);
    /**
     * Similar to the dequeue(), but do not delete the first element of the queue
     * 
     * @para obj
     *   A pointer to a void * pointer (object) that will be filled.
     * @return
     *   Non-zero on success;
     *   0 on failure (only buffer is empty).
     */
    int front(void* obj);
    /**
     * @return 
     *   true, queue is empty
     *   false, otherwise
     */
    bool empty();
    bool full();
    int size();
    /**
     * Free memory allocated to the ring buffer
     */
    ~ring(void);

private:
    /**
     * Atomic compare and set.
     *   (atomic) equivalent to: if (*dst == exp) *dst = src (all 32-bit words)
     *
     * @para dst
     *   The destination location into which the value will be written.
     * @para exp
     *   The expected value.
     * @para src
     *   The new value.
     *
     * @return
     *   Non-zero on success;
     *   0 on failure.
     */
    int atomic32_cmpset(volatile uint32_t *dst, uint32_t exp, uint32_t src);

    // ring buffer
    void* ring_buffer;

    size_t ele_size;
    // size of the ring
    unsigned int count;
    // count - 1
    unsigned int mask;
    // size of an object
    // size_t obj_size;

    // consumer head pointer
    volatile uint32_t cons_head;
    // consumer tail pointer
    volatile uint32_t cons_tail;
    // producer head pointer
    volatile uint32_t prod_head;
    // producer tail pointer
    volatile uint32_t prod_tail;
};

#endif
