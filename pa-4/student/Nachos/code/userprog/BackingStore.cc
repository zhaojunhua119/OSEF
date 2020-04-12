#include "BackingStore.h"
#include "kernel.h"
#include "main.h"
#include "stats.h"
/**
 * backing store for the virtual address. swap to disk file
 */
BackingStore::BackingStore(AddrSpace *as, int nPages, int pid) {
  // set file name to PID
  swapFileName = new char[10];
  sprintf(swapFileName, "swap%d", pid);
  //printf("+++++++++++%s++++++++++++++",str);

  swapFile = NULL;

  this->numPages = nPages;
  valid = new bool[nPages];
  for (int i=0;i<nPages;i++){
      valid[i]=FALSE;
  }
  this->pid = pid;
}

/* Actually create the backing store file (on first evict) */
void
BackingStore::init() {
  kernel->fileSystem->Create(swapFileName);
  swapFile = kernel->fileSystem->Open(swapFileName);
}

BackingStore::~BackingStore() {
  // delete file
  kernel->fileSystem->Remove(swapFileName);
  // delete file pointer
  delete swapFile;
  delete[] swapFileName;
  delete[] valid;
}
/* Write the virtual page referenced by PTE to the backing store
   Example invocation: PageOut(&machine->pagetTable[virtualPage] or
   PageOut(&space->pageTable[virtualPage])
*/
void BackingStore::PageOut(TranslationEntry *pte) {

  if (swapFile==NULL){
    init();
  }

  // virtual page n in the address space will be stored at n * PageSize in the file
  int offset = pte->virtualPage * PageSize;
  int physAddr = pte->physicalPage * PageSize;


  swapFile->WriteAt(&kernel->machine->mainMemory[physAddr], PageSize, offset);
  printf("S[pid: %d]: [physicalPage: %d] ! [virtualPage: %d]\n", pid, pte->physicalPage, pte->virtualPage);
  valid[pte->virtualPage]=TRUE;
}

/* Read the virtual page referenced by PTE from the backing store */
int BackingStore::PageIn(TranslationEntry *pte) {
  if (valid[pte->virtualPage]){
    int offset = pte->virtualPage * PageSize;
    int physAddr = pte->physicalPage * PageSize;

    swapFile->ReadAt(&kernel->machine->mainMemory[physAddr], PageSize, offset);

    printf("L[pid: %d]: [virtualPage: %d] ! [physicalPage: %d]\n", pid, pte->virtualPage, pte->physicalPage);
    return 0;
  }else{
    return -1;
  }
}
