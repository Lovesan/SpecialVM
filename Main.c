#include "SVM.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{  
  SVM_WORD rv;
  if(!SVMInitOpts(argc, argv))
    return 1;
  if(!SVMInitLog())
    return 1;
  if(!SVMInitDisks())
  {
    SVMCloseLog();
    return 1;
  }  
  if(!SVMInitConsole())
  {
    SVMCloseDisks();
    SVMCloseLog();
    return 1;
  }
  if(!SVMInitMemory())
  {
    SVMCloseConsole();
    SVMCloseDisks();
    SVMCloseLog();
    return 1;
  }
  rv = SVMMain();
  SVMCloseMemory();
  SVMCloseConsole();
  SVMCloseDisks();
  SVMCloseLog();
return rv;
}