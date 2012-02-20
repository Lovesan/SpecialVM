#ifndef __SVM_H__
#define __SVM_H__

#include "SVMTypes.h"
#include "SVMOpts.h"
#include "SVMCon.h"
#include "SVMLog.h"
#include "SVMDisk.h"
#include "SVMSleep.h"

#define CLOCKS_PER_MS (CLOCKS_PER_SEC / 1000)
#define SVM_TIMER_MS (10)
#define SVM_TIMER_CLOCKS (SVM_TIMER_MS * CLOCKS_PER_MS)

typedef enum
{
  SVM_IR_DIV0,
  SVM_IR_INVALID_INST,
  SVM_IR_PAGE_FAULT,
  SVM_IR_DOUBLE_FAULT,
  SVM_IR_ALIGNMENT_FAULT,
  SVM_IR_TIMER = 16,
  SVM_IR_KEYBOARD,
  SVM_IR_DISK,
} SVM_IR;

typedef struct
{
    SVM_INT ir;
    SVM_WORD ipr;
    SVM_WORD flr;
} SVM_IR_DESC;

SVM_BYTE* SVMInitMemory();
SVM_WORD SVMCloseMemory();
SVM_WORD SVMLogContext();
SVM_WORD SVMMain();

#endif // __SVM_H__
