#include <stdio.h>
#include <unistd.h>	
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
int p[2];
int p1[2];

int main(int argc, char *argv[]){
    char mode = 's';
    if(argc >= 2){
        mode = argv[1][0];
        //printf("%c\n",mode);
    }
    //printf("%c\n",mode);
    if(mode == 's'){
        pipe(p);
        pipe(p1);
        printf("+++ CSE in supervisor mode: Started\n");
        printf("+++ CSE in supervisor mode: pfd = [%d %d]\n",p[0],p[1]);
        printf("+++ CSE in supervisor mode: forking first child\n");
        int pid = fork();
        if(pid>0){
            printf("+++ CSE in supervisor mode: forking second child\n");
            int pid1 = fork();
            if(pid1>0){
                waitpid(pid,NULL,0);
                printf("+++ CSE in supervisor mode: First child terminated\n");
                waitpid(pid1,NULL,0);
                printf("+++ CSE in supervisor mode: Second child terminated\n");
            }
            else{
                char buf[100];
                char buf1[100];
                sprintf(buf,"%d",p[0]);
                sprintf(buf1,"%d",p1[1]);
                execlp("xterm","xterm","-T","Second Child","-e","./CSE","e",buf,buf1,(char*)0);
                printf("this shouldn't happen\n");
            }
        }
        else{
            char buf[100];
            char buf1[100];
            sprintf(buf,"%d",p[1]);
            sprintf(buf1,"%d",p1[0]);
            execlp("xterm","xterm","-T","First Child","-e","./CSE","c",buf,buf1,(char*)0);
            printf("this shouldn't happen\n");
        }
    }
    else if(mode == 'c'){
        //close(1);
        int cop = dup(1);
        int cop1 = dup(0);
        int j = atoi(argv[2]);
        int j1 = atoi(argv[3]);
        int k = dup2(j,1);
        while(1){
            if(mode == 'c'){
                write(cop,"Enter Command>",15);
                char s[1000];
                fflush(stdin);
                scanf("%[^\n]%*c", s);
                printf("%s\n",s);
                fflush(stdout);
                if(!strcmp(s,"swaprole")){
                    mode = 'e';
                    close(1);
                    dup2(cop,1);
                    dup2(j1,0);
                    continue;
                }
                else if(!strcmp(s,"exit")){
                    exit(0);
                }
            }
            else{
                printf("Waiting for command>");
                fflush(stdout);
                char s[1000];
                //flush(stdin);
                scanf("%[^\n]%*c", s);
                printf("%s\n",s);
                fflush(stdout);
                if(!strcmp("swaprole",s)){
                    mode = 'c';
                    close(0);
                    dup2(cop1,0);
                    dup2(j,1);
                    continue;
                }
                else if(!strcmp(s,"exit")){
                    exit(0);
                }
                int ns = 0;
                for(int i=0;i<strlen(s);i++){
                    if(s[i] == ' ') ns++;
                }
                char *t[ns+2];
                char* token = strtok(s, " ");
                int i = 0;
                while (token != NULL && i<=ns) {
                    //printf("%d %s\n",i,token);
                    t[i] = (char*)malloc(sizeof(char)*(strlen(token)+1));
                    strcpy(t[i],token);
                    token = strtok(NULL, " ");
                    i++;
                }
                t[ns+1] = NULL;
                //sleep(10);
                int pi = fork();
                if(pi>0){
                    waitpid(pi,NULL,0);
                }
                else{
                    dup2(cop1,0);
                    dup2(cop,1);
                    execvp(t[0],t);
                }
            }
        }
    }
    else if(mode == 'e'){
        //close(0);
        int cop = dup(1);
        int cop1 = dup(0);
        int j = atoi(argv[2]);
        int j1 = atoi(argv[3]);
        int k = dup2(j,0);
        while(1){
            if(mode == 'e'){
            printf("Waiting for command>");
            fflush(stdout);
            char s[1000];
            //flush(stdin);
            scanf("%[^\n]%*c", s);
            printf("%s\n",s);
            fflush(stdout);
            if(!strcmp("swaprole",s)){
                    mode = 'c';
                    close(0);
                    dup2(cop1,0);
                    dup2(j1,1);
                    continue;
            }
            else if(!strcmp(s,"exit")){
                exit(0);
            }
            int ns = 0;
            for(int i=0;i<strlen(s);i++){
                if(s[i] == ' ') ns++;
            }
            char *t[ns+2];
            char* token = strtok(s, " ");
            int i = 0;
            while (token != NULL && i<=ns) {
                //printf("%d %s\n",i,token);
                t[i] = (char*)malloc(sizeof(char)*(strlen(token)+1));
                strcpy(t[i],token);
                token = strtok(NULL, " ");
                i++;
            }
            t[ns+1] = NULL;
            //sleep(10);
            int pi = fork();
            if(pi>0){
                waitpid(pi,NULL,0);
            }
            else{
                dup2(cop1,0);
                dup2(cop,1);
                execvp(t[0],t);
            }
            }
            else{
                write(cop,"Enter Command>",15);
                char s[1000];
                fflush(stdin);
                scanf("%[^\n]%*c", s);
                printf("%s\n",s);
                fflush(stdout);
                if(!strcmp(s,"swaprole")){
                    mode = 'e';
                    close(0);
                    dup2(cop,1);
                    dup2(j,0);
                    continue;
                }
                else if(!strcmp(s,"exit")){
                    exit(0);
                }
            }
        }
    }
    else{
        printf("Invalid mode %c\n",mode);
    }
}
