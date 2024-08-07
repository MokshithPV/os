#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct node
{
    int pid;
    int gid;
    int status;
    char a;
}node;

int cur_p;

node process[11];
int nprocess = 1;

void print(){
    printf("NO\tPID\tPGID\tSTATUS\tNAME\n");
    for(int i=0;i<nprocess;i++){
        printf("%d\t%d\t%d\t",i,process[i].pid,process[i].gid);
        if(process[i].status == 0){
            printf("SELF\t");
        }
        else if(process[i].status == 1){
            printf("FINISHED\t");
        }
        else if(process[i].status == 2){
            printf("TERMINATED\t");
        }
        else if(process[i].status == 3){
            printf("SUSPENDED\t");
        }
        else if(process[i].status == 4){
            printf("KILLED\t");
        }
        if(process[i].a == '\0'){
            printf("mgr\n");
        }
        else{
            printf("job %c\n",process[i].a);
        }
    }
}

void cc(){
    if(getpid() != cur_p){
        kill(cur_p,SIGINT);
        for(int i=1;i<11;i++){
            if(cur_p == process[i].pid){
                process[i].status = 2;
                break;
            }
        }
    }
    printf("\n");
}

void zc(){
    if(getpid() != cur_p){
        kill(cur_p,SIGTSTP);
        for(int i=1;i<11;i++){
            if(cur_p == process[i].pid){
                process[i].status = 3;
                break;
            }
        }
    }
    printf("\n");
}

void cc1(){

}

int main()
{
    process[0].a = '\0';
    process[0].status = 0;
    process[0].pid = getpid();
    process[0].gid = getpgrp();
    char c;
    char c1;
    srand(time(NULL));
    //cur_p = getpid();
    int r;
    while(1){
        cur_p = getpid();
        signal(SIGINT,cc1);
        signal(SIGTSTP,cc1);
        fflush(stdin);
        fflush(stdout);
        printf("mgr> ");
        scanf(" %c",&c);
        fflush(stdin);
        fflush(stdout);
        if(c == 'h'){
            printf("Command : Action\nc : Continue a suspended job\nh : Print this help message\nk : Kill a suspended job\np : Print the process table\nq : Quit\nr : Run a new job\n");
        }
        else if(c == 'p'){
            print();
        }
        else if(c == 'r'){
            if(nprocess == 11){
                printf("Process table is full. Quiting...\n");
                for(int i=1;i<nprocess;i++){
                    if(process[i].status != 1){
                        kill(process[i].pid,SIGKILL);
                    }
                }
                exit(-1);
            }
            r = rand()%26;
            c1 = 'A' + r;
            int pid = fork();
            cur_p = pid;
            if(pid == 0){
                char s[2];
                sprintf(s,"%c",c1);
                setpgid(getpid(),0);
                execl("./job","./job",s,(char *)NULL);
                printf("Shouldn't\n");
            }
            else{
                signal(SIGINT,cc);
                signal(SIGTSTP,zc);
                int ret;
                nprocess++;
                process[nprocess-1].a = c1;
                process[nprocess-1].pid = pid;
                process[nprocess-1].gid = pid;
                waitpid(pid,&ret,WUNTRACED);
                if(WIFEXITED(ret)){
                    process[nprocess-1].status = 1;
                }
                else if(WIFSIGNALED(ret)){
                    process[nprocess-1].status = 2;
                }
                else if(WIFSTOPPED(ret)){
                    process[nprocess-1].status = 3;
                }
                cur_p = getpid();
            }
        }
        else if(c == 'k'){
            printf("Suspended jobs are: ");
            for(int i=1;i<nprocess;i++){
                if(process[i].status == 3){
                    printf("%d ",i);
                }
            }

                printf("\n");
            int ijk;
            printf("Select a process: ");
            scanf("%d",&ijk);
            kill(process[ijk].pid,SIGKILL);
            process[ijk].status = 4;
        }
        else if(c == 'c'){
            printf("Suspended jobs are: ");
            for(int i=1;i<nprocess;i++){
                if(process[i].status == 3){
                    printf("%d ",i);
                }
            }
            printf("\n");
            int ijk;
            printf("Select a process: ");
            scanf("%d",&ijk);
            signal(SIGINT,cc);
            signal(SIGTSTP,zc);
            kill(process[ijk].pid,SIGCONT);
            cur_p = process[ijk].pid;
            int ret;
            waitpid(process[ijk].pid,&ret,WUNTRACED);
            if(WIFEXITED(ret)){
                    process[ijk].status = 1;
                }
        }
        else if(c == 'q'){
            printf("Quiting...\n");
                for(int i=1;i<nprocess;i++){
                    if(process[i].status != 1){
                        kill(process[i].pid,SIGKILL);
                    }
                }
                exit(-1);
        }
    }
}

