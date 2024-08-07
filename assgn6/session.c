#include <stdio.h>
#include <stdlib.h>
#include <string.h>   
#include <unistd.h>  
#include <pthread.h>  
#include "event.h"

typedef struct {
    int no;
    int arr;
    int dur;
}info;

pthread_cond_t doctor;
pthread_cond_t patient;
pthread_cond_t reports;
pthread_cond_t sales;
int ndavail = 0;
int endsess = 0;
int npwait = 0;
int nrwait = 0;
int nswait = 0;
int timer = 0;
int done = 1;
pthread_mutex_t mfordo;
pthread_cond_t  don;
pthread_mutex_t mford;
pthread_mutex_t mfort;
pthread_mutex_t mforr;
pthread_mutex_t mforp;
pthread_mutex_t mfors;
eventQ E;
pthread_mutex_t mfore;

void printtime(int t){
    //t can be negative and t is no of minutes from 9am
    int h = 0;
    while(t < 0){
        t = t + 60;
        h--;
    }
    while(t >= 60){
        t = t - 60;
        h++;
    }
    int m = t%60;
    int a = 0;
    if(9+h >= 12) a = 1;
    if(9+h> 12) h = h - 12;
    printf("%2d:%2d ",9+h,m);
    if(a) printf("pm");
    else printf("am");
}

void *doc(){
    while(1){
        pthread_mutex_lock(&mford);
        pthread_cond_wait(&doctor,&mford);
        ndavail--;
        if(endsess){
            pthread_mutex_unlock(&mford);
            pthread_exit(0);
        }
        //printf("Doctor finish\n");
        pthread_mutex_unlock(&mford);
    }
}

void *patient_routine(void *dur){
    pthread_mutex_lock(&mforp);
    info *z = (info *)dur;
    //printf("patient %d waiting\n",z->no);
    npwait++;
    pthread_cond_wait(&patient,&mforp);
    npwait--;
    pthread_mutex_lock(&mfort);
    printf("[");
    printtime(timer);
    printtime(timer + z->dur);
    printf("] Patient %d in doctor's chamber\n",z->no);
    event temp;
    temp.type = 'D';
    temp.time = timer + z->dur;
    temp.duration = 0;
    pthread_mutex_lock(&mfore);
    E = addevent(E,temp);
    pthread_mutex_unlock(&mfore);
    pthread_mutex_unlock(&mfort);
    pthread_mutex_unlock(&mforp);
    pthread_mutex_lock(&mfordo);
    done = 1;
    pthread_cond_signal(&don);
    pthread_mutex_unlock(&mfordo);
    pthread_exit(0);
}

void *reporter_routine(void *dur){
    pthread_mutex_lock(&mforr);
    info *z = (info *)dur;
    //printf("Reporter %d waiting\n",z->no);
    nrwait++;
    pthread_cond_wait(&reports,&mforr);
    nrwait--;
    pthread_mutex_lock(&mfort);
    printf("[");
    printtime(timer);
    printtime(timer + z->dur);
    printf("] Reporter %d in doctor's chamber\n",z->no);
    event temp;
    temp.type = 'D';
    temp.time = timer + z->dur;
    temp.duration = 0;
    pthread_mutex_lock(&mfore);
    E = addevent(E,temp);
    pthread_mutex_unlock(&mfore);
    pthread_mutex_unlock(&mfort);
    pthread_mutex_unlock(&mforr);
    pthread_mutex_lock(&mfordo);
    done = 1;
    pthread_cond_signal(&don);
    pthread_mutex_unlock(&mfordo);
    pthread_exit(0);
}

void *sales_routine(void *dur){
    pthread_mutex_lock(&mfors);
    info *z = (info *)dur;
    //printf("Salesman %d waiting\n",z->no);
    nswait++;
    pthread_cond_wait(&sales,&mfors);
    nswait--;
    pthread_mutex_lock(&mfort);
    printf("[");
    printtime(timer);
    printtime(timer + z->dur);
    printf("] Salesman %d in doctor's chamber\n",z->no);
    event temp;
    temp.type = 'D';
    temp.time = timer + z->dur;
    temp.duration = 0;
    pthread_mutex_lock(&mfore);
    E = addevent(E,temp);
    pthread_mutex_unlock(&mfore);
    pthread_mutex_unlock(&mfort);
    pthread_mutex_unlock(&mfors);
    pthread_mutex_lock(&mfordo);
    done = 1;
    pthread_cond_signal(&don);
    pthread_mutex_unlock(&mfordo);
    pthread_exit(0);
}

