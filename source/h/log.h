/* log.c */
#include <time.h>
int init_log(void);
void logData(int logFile, double *val, int nval, struct timespec *tstamp);
void timespec_diff(struct timespec *r, struct timespec *x, struct timespec *y);
