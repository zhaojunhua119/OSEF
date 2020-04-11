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

int strUser2Kernel(char* src, char* dst, int size) {
	int virtAddr = (int)src;
	int data;
	for (int i = 0; i < size; i++) {
		if (!kernel->machine->ReadMem(virtAddr, sizeof(char), &data))//try twice
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
                kernel->currentThread->Finish();
                return;
      default:
	cerr << "Unexpected system call " << type << "\n";
	break;
      }
      break;
    default:
      cerr << "Unexpected user mode exception" << (int)which << "\n";
      break;
    }
    ASSERTNOTREACHED();
}
