#ifndef __SVM_H__
#define __SVM_H__

#include "SVMTypes.h"
#include "SVMOpts.h"
#include "SVMCon.h"
#include "SVMLog.h"
#include "SVMDisk.h"

SVM_BYTE* SVMInitMemory();
SVM_WORD SVMCloseMemory();
SVM_WORD SVMMain();

#endif // __SVM_H__
