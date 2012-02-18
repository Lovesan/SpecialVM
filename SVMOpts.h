#ifndef __SVM_OPTS_H__
#define __SVM_OPTS_H__

#include "SVMTypes.h"

#define SVM_MAX_DISKS 4
#define SVM_DEFAULT_MEMORY_SIZE 32

SVM_WORD SVMInitOpts(int argc, char **argv);
SVM_WORD SVMGetDiskCount();
SVM_PCSTR SVMGetDiskFileName(SVM_WORD nDisk);
SVM_PCSTR SVMGetLogFileName();
SVM_WORD SVMGetMemSize();

#endif // __SVM_OPTS_H__
