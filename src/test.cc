#include "../tal.h"
#include <cstdio>
#include <cstdlib>
using namespace std;

// ./test <size> <rank> <ps_ip> <ps_port>
int main(int argc, char **argv)
{
    if(argc != 5) {
        printf("Usage: test <size> <rank> <ps_ip> <ps_port>\n");
        exit(1);
    }
    tal_args_t init_arg;
    init_arg.size = atoi(argv[1]);
    init_arg.rank = atoi(argv[2]);
    init_arg.ps_server_addr.sin_family = AF_INET;
    init_arg.ps_server_addr.sin_addr.s_addr = inet_addr(argv[3]);
    init_arg.ps_server_addr.sin_port = htons((short)atoi(argv[4]));

    tal_init(init_arg);

    const int task_num = 10;
    const int test_len = 5;
    tal_task_t *task = new tal_task_t[task_num];

    //if(init_arg.rank & 1) {
        for(int task_id = 0; task_id < task_num; task_id++) {
            task[task_id].base = new int[test_len];
            for(int pos = 0; pos < test_len; pos++) {
                ((int*)task[task_id].base)[pos] = init_arg.rank << 20 | task_id << 10 | pos;
            }
            task[task_id].bytes = test_len * sizeof(int);
            task[task_id].key = task_id;

            tal_push(task[task_id]);
        }
    /*}
    else {
        for(int task_id = task_num-1; task_id >= 0; task_id--) {
            task[task_id].base = new int[test_len];
            for(int pos = 0; pos < test_len; pos++) {
                ((int*)task[task_id].base)[pos] = init_arg.rank << 20 | task_id << 10 | pos;
            }
            task[task_id].bytes = test_len;
            task[task_id].key = task_id;

            tal_push(task[task_id]);
        }
    }*/
        
    for(int cnt = task_num; cnt; ){
        int task_id = tal_poll();
        if(task_id == -1) continue;
        //printf("task_id: %d\n", task_id);
        cnt --;
        for(int pos = 0; pos < test_len; pos++) {
            if( ((int*)task[task_id].base)[pos] != 
                ((init_arg.size-1)*init_arg.size/2 << 20 | task_id*init_arg.size << 10 | pos*init_arg.size)
            ) {
                printf("error\n");
                exit(2);
            }
        }
    }
    delete task;

    tal_exit();
    printf("success\n");
    return 0;
}