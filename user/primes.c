#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


int filter_prime(int p[]){
    int prime; // 获取prime，第一个读取到的就是prime
    if(read(p[0] , &prime, 4) == 0 || prime >=35){
        // 证明此时read结束，没有来自父节点的输入了
        close(p[0]);
        close(p[1]);
        exit(0);
    }
    
    printf("prime %d\n",prime);

    int p2[2]; //向后传递数据使用的管道
    pipe(p2);
    int recv_int;
    if (fork() != 0)
    {
        close(p2[0]); // 父进程不用读取
        close(p[1]); // 父进程不用向左侧写入
        do
        {   
            // 筛选之后向子进程进行传输
            if(recv_int % prime != 0){
                write(p2[1], &recv_int, 4);
            }
        } while (read(p[0], &recv_int, 4) != 0);
        
        close(p2[1]);
        close(p[0]);
        wait(0); // 等待子进程结束
        exit(0);
    }else{
        close(p2[1]); // 子进程不写，仅读
        filter_prime(p2);
        close(p2[0]);
        exit(0);
    }
    
    
}



int main(int argc, char const *argv[])
{
    int p[2];
    pipe(p);

    if(fork() != 0){
        close(p[0]); // 父进程不用读取

        /* 父进程将所有数字放入子进程 */
        for(int i = 2; i<=35; i++){
            write(p[1], &i, 4);
        }
        close(p[1]);
        
        wait(0); // 等待子进程结束
        exit(0);
    }else{
        close(p[1]);
        filter_prime(p);
        exit(0);
    }
}
