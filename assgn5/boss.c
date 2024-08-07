#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>	
#include <sys/shm.h>
#include <fcntl.h>

int main()
{
    FILE *fp = fopen("graph.txt","r");
    //char s[1000];
    int n;
    fscanf(fp,"%d",&n);
    //printf("%d\n",n);
    int n2a = shmget(ftok(".",1001),(n*n)*sizeof(int),IPC_CREAT|0777);
    int *a;
    a = (int *)shmat(n2a,0,0);
    int nt = shmget(ftok(".",1002),n*sizeof(int),IPC_CREAT|0777);
    int ni = shmget(ftok(".",1003),1*sizeof(int),IPC_CREAT|0777);
    int i = 0;
    for(i=0;i<n;i++){
        for(int j = 0;j<n;j++){
            fscanf(fp,"%d",&a[i*n+j]);
            //printf("%d ",a[i*n+j]);
        }
        //printf("\n");
    }
    int *idx;
    idx = (int *)shmat(ni,0,0);
    *idx = 0;
    int num[n];
    for(i=0;i<n;i++){
        num[i] = 0;
    }
    for(i = 0;i<n;i++){
        for(int j =0;j<n;j++){
            if(a[n*i + j]){
                num[j]++;
            }
        }
    }
    int nw = semget(ftok(".",1004),n,IPC_CREAT|0666);
    int nin = semget(ftok(".",1005),1,IPC_CREAT|0666);
    // if(nw == -1){
    //     printf("Error\n");
    // }
    // if(nin == -1){
    //     printf("Error\n");
    // }
    for(int i=0;i<n;i++){
        semctl(nw,i,SETVAL,0);
    }
    semctl(nin,0,SETVAL,1);
    //printf("%d\n",semctl(nin,0,GETVAL));
    printf("+++ Topological sorting of the vertices is: \n");
    while(idx[0] != n){}
    int *res;
    res = shmat(nt,NULL,SHM_RDONLY);
    for(i=0;i<n;i++){
        printf("%d ",res[i]);
    }
    printf("\n");
    for(int i=0;i<n;i++){
        for(int j = 0;j<n;j++){
            if(a[res[i]*n+j]){
                num[j]--;
            }
        }
    }
    int sum = 0;
    for(int i=0;i<n;i++){
        sum += num[i];
    }
    if(sum){
        printf("error\n");
    }
    else{
        printf("+++ Well done team\n");
    }
    semctl(nw,0,IPC_RMID,0);
    semctl(nin,0,IPC_RMID,0);
    shmctl(n2a,IPC_RMID,0);
    shmctl(nt,IPC_RMID,0);
    shmctl(ni,IPC_RMID,0);
    return 0;
}
