#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<string.h>

#define MAX 1000

int main(int argc, char *argv[])
{
    int nind;
    if(argc == 2){
        nind = 0;
    }
    else{
        nind = atoi(argv[2]);
    }
    FILE *fptr = fopen("treeinfo.txt","r");
    char BUF[MAX];
    char *z;
    int nchild;
    char city[MAX];
    while( (z = fgets(BUF,MAX,fptr)) != NULL){
        sscanf(BUF,"%s %d",city,&nchild);
        if(!strcmp(city,argv[1])){
            //printf("Found %s with %d children\n",city,nchild);
             for(int i=0;i<nind;i++){
                printf("\t");
            }
            printf("%s(%d)\n",city,getpid());
            int i;
            for(i=0;BUF[i]!=' ';i++);
            i++;
            for(;BUF[i]!=' ';i++);
            i++;
            for(int j=0;j<nchild;j++){
                int k = i;
                int len = 1;
                for(;BUF[i]!=' '&&BUF[i]!='\n';i++) len++;
                i++;
                char cit[len];
                for(int te = 0;k<i-1;k++,te++){
                    cit[te] = BUF[k];
                }
                cit[len-1] = '\0';
                char TEM[MAX];
                sprintf(TEM,"%d",nind + 1);
                int rp = fork();
                if(rp == 0){
                    execl(argv[0],argv[0],cit,TEM,(char *)(0));
                    printf("Error\n");
                }
                else{
                    waitpid(rp,NULL,0);
                }
            }
            break;
        }
    }
    if(z == NULL) printf("Couldn't find city %s\n",argv[1]);
}
