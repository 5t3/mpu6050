#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

/*index of date string "Wed Jun 30 21:49:08 1993\n"*/
#define WEEKDAY 0
#define MONTH 4
#define DAY 8
#define HOUR 11
#define YEAR 20
/*logfile format*/
/*log row lenght*/
#define LROWLEN 89
/*values index*/
#define DATE 0
#define ACC_X 17
#define ACC_Y 27
#define ACC_Z 37
#define GYR_X 47
#define GYR_Y 57
#define GYR_Z 67
#define TEMP 77
#define INC 10

int init_log(){
/*get date*/
	time_t t=time(NULL);
/*parse date, ctime return value="Wed Jun 30 21:49:08 1993\n"*/
	char *date=ctime(&t);
	char delim=' ';
/*	char *wd=strtok(date,&delim);*/
/*skip week day*/
	strtok(date,&delim);
	char *m=strtok(NULL,&delim);
	char *d=strtok(NULL,&delim);
/*skip hour:minute:seconds*/
	strtok(NULL,&delim);
	char *y=strtok(NULL,"\n");
	char filename[20];
	sprintf(filename,"%s-%s-%s.log",y,m,d);
/*open an existing log file*/
	int log=open(filename,O_APPEND|O_RDWR);
/*error, file does not exist, create a new one*/
	if(log==-1){
		log=open(filename,O_CREAT|O_APPEND|O_RDWR,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if(log==-1){
		/*error, exit*/
			perror("Error on create/open log file");
			exit(1);
		}

		char header[81]={[0 ... 80] = ' '};
		strncpy(&header[DATE],"#DATE",5);
		strncpy(&header[ACC_X],"ACC_X",5);
		strncpy(&header[ACC_Y],"ACC_Y",5);
		strncpy(&header[ACC_Z],"ACC_Z",5);
		strncpy(&header[GYR_X],"GYR_X",5);
		strncpy(&header[GYR_Y],"GYR_Y",5);
		strncpy(&header[GYR_Z],"GYR_Z",5);
		strncpy(&header[TEMP],"TEMP",4);
		write(log,header,81);
	}
	return log;
}

void logData(int logFile, double *val,int nval, struct timespec *tstamp){
/*new empty log file row*/
	char lrow[LROWLEN]={[0]='\n',[1 ... LROWLEN-1]=' '};
/*temp for string converion*/
	char tmp[11];
	int strl=0;
/*get HH:MM:SS and millisec*/
	char *time=ctime(&(tstamp->tv_sec));
	int msec=tstamp->tv_nsec/1000000;
/*copy HH:MM:SS*/
	strncpy(&lrow[1],&time[HOUR],8);
/*convert and copy millisec*/
	sprintf(tmp,",%d",msec);
	strl=strlen(tmp);
	strncpy(&lrow[9],tmp,strl);
/*convert and copy sensor value*/
	int i=0;
	for(i;i<nval;i++){
		sprintf(tmp,"%.3f",val[i]);
		strl=strlen(tmp);
		strncpy(&lrow[ACC_X+1+(i*INC)],tmp,strl);
	/*add null byte to close the row*/	
		if(i==nval-1)
			strncpy(&lrow[ACC_X+1+(i*INC)+strl],"\0",1);
	}
	strl=strlen(lrow);
	write(logFile,lrow,strl);
}

/*return in *r the time elapsed between x (oldest value) and y (newest value)*/
void timespec_diff(struct timespec *r,struct timespec *x, struct timespec *y){
long asec,ansec,bsec,bnsec;
	asec=x->tv_sec;
	ansec=x->tv_nsec;
	bsec=y->tv_sec;
	bnsec=y->tv_nsec;

	r->tv_sec=bsec-asec;
	if(ansec<bnsec){
		r->tv_nsec=bnsec-ansec;
	}
	else{
		r->tv_nsec=(1000000000-ansec)+bnsec;
		r->tv_sec-=1;
	}
}
