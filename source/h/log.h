/* log.c */
#include <time.h>
int init_log(void);
void logData(int logFileStream, double *val, int nval, struct timespec *tstamp);
