#include "SVMSleep.h"

#ifdef _WIN32
#include <windows.h>

SVM_WORD SVMSleep(SVM_WORD milliseconds)
{
  Sleep(milliseconds);
  return 1;
}

#else
#include <unistd.h>

SVM_WORD SVMSleep(SVM_WORD milliseconds)
{
  usleep(1000 * milliseconds);
  return 1;
}

#endif // _WIN32