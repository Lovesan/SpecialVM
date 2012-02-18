#ifndef __SVM_CON_H__
#define __SVM_CON_H__

#include "SVMTypes.h"

SVM_WORD SVMInitConsole();
SVM_WORD SVMCloseConsole();
SVM_WORD SVMSetSymbol(SVM_WORD idx, SVM_WORD sym);
SVM_WORD SVMGetSymbol(SVM_WORD idx);
SVM_WORD SVMSetCursorPos(SVM_WORD pos);
SVM_WORD SVMGetCursorPos();
SVM_WORD SVMSetCursorVisibility(SVM_WORD visible);
SVM_WORD SVMGetCursorVisibility();
SVM_INT SVMPeekConsoleInput();

#endif // __SVM_CON_H__
