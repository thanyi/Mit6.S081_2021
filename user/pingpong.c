#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char const *argv[])
{
    /* code */

    int p[2];
    pipe(p);
    int pid = fork();
    
    if(pid == 0){
        char ping;
        read(p[0],&ping,1);
        int pid = getpid();

        printf("%d: received ping\n",pid);
        char pong = 'a';
        write(p[1],&pong,1);
        close(p[1]);  // 端口写完了，关闭端口

        exit(0);
    }else{
        char ping = 'a';
        write(p[1],&ping,1);  // 开始进行传输ping

        
        char pong;
        read(p[0],&pong,1);
        int pid = getpid();

        printf("%d: received pong\n",pid);
        close(p[0]);

        exit(0);
    }


    

    // return 0;
}
