// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
#include "synchconsole.h"
#include "addrspace.h"

int strUser2Kernel(char* src, char* dst, int size) {
	int virtAddr = (int)src;
	int data;
	for (int i = 0; i < size; i++) {
		if (!kernel->machine->ReadMem(virtAddr, sizeof(char), &data))
		  if (!kernel->machine->ReadMem(virtAddr, sizeof(char), &data))
				return -1;
		dst[i] = (char)data;
		virtAddr++;
	}
	dst[size] = '\0';
	return 0;
}


int strKernel2User(char* src, char* dst, int size) {
	int virtAddr = (int)dst;
	for (int i = 0; i < size; i++) {
		if (!kernel->machine->WriteMem(virtAddr, sizeof(char), src[i]))
		  if (!kernel->machine->WriteMem(virtAddr, sizeof(char), src[i]))
				return -1;
		virtAddr++;
	}
	if (!kernel->machine->WriteMem(virtAddr, sizeof(char), '\0'))
	  if (!kernel->machine->WriteMem(virtAddr, sizeof(char), '\0'))
			return -1;
	return 0;
}
void returnSyscall(int arg) {
        DEBUG(dbgSys, "Add returning with " << arg << "\n");
        /* Prepare Result */
        kernel->machine->WriteRegister(2, (int)arg);

        /* Modify return point */
        {
          /* set previous programm counter (debugging only)*/
          kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

          /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
          kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

          /* set next programm counter for brach execution */
          kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
        }
}
void UserThreadStart(int func) {
  //printf("##New user level thread starts\n");
  //kernel->currentThread->space->InitRegisters();
  kernel->currentThread->RestoreUserState();
  kernel->currentThread->space->RestoreState();   // load page table register
  //kernel->machine->WriteRegister(PCReg, func);
  //kernel->machine->WriteRegister(NextPCReg, func+4);

  kernel->machine->Run();     // jump to the user program
  ASSERT(FALSE);
}
void ExecUserThreadStart(int filename) {
  //printf("##New user level thread starts\n");
  kernel->currentThread->space->InitRegisters();
  kernel->currentThread->space->RestoreState();   // load page table register
  //kernel->machine->WriteRegister(PCReg, func);
  //kernel->machine->WriteRegister(NextPCReg, func+4);
  if (kernel->currentThread->space->Load((char*)filename)) {  // load the program into the space
      kernel->currentThread->space->Execute();         // run the program
  }
  ASSERTNOTREACHED();
}

Lock* memoryPagingLock=NULL;

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions
//	is in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) {
    case SyscallException:
      switch(type) {
      case SC_Halt:
	DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

	SysHalt();

	ASSERTNOTREACHED();
	break;

      case SC_Add:
	DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");

	/* Process SysAdd Systemcall*/
	int result;
	result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
			/* int op2 */(int)kernel->machine->ReadRegister(5));

	DEBUG(dbgSys, "Add returning with " << result << "\n");
	/* Prepare Result */
	kernel->machine->WriteRegister(2, (int)result);

	/* Modify return point */
	{
	  /* set previous programm counter (debugging only)*/
	  kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

	  /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
	  kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

	  /* set next programm counter for brach execution */
	  kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
	}

	return;

	ASSERTNOTREACHED();

	break;
        case SC_Read:
        case SC_Write:
        {

        	int size = kernel->machine->ReadRegister(5);
        	if (size <= 0) {
        		printf("Error: Nothing to read or write with size 0, or less than 0\n");
        		//returnSyscall(-1);
        		return;
        	}
        	OpenFileId fileId = (OpenFileId)kernel->machine->ReadRegister(6);
        	if (( fileId == CONSOLEINPUT && type == SC_Read )
        		|| ( fileId == CONSOLEOUTPUT && type == SC_Write )) {

        		char str[size + 1];//a string in kernel

        		if (type == SC_Read) {		//SC_Read

        			char *buffer = (char*)kernel->machine->ReadRegister(4);

                                char c;

                        	// read {size} characters into buffer
                        	for (int i = 0; i < size; ++i) {

                        		c = kernel->synchConsoleIn->GetChar();
                        		str[i] = c;
                        	}


        			if (strKernel2User(str, buffer, size) == -1) {
        				returnSyscall(-1);
        				return;
        			}

        		}
        		else
        		{					//SC_Write
        			if (strUser2Kernel((char*)kernel->machine->ReadRegister(4), str, size) == -1) {
        				returnSyscall(-1);
        				return;
        			}
                                for (int i = 0; i < size; ++i) {
                        		kernel->synchConsoleOut->PutChar(str[i]);
                        	}
        		}
        	}
        	else
        	{
        		returnSyscall(-1);
        		return;
        	}
        	returnSyscall(0);
        	return;
        }
        case SC_Exit:
                //kernel->currentThread->exitStatus = (int)machine->ReadRegister(4);
                returnSyscall(0);
                kernel->currentThread->Finish();
                return;
        case SC_SysFork:
          {
            Thread *parent = kernel->currentThread;

            char childname[256] = {};
            sprintf(childname, "%s-forked-%d", parent->getName(), kernel->ThreadId + 1);
            Thread *child = new Thread(strdup(childname));
            child->space=new AddrSpace(*parent->space, child->pid);
            kernel->machine->WriteRegister(2,child->pid);

           /* set previous programm counter (debugging only)*/
           kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

           /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
           kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

           /* set next programm counter for brach execution */
           kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);

            parent->SaveUserState();
            child->SaveUserState();

            kernel->machine->WriteRegister(2,0);
            void *arg = (void*)kernel->machine->ReadRegister(PCReg);
            child->Fork((VoidFunctionPtr)UserThreadStart,arg);

            return;
          }
        case SC_Exec:
          {
            char filename[100];
            int i=0;

            int vaddr = kernel->machine->ReadRegister(4);
            int memval = 0;
            kernel->machine->ReadMem(vaddr, 1, &memval);
            while ((*(char*)&memval) != '\0') {
                filename[i]  = (char)memval;
                ++i;
                vaddr++;
                kernel->machine->ReadMem(vaddr, 1, &memval);
            }
            filename[i]  = (char)memval;
            Thread *child = new Thread(filename);
            AddrSpace *space = new AddrSpace;
            child->space = space;
            ASSERT(space != (AddrSpace *)NULL);

            //return pid as the space id
            kernel->machine->WriteRegister(2,child->pid);

            /* set previous programm counter (debugging only)*/
            kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

            /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
            kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

            /* set next programm counter for brach execution */
            kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
            Thread *parent = kernel->currentThread;
             parent->SaveUserState();
             child->SaveUserState();
             void *arg = (void*)filename;
             child->Fork((VoidFunctionPtr)ExecUserThreadStart,arg);

            return;
          }
      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      }
      break;
    case PageFaultException: // No valid translation found
    {
      if (memoryPagingLock == NULL)
        memoryPagingLock = new Lock("memoryPagingLock");

      int vpn = kernel->machine->ReadRegister(BadVAddrReg) / PageSize;

      memoryPagingLock->Acquire();
      kernel->currentThread->space->pageFault(vpn);
      memoryPagingLock->Release();
      return;
    }
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
