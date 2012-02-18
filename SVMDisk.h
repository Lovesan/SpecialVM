#ifndef __SVM_DISK_H__
#define __SVM_DISK_H__

#include "SVMTypes.h"
#include "SVMOpts.h"

#define SVM_SECTOR_SIZE 512

SVM_WORD SVMInitDisks();
SVM_WORD SVMCloseDisks();
SVM_WORD SVMSetDiskSector(SVM_WORD nDisk, SVM_INT sector);
SVM_INT SVMGetDiskSector(SVM_WORD nDisk);
SVM_WORD SVMReadSector(SVM_WORD nDisk, SVM_WORD *buffer);
SVM_WORD SVMWriteSector(SVM_WORD nDisk, SVM_WORD *buffer);


#endif // __SVM_DISK_H__