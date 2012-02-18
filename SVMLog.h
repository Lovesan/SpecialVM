#ifndef __SVM_LOG_H__
#define __SVM_LOH_H__

#include "SVMTypes.h"

SVM_WORD SVMInitLog();
SVM_WORD SVMCloseLog();
SVM_WORD SVMLogPrint(SVM_PCSTR format, ...);

#endif // __SVM_LOG_H__
