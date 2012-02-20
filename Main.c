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
  if(!SVMInitMemory())
  {
    SVMCloseDisks();
    SVMCloseLog();
    return 1;
  }
  if(!SVMInitConsole())
  {
    SVMCloseMemory();
    SVMCloseDisks();
    SVMCloseLog();
    return 1;
  }
  rv = SVMMain();  
  SVMCloseConsole();
  SVMCloseMemory();
  SVMCloseDisks();
  SVMCloseLog();
return (int)!rv;
}
