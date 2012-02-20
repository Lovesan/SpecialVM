#include <stdio.h>
#include <string.h>
#include "SVMOpts.h"

static SVM_PCSTR logFileName;
static SVM_WORD nDisks;
static SVM_PCSTR diskFileNames[SVM_MAX_DISKS];
static SVM_WORD memSize = SVM_DEFAULT_MEMORY_SIZE;
static const SVM_PCSTR usageMsg = "Usage: %s [-l logFileName] [-m MBsOfMemory] bootDiskFile [diskFile ...]\n";

SVM_WORD SVMGetMemSize()
{
  return memSize;
}

SVM_WORD SVMGetDiskCount()
{
  return nDisks;
}

SVM_PCSTR SVMGetDiskFileName(SVM_WORD nDisk)
{
  if(nDisk >= nDisks)
    return NULL;
  return diskFileNames[nDisk];
}
SVM_PCSTR SVMGetLogFileName()
{
  return logFileName;
}

SVM_WORD SVMInitOpts(int argc, char **argv)
{
  int i, j;
  wchar_t c;
  SVM_WORD ms = 0;
  if(argc < 2)
  {
    fprintf(stderr, usageMsg, argv[0]);
    return 0;
  }
  i = 1;
start:
  if(i >= argc)
    goto end;
  if(0 == strcmp(argv[i], "-l"))
  {
    ++i;
    goto logFileName;
  }
  else if(0 == strcmp(argv[i], "-m"))
  {
    ++i;
    goto memSize;
  }
  else
  {
    if(nDisks < SVM_MAX_DISKS)
    {
      diskFileNames[nDisks] = argv[i];
      ++nDisks;
      ++i;
      goto start;
    }
    else
    {
      fprintf(stderr, "SVM support %d disks at max\n", SVM_MAX_DISKS);
      return 0;
    }
  }
logFileName:
  if(i >= argc)
  {
    fprintf(stderr, usageMsg, argv[0]);
    return 0;
  }
  else
  {
    logFileName = argv[i];
    ++i;
    goto start;
  }
memSize:
  if(i >= argc)
  {
    fprintf(stderr, usageMsg, argv[0]);
  }
  else
  {
    for(j = 0; ( c = (argv[i])[j] ); ++j)
    {
      if(c >= '0' && c<='9')
        ms = ms * 10 + (c - '0');
      else
      {
        fprintf(stderr, usageMsg, argv[0]);
        return 0;
      }
    }
    if(ms > 4095)
    {
        fprintf(stderr,
                "Specified memory size(%d MB) exceeds address "
                "space size\n",
                ms);
        return 0;
    }
    memSize = ms;
    ++i;
    goto start;
  }
end:
  if(nDisks < 1)
  {
    fprintf(stderr, usageMsg, argv[0]);
  }
  memSize *= 1024 * 1024;
  return 1;
}
