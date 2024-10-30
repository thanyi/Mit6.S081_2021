#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

/* xargs命令：
    实现：将管道符前的命令执行后的每一行输出单独作为管道符后命令的参数
*/

// new_args: 保存的参数数组     lastarg: 当前的参数idx
int readline(char **new_args,int lastarg ){
    char input_buf[1024];
    int n = 0;
    
    // 从标准输入读取，存储到新的new_args中
    // 从管道符后的输出也会被看作是标准输入
    while ( read(0, input_buf+n ,1)!= 0 )
    {
        if (n == 1023){
            fprintf(2,"Usage: too long args.\n");
            exit(1);
        }
        if (input_buf[n] == '\n')
        {
            // 这里直接break可以和循环外对接上
            break;
        }
        n++;
    }
    
    input_buf[n] = 0;
    if (n == 0)return 0;
    
    // 为参数分配新的内存并复制内容
    new_args[lastarg] = malloc(strlen(input_buf) + 1);
    strcpy(new_args[lastarg], input_buf);

    return lastarg;
}


int main(int argc, char *argv[])
{
	if (argc <= 1)
    {
       fprintf(2,"Usage: xargs command (arg ...)\n");
    }

    char *newargv[MAXARG];

    // xargs要使用的命令为command
    char * command = argv[1];   

    int i;
    // 将xargs之后的参数放入newargv,注意不要读到脏数据
    for (i = 1; i < argc; i++)
    {
        newargv[i-1] = argv[i];   
    }
    
    int lastarg =  argc - 1;
    while (readline(newargv, lastarg) != 0)
    {   
        // 不能让lastarg发生变化，因为这是循环调用command newargv，只需要让最后一个参数变化
        if (lastarg >= MAXARG - 1) { 
            fprintf(2, "Usage: too many args.\n");
            exit(1);
        }

        if (fork() == 0)
        {
            exec(command,newargv);
            fprintf(2, "exec failed\n");
            exit(1);
        }
        wait(0);
        
    }
    
    exit(0);
}