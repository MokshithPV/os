#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

int main()
{
    int shmid,status;
    int *a;
    int n,t;
    printf("n = ");
    scanf("%d",&n);
    printf("t = ");
    scanf("%d",&t);
    shmid = shmget(IPC_PRIVATE, 2*sizeof(int), 0777|IPC_CREAT);
    a = (int *) shmat(shmid, 0, 0);
    a[0] = 0;
    a[1] = 0;
    int num[n];
    int res[n];
    for(int i=0;i<n;i++){
        num[i] = 0;
        res[i] = 0;
    }
    for(int i=0;i<n;i++){
        if(fork()==0){
            printf("\t\t\tConsumer %d is alive\n",i+1);
            int *b;
            b = (int *) shmat(shmid, 0, 0);
            int nr=0,ns=0;
            while(b[0]!=-1){
                while(b[0]==0);
                if(b[0] == i+1){
                    nr++;
                    ns += b[1];
                    #ifdef VERBOSE
                    printf("\t\t\tConsumer %d reads %d\n",i+1,b[1]);
                    #endif
                    b[0] = 0;
                }
            }
            shmdt(b);
            printf("\t\t\tConsumer %d read %d items: Checksum = %d\n",i+1,nr,ns);
            exit(0);
        }
    }
    printf("Producer is alive\n");
    srand(time(0));
    int temp,temp1;
    for(int i=0;i<t;i++){
        while(a[0]);
        temp = rand()%900 + 100;
        temp1 = rand()%n + 1;
        a[0] = temp1;
        #ifdef SLEEP
        usleep(1);
        #endif
        a[1] = temp;
        #ifdef VERBOSE
        printf("Producer produces %d for consumer %d\n",temp,temp1);
        #endif
        num[temp1-1]++;
        res[temp1-1] += temp;
    }
    while(a[0]);
    a[0] = -1;
    for(int i=0;i<n;i++){
        wait(NULL);
    }
    printf("Prodecer has produced %d items\n",t);
    for(int i=0;i<n;i++){
        printf("%d items for consumer %d: Checksum = %d\n",num[i],i+1,res[i]);
    }
    shmdt(a);
    shmctl(shmid, IPC_RMID, 0);
    return 0;
}
