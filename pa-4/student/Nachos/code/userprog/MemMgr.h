#ifndef MEMMGR_H
#define MEMMGR_H

#include "copyright.h"
#include "bitmap.h"

class Lock;
class AddrSpace;
class MemMgr {

public:
  MemMgr(int numPages);
  ~MemMgr();

  int AllocPage(AddrSpace* space, int vpn);
  void FreePage(int physPageNum);
  bool PageIsAllocated(int physPageNum);
  void Print() { memMap->Print(); }

private:
  int numPhysPages;//now just a copy of NumPhysPages
  Bitmap* memMap;
  Lock* lock;

  //used by demand paging
  AddrSpace** spaceTable;
  int* vpnTable;
  int victimPage();
};

#endif
