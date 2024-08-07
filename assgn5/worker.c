#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>	
#include <sys/shm.h>

int main(int argc, char *argv[]){
    int n = atoi(argv[1]);
    int k = atoi(argv[2]);
    //printf("%d %d\n",n,k);
    int n2a = shmget(ftok(".",1001),n*n*sizeof(int),IPC_CREAT);
    int *a;
    int i;
    a = (int *)shmat(n2a,NULL,0);
    int num[n];
    for(i=0;i<n;i++){
        num[i] = 0;
    }
    for(i = 0;i<n;i++){
        for(int j =0;j<n;j++){
            if(a[n*i + j]){
                num[j]++;
            }
            //printf("%d ",a[n*i+j]);
        }
        //printf("\n");
    }
    struct sembuf vop,pop;
    pop.sem_num = vop.sem_num = 0;
	pop.sem_flg = vop.sem_flg = 0;
	pop.sem_op = -num[k]; vop.sem_op = 1 ;
    int nw = semget(ftok(".",1004),n,IPC_CREAT|0666);
    int nin = semget(ftok(".",1005),1,IPC_CREAT|0666);
    int ni = shmget(ftok(".",1003),1*sizeof(int),IPC_CREAT);
    int nt = shmget(ftok(".",1002),n*sizeof(int),IPC_CREAT);
    int *res = shmat(nt,NULL,0);
    int *ind = shmat(ni,NULL,0);
    pop.sem_num = k;
    semop(nw,&pop,1);
    //printf("entered %d\n",k);
    struct sembuf temp;
    temp.sem_flg = 0;
    temp.sem_num = 0;
    temp.sem_op = -1;
    semop(nin,&temp,1);
    //printf("entered %d\n",k);
    res[ind[0]] = k;
    //printf("entered %d ",ind[0]);
    ind[0]++;
    //printf("%d\n",ind[0]);
    struct sembuf temp1;
    temp1.sem_flg = 0;
    temp1.sem_num = 0;
    temp1.sem_op = 1;
    semop(nin,&temp1,1);
    for(i=0;i<n;i++){
        if(a[n*k+i]){
            vop.sem_num = i;
            semop(nw,&vop,1);
        }
    }
    return 0;
}
