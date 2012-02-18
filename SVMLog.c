#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "SVMOpts.h"
#include "SVMLog.h"

static FILE* hLog;

SVM_WORD SVMInitLog()
{
  SVM_PCSTR fileName = SVMGetLogFileName();
  if(!fileName)
    return 1;
  hLog = fopen(fileName, "w");  
  if(!hLog)
  {
    fprintf(stderr, "Unable to open log file for writing\n");
    return 0;
  }
  return 1;
}

SVM_WORD SVMCloseLog()
{
  if(!hLog)
    return 1;
  return (SVM_WORD)fclose(hLog);
}

SVM_WORD SVMLogPrint(SVM_PCSTR format, ...)
{
  int n;  
  va_list args;
  if(!hLog)
    return 1;
  va_start(args, format);
  n = vfprintf(hLog, format, args);
  va_end(args);
  return n;
}