int main(){
    //initializing the mutexes and condition variables
    pthread_mutex_init(&mford,NULL);
    pthread_mutex_init(&mfort,NULL);
    pthread_mutex_init(&mforr,NULL);
    pthread_mutex_init(&mforp,NULL);
    pthread_mutex_init(&mfors,NULL);
    pthread_mutex_init(&mfore,NULL);
    pthread_cond_init(&doctor,NULL);
    pthread_cond_init(&patient,NULL);
    pthread_cond_init(&reports,NULL);
    pthread_cond_init(&sales,NULL);
    pthread_t d;
    pthread_create(&d,NULL,doc,NULL);
    E = initEQ("arrival.txt");
    event e;
    e.type = 'D';
    e.time = 0;
    e.duration = 0;
    E = addevent(E,e);
    int pattok = 0;
    int reptok = 0;
    int saletok = 0;
    //visually displaying the events
    while(!emptyQ(E)){
        //displayEQ(E);
        pthread_mutex_lock(&mfordo);
        while(!done){
            pthread_cond_wait(&don,&mfordo);
        }
        pthread_mutex_unlock(&mfordo);
        pthread_mutex_lock(&mfore);
        e = nextevent(E);
        E = delevent(E);
        pthread_mutex_unlock(&mfore);
        pthread_mutex_lock(&mfort);
        timer = e.time;
        //printtime(timer);
        pthread_mutex_unlock(&mfort);
        if(e.type == 'D'){
            pthread_mutex_lock(&mford);
            ndavail++;
            printf("[");
            printtime(e.time);
            printf("] Doctor arrives\n");
            pthread_mutex_unlock(&mford);
        }
        if(e.type == 'P'){
            pattok++;
            printf("\t\t[");
            printtime(e.time);
            printf("] Patient %d arrives\n",pattok);
            if(endsess){
                printf("\t\t[");
                printtime(e.time);
                printf("] Patient %d leaves(Session completed)\n",pattok);
                continue;
            }
            if(pattok>25){
                printf("\t\t[");
                printtime(e.time);
                printf("] Patient %d leaves(Quota Full)\n",pattok);
            }
            else{
            info z;
            z.no = pattok;
            z.arr = e.time;
            z.dur = e.duration;
            pthread_t p;
            pthread_create(&p,NULL,patient_routine,&z);
            }
        }
        else if(e.type == 'R'){
            reptok++;
            printf("\t\t[");
            printtime(e.time);
            printf("] Reporter %d arrives\n",reptok);
            if(endsess){
                printf("\t\t[");
                printtime(e.time);
                printf("] Reporter %d leaves(Session completed)\n",reptok);
                continue;
            }
            info z;
            z.no = reptok;
            z.arr = e.time;
            z.dur = e.duration;
            pthread_t r;
            pthread_create(&r,NULL,reporter_routine,&z);
        }
        else if(e.type == 'S'){
            saletok++;
            printf("\t\t[");
            printtime(e.time);
            printf("] Salesman %d arrives\n",saletok);
            if(endsess){
                printf("\t\t[");
                printtime(e.time);
                printf("] Salesman %d leaves(Session completed)\n",saletok);
                continue;
            }
            if(saletok>3){
                printf("\t\t[");
                printtime(e.time);
                printf("] Salesman %d leaves(Quota Full)\n",saletok);
            }
            else{
            info z;
            z.no = saletok;
            z.arr = e.time;
            z.dur = e.duration;
            pthread_t s;
            pthread_create(&s,NULL,sales_routine,&z);
            }
        }
        if(endsess) continue;
        pthread_mutex_lock(&mford);
        if(ndavail){
            pthread_mutex_lock(&mforr);
            pthread_mutex_lock(&mforp);
            pthread_mutex_lock(&mfors);
            pthread_mutex_lock(&mfordo);
            done = 0;
            //printf("rw %d pw %d sw %d\n",nrwait,npwait,nswait);
            if(nrwait){
                pthread_cond_signal(&reports);
                pthread_cond_signal(&doctor);
                printf("[");
                printtime(timer);
                printf("] New Visitor for doctor\n");
            }
            else if(npwait){
                pthread_cond_signal(&patient);
                pthread_cond_signal(&doctor);
                printf("[");
                printtime(timer);
                printf("] New Visitor for doctor\n");
            }
            else if(nswait){
                pthread_cond_signal(&sales);
                pthread_cond_signal(&doctor);
                printf("[");
                printtime(timer);
                printf("] New Visitor for doctor\n");
            }
            else{
                if(pattok > 25 && saletok > 3){
                    endsess = 1;
                    pthread_cond_signal(&doctor);
                    printf("[");
                    printtime(timer);
                    printf("] Doctor leaves\n");
                }
                done = 1;
            }
            pthread_mutex_unlock(&mfordo);
            pthread_mutex_unlock(&mforr);
            pthread_mutex_unlock(&mforp);
            pthread_mutex_unlock(&mfors);
        }
        pthread_mutex_unlock(&mford);
    }
}

