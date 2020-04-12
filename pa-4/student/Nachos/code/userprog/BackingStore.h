// backingstore.h

#ifndef BACKINGSTORE_H
#define BACKINGSTORE_H

#include "filesys.h"
#include "translate.h"
class AddrSpace;
class TranslationEntry;

class BackingStore {
 private:
  OpenFile *swapFile;
  int pid;
  int numPages;
  char *swapFileName;
  bool *valid;
  char test[3200];
 public:
  /* Store file name for an AddrSpace backing store */
  BackingStore(AddrSpace *as, int numPages, int pid);

  ~BackingStore();

  /* Actually create the backing store file (on first evict) */
  void init();

  /* Write the virtual page referenced by PTE to the backing store
     Example invocation: PageOut(&machine->pagetTable[virtualPage] or
     PageOut(&space->pageTable[virtualPage])
  */
  void PageOut(TranslationEntry *pte);

  /* Read the virtual page referenced by PTE from the backing store */
  int PageIn(TranslationEntry *pte);

};


#endif
