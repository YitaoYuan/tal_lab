#include "../tal.h"
#include <unistd.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
using namespace std;

#include "ring.h"

/*
在同一台机器上，PROCESS_KEY能唯一的指定一个进程，是在进程创建时被分配的本机唯一随机值
for client:
    send (ALLREDUCE_INIT, [IPs, PROCESS_KEYs], (my IP, my PROCESS_KEY), MY_DATA) to daemon
    poll daemon

for daemon:
    listen, create threads to solve connections
    for a created thread, when receiving message:
        if it is task create, put the task in daemon's task pool, wakeup waiting threads, create a thread to send my entry to all others
        if it is a task's transmission, try to find the task in the pool
            if not found, hang the data transmission, pthread_cond_wait this task
            else receive all data to fill the task


*/



struct tal_ps_metadata {
	int rank;
	int size;
	int *fd;
}meta;

const int num_task = 1024; 
ring task_todo(num_task, sizeof(struct tal_task_t));
ring task_done(num_task, sizeof(struct tal_task_t));

std::thread* reduce_thread = NULL;
volatile bool force_quit = false;

void tal_reduce() {
	struct tal_task_t task;
	while(!force_quit) {
		if (task_todo.dequeue(&task) != 0) {
			//int64_t key = task.key;
			//如何解决乱序问题？？我看mpi sample的版本没有解决这个问题
			if(meta.rank == 0) {
				char *buf = new char[task.bytes];
				for(int i = 1; i < meta.size; i++) {
					int bytes = read(meta.fd[i], buf, task.bytes);//
					//printf("read %d bytes form %d\n", bytes, i);
					for(int j = 0; j < task.bytes/sizeof(int); j++) {
						//printf("%d ", ((int*)buf)[j]);
						((int*)task.base)[j] += ((int*)buf)[j];
					}
					//printf("\n");
				}
				delete buf;

				for(int i = 1; i < meta.size; i++) {
					write(meta.fd[i], task.base, task.bytes);
				}
			}
			else {
				write(meta.fd[0], task.base, task.bytes);
				read(meta.fd[0], task.base, task.bytes);
			}
			task_done.enqueue(&task);
		}
	}
}

void tal_init(struct tal_args_t arg) {
	meta.rank = arg.rank;
	meta.size = arg.size;
	if(arg.rank == 0) {
		meta.fd = new int[arg.size];

		int listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		int Reuseaddr=1;
		if(setsockopt(listenfd, SOL_SOCKET ,SO_REUSEADDR, &Reuseaddr, sizeof(Reuseaddr)) != 0){
			printf("set reuse failed\n");
			exit(1);
		}

		if(bind(listenfd, (struct sockaddr*)&arg.ps_server_addr, sizeof(arg.ps_server_addr)) != 0) {
			printf("bind failed\n");
			exit(2);
		}
		
		if(listen(listenfd, meta.size) != 0) {
			printf("listen failed\n");
			exit(3);
		}

		for(int i = 1; i < arg.size; i++) {
			do {
				printf("accepting thread %d\n", i);
				meta.fd[i] = accept(listenfd, NULL, NULL);
			}
			while(meta.fd[i] == -1);
		}
		close(listenfd);
	}
	else {
		meta.fd = new int[1];

		meta.fd[0] = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		int ret;
		do {
			printf("connecting ps\n");
			ret = connect(meta.fd[0], (struct sockaddr*)&arg.ps_server_addr, sizeof(arg.ps_server_addr));
		}
		while(ret != 0);
	}
	reduce_thread = new std::thread(tal_reduce);
}

void tal_exit(void) {
	if(meta.rank == 0) {
		for(int i = 1; i < meta.size; i++) {
			close(meta.fd[i]);
		}
	}
	else {
		close(meta.fd[0]);
	}
	delete [] meta.fd;

	force_quit = true;
	reduce_thread->join();
	delete reduce_thread;
}

void tal_push(struct tal_task_t task) {
	// printf("push key=%ld\n", task.key);
	task_todo.enqueue(&task);
}

int64_t tal_poll(void) {
	int64_t key = -1;
	struct tal_task_t task;
	if (task_done.dequeue(&task) == 0) {
		return -1;
	} else {
		return task.key;
	}
}
