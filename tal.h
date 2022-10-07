/**
 * Tensor Aggregation Layer 
 */

#ifndef _TAL_H_
#define _TAL_H_


#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


/**
 * Arguments passed to tal
 */
struct tal_args_t { 
	int rank;
	int size;
    // More
	struct sockaddr_in ps_server_addr;
};

/**
 * Aggregation task
 */
struct tal_task_t {
	// Base address of the tensor in CPU
	void*    base;
	// Total bytes of the tensor
	uint64_t bytes;
	// A globally unique key used to identify a tensor
	int64_t  key;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize tal
 *
 * @para arg: arguments to initialize tal
 */
void tal_init(struct tal_args_t arg); 

/**
 * Free resources and exit tal
 */
void tal_exit(void); 

/**
 * Pass a tensor into the aggregation layer
 *
 * @para task: An aggregation task 
 */
void tal_push(struct tal_task_t task);

/**
 * Poll a completed tensor from the aggregation layer 
 *
 * @return
 *   key, key of the tensor
 *   -1, if no tensor aggregation is completed
 */
int64_t tal_poll(void);

#ifdef __cplusplus
}
#endif

#endif
