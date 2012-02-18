#include <stdio.h>
#include <string.h>
#include "SVMDisk.h"

typedef struct
{
  FILE* h;
  SVM_WORD size;
} SVMDisk;

static SVMDisk disks[SVM_MAX_DISKS];

SVM_WORD SVMReadSector(SVM_WORD nDisk, SVM_WORD *buffer)
{
  size_t oRead;
  if(!disks[nDisk].h)
    return 0;
  oRead = fread(buffer, 1, SVM_SECTOR_SIZE, disks[nDisk].h);
  if(oRead > 0)
  {
    memset(buffer, 0, SVM_SECTOR_SIZE - oRead);
    return 1;
  }
  return 0;
}

SVM_WORD SVMWriteSector(SVM_WORD nDisk, SVM_WORD *buffer)
{
  size_t oWrite;
  SVM_INT sector;
  if(!disks[nDisk].h)
    return 0;
  sector = SVMGetDiskSector(nDisk);
  if(sector < (SVM_INT)(disks[nDisk].size >> 9))
  {
    oWrite = fwrite(buffer, 1, SVM_SECTOR_SIZE, disks[nDisk].h);
    return oWrite > 0;
  }
  return 0;
}

SVM_WORD SVMSetDiskSector(SVM_WORD nDisk, SVM_INT sector)
{
  fpos_t pos;
  if(!disks[nDisk].h)
    return 0;
  pos = sector << 9;
  fsetpos(disks[nDisk].h, &pos);
  return 1;
}

SVM_INT SVMGetDiskSector(SVM_WORD nDisk)
{
  fpos_t pos;
  if(!disks[nDisk].h)
    return -1;
  fgetpos(disks[nDisk].h, &pos);
  return (SVM_INT)(pos >> 9);
}

SVM_WORD SVMInitDisks()
{
  SVM_WORD nDisks = SVMGetDiskCount(), i;
  for(i = 0; i<nDisks; ++i)
  {
    disks[i].h = fopen(SVMGetDiskFileName(i), "r+b");
    if(!disks[i].h)
    {
      fprintf(stderr, "Unable to open file: %s", SVMGetDiskFileName(i));
      return 0;
    }
    fseek(disks[i].h, 0, SEEK_END);
    disks[i].size = (SVM_WORD)ftell(disks[i].h);
    fseek(disks[i].h, 0, SEEK_SET);
  }
  return 1;
}

SVM_WORD SVMCloseDisks()
{
  SVM_WORD i;
  int rv = 1;
  for(i = 0; i<SVM_MAX_DISKS;++i)
  {
    if(disks[i].h)
    {
      rv = rv & fclose(disks[i].h);
    }
  }
  return rv;
}