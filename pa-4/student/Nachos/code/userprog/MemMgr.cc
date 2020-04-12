#include "copyright.h"
#include "MemMgr.h"
#include "kernel.h"
#include "sysdep.h"
#include "synch.h"

/* Create a manager to track the allocation of numPages of physical memory.
You will create one by calling the constructor with NumPhysPages as
the parameter.  All physical pages start as free, unallocated pages. */
MemMgr::MemMgr(int numPages)
{
  numPhysPages = numPages;
  memMap = new Bitmap(numPages);
  lock = new Lock("mapLock");

  //for demand paging
  spaceTable = new AddrSpace*[numPages];
  vpnTable = new int[numPages];
  for(int i=0;i<numPages;i++){
    spaceTable[i] = NULL;
    vpnTable[i] = -1;
  }
  //debug
  //printf("##NumPhysPages=%d\n",numPages);
}

MemMgr::~MemMgr()
{
  delete memMap;
  delete lock;

  delete[] spaceTable;
  delete[] vpnTable;

}

/* Allocate a free page, returning its physical page number or -1
if there are no free pages available. */
int MemMgr::AllocPage(AddrSpace* space, int vpn) {
  lock->Acquire();

  int physNum = memMap->FindAndSet();
  if (physNum ==-1){

    physNum = victimPage();
    //ask victim addrspace to evict its page
    //assuming no error
    spaceTable[physNum]->evictPage(vpnTable[physNum]);


//    printf("phys %d, (PID %d vpn %d)\t->\t(PID %d vpn %d) \n",physNum,
//         spaceTable[physNum]->debugPid,vpnTable[physNum],
//         space->debugPid, vpn);

  }

  spaceTable[physNum] = space;
  vpnTable[physNum] = vpn;
  kernel->machine->lru[physNum] = 0;
  DEBUG(dbgDisk, "phys " << physNum << " -> vpn " << vpn << "(pid:" << space->pid << ")");
  lock->Release();
  return physNum;
}

/* Free the physical page and make it available for future allocation. */
void MemMgr::FreePage(int physPageNum){
  lock->Acquire();
  memMap->Clear(physPageNum);
  spaceTable[physPageNum] = NULL;
  vpnTable[physPageNum] = -1;
  lock->Release();
}

/* True if the physical page is allocated, false otherwise. */
bool MemMgr::PageIsAllocated(int physPageNum) {
  lock->Acquire();
  bool physAllocated = memMap->Test(physPageNum);
  lock->Release();
  return physAllocated;
}

//return the physical page number of a victim page to be evict
int MemMgr::victimPage() {
  //assuming lock is already acquired
  int ppn = 0;
  int i;
  for (i = 1; i < numPhysPages; i++) {
    if (kernel->machine->lru[i] > kernel->machine->lru[ppn])
      ppn = i;
  }

  //printf("Evicting %d\n",ppn);
  return ppn;
}
