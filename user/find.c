#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

// startpath: 目录文件  filename：希望查找文件
void findfile(char* dirpath, char* filename){
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    /* 验证是否可以打开目录文件 */
    if((fd = open(dirpath, 0)) < 0){
        fprintf(2, "ls: cannot open %s\n", dirpath);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "ls: cannot stat %s\n", dirpath);
        close(fd);
        return;
    }


    // "dirpath/filename"的长度
    if(strlen(dirpath) + 1 + DIRSIZ + 1 > sizeof buf){
        printf("ls: path too long\n");
    }
    strcpy(buf, dirpath);
    p = buf+strlen(buf);
    *p++ = '/';

    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)    // 当为0时代表数据不存在
            continue;
        if (strcmp(de.name,".") == 0 || strcmp(de.name,"..") == 0  )
        {   // 当为.以及..目录直接跳过
            continue;
        }

        // 此时buf内容是"dirpath/filename"
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){     // stat系统调用
            printf("ls: cannot stat %s\n", buf);
            continue;
        }

        switch(st.type){
            case T_FILE:
                if (strcmp(de.name, filename) == 0)
                {
                    printf("%s\n",buf);
                }
                break;
            case T_DIR:
                findfile(buf,filename);
                break;
        }
        
    }
}


int main(int argc, char *argv[])
{
    if(argc < 2){
        fprintf(2, "Usage: find <path> <filename>...\n");
        exit(1);
    }

    char *path = argv[1];
    char *filename = argv[2];
    findfile(path,filename);
    exit(0);
}
