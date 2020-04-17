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
#include <stdlib.h>
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
forkFunc(void){
	kernel->currentThread->RestoreUserState();
	kernel->currentThread->space->RestoreState();
	// cout << "Ln57 exeception.cc" << endl;
	kernel->machine->Run();
	// cout << "Ln58 exeception.cc" << endl;
}

void
execFunc(Thread* newThread){
	newThread->space->Execute();
}

void
ExceptionHandler(ExceptionType which)
{
    int type = kernel->machine->ReadRegister(2);

    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");

    switch (which) 
	{
		case SyscallException:
		{
			switch(type) 
			{
				case SC_Halt:
				{
					DEBUG(dbgSys, "Shutdown, initiated by user program.\n");

					SysHalt(); 

					return;
					ASSERTNOTREACHED();				
				}break;

				case SC_Fork:
				{
					DEBUG(dbgSys, "Fork address" << kernel->machine->ReadRegister(4) << "\n");
					int funAddr = kernel->machine->ReadRegister(4);
					// for setting
					Thread *t = new Thread("Syscall Fork");
					//t->swap = kernel->currentThread->swaplocation;
					//t->space = new AddrSpace(kernel->currentThread->space);	
					t->space = new AddrSpace();
					t->space = kernel->currentThread->space;		
					t->SaveUserState();
					// cout << "PCReg: " << PCReg <<"; funAddr: " << funAddr << endl;					
					t->setUserRegister(PCReg, funAddr);
					t->setUserRegister(NextPCReg, funAddr + 4);
					
					// cout << "Ln103 in excetion.cc" << endl;
					t->Fork((VoidFunctionPtr) forkFunc, (void*) 0);

					/* Modify return point */
					{
					/* set previous programm counter (debugging only)*/
					kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

					/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
					kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
					
					/* set next programm counter for brach execution */
					kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
					}
					// SysHalt(); 
					return;
					ASSERTNOTREACHED();	
				}break;
			
				case SC_Exec:
				{
					// cout<< "Executing SC_Exec" << endl;
					//char* prog = (char*)kernel->machine->ReadRegister(4);
					int vaddr, memval, i =0;
					char filename[100];
					vaddr = kernel->machine->ReadRegister(4);
					kernel->machine->ReadMem(vaddr, 1, &memval);
					while((*(char*)&memval) != '\0'){
						filename[i] = (char) memval;
						++i;
						vaddr++;
						kernel->machine->ReadMem(vaddr, 1, &memval);
					}
					filename[i]  = (char)memval;
					cout << "Executing " <<  filename << endl;


					Thread *t = new Thread("New Exec Thread");
					t->space = new AddrSpace();
					
					t->space->Load(filename);
					t->Fork((VoidFunctionPtr) execFunc, t);

					/* Modify return point */
					{
					/* set previous programm counter (debugging only)*/
					kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

					/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
					kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
					
					/* set next programm counter for brach execution */
					kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
					}
					// SysHalt(); 
					return;
					ASSERTNOTREACHED();	
				}break;

				case SC_Exit:
				{
					cout << "Executing SC_Exit." << endl;
					//1. first part: free TLB
					TranslationEntry* pageEntry = kernel->currentThread->space->getPageTable();
					int num = kernel->currentThread->space->getNumPages();
					// cout << "Gonna clear" << num << " pages" << endl;
					for(int i = 0; i < num; i++){
						int n = pageEntry[i].physicalPage;
						if(pageEntry[i].valid == true && pageEntry[i].physicalPage != -1){
							cout << "Deallocate physical page number:" << n << endl;
							kernel->freeMap->Clear(pageEntry[i].physicalPage);
							pageEntry[i].physicalPage = -1; 
							pageEntry[i].valid = false; 
						}				
							
					}
					

					//2. second part: finsh thread();
					kernel->currentThread->Finish();

					/* Modify return point */
					{
					/* set previous programm counter (debugging only)*/
					kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

					/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
					kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
					
					/* set next programm counter for brach execution */
					kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
					}
					// SysHalt(); 

					return;
					ASSERTNOTREACHED();	
				}break;

				case SC_ConsoleRead:
				{
					DEBUG(dbgSys, "ConsoleRead " << kernel->machine->ReadRegister(4) << "\n");
					
					/* Process SysAdd Systemcall*/
					// 5 is the size of buffer
					int size = kernel->machine->ReadRegister(5);
					// 4 get the first address of buffer
					int addr = kernel->machine->ReadRegister(4);

					//define in ksyscall.h
					int res = SysRead(addr, size);

					// void WriteRegister(int num, int value);
					kernel->machine->WriteRegister(2, res);

					cout << "SC_ConsoleRead Done" << endl;

					/* Modify return point */
					{
					/* set previous programm counter (debugging only)*/
					kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

					/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
					kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
					
					/* set next programm counter for brach execution */
					kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
					}
					// SysHalt(); 

					return;
					ASSERTNOTREACHED();	
				}break;
		
				case SC_ConsoleWrite:
				{
					DEBUG(dbgSys, "ConsoleWrite " << kernel->machine->ReadRegister(4) << "\n");
					
					/* Process SysAdd Systemcall*/
					// 5 is the size of buffer
					int size = kernel->machine->ReadRegister(5);
					// 4 get the first address of buffer
					int addr = kernel->machine->ReadRegister(4);					

					//define in ksyscall.h
					int res = SysWrite(addr, size);

					// void WriteRegister(int num, int value);
					kernel->machine->WriteRegister(2, res);

					//cout << "SC_ConsoleWrite Done" << endl;

					/* Modify return point */
					{
					/* set previous programm counter (debugging only)*/
					kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

					/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
					kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
					
					/* set next programm counter for brach execution */
					kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
					}
					// SysHalt(); 

					return;
					ASSERTNOTREACHED();	
				}break;

				case SC_Add:
				{
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
				}break;
			

				default:
					cerr << "Unexpected system call " << type << "\n";
				// break;
			}
			
		}break;
			
		
		case PageFaultException:
		{
			RandomInit(100);
			bool randTag;
			int rand_res = RandomNumber() % 2;
			// Fetch virtual address
			int pageFaultAddr = kernel->machine->ReadRegister(BadVAddrReg);
			// Fetch virtual page number from thread pageTable
			int pageFaultNum = pageFaultAddr / PageSize;
			//check the free physical page number from main memory
			int PPN = kernel->freeMap->FindAndSet();
			//Fetch page entry of current thread [by VPN(pageFaultNum)]
			TranslationEntry* pageEntry = &kernel->currentThread->space->getPageTable()[pageFaultNum];
			TranslationEntry* pageTable = kernel->currentThread->space->getPageTable();
			int spaceSize = kernel->currentThread->space->getNumPages();
			int phyPageMem;
			//cout << "Checkpoint PPN:" << PPN << endl;
			if(PPN != -1)
			// if there is space for insert
			{
				cout << "Allocating PPN:" << PPN << endl;
				// cout << "		VPN:" << pageEntry->virtualPage << endl;
				pageEntry->physicalPage = PPN;
				pageEntry->valid = true;
				//read data from swap space into main memory, using ReadAt();
				phyPageMem = PPN * PageSize;
				kernel->swapSpace->ReadAt(&(kernel->machine->mainMemory[phyPageMem]), PageSize, pageEntry->virtualPage * PageSize);
				//================> update container, such as entryList->insert(pageEntry);
				if (rand_res == 0 || kernel->entryList->IsInList(pageEntry) )
				{
					kernel->entryList->Append(pageEntry);
				}
				else
				{
					kernel->entryList->Prepend(pageEntry);
				}
			}
			else 
			// if there is no free memory
			{				
				// Get the victim page from entryList
				TranslationEntry* victim = kernel->entryList->RemoveFront();

				//cout << "Select the victim page, PPN: " << victim->physicalPage << "; VPN: " << victim->virtualPage << endl;
				if(victim->physicalPage != -1 && victim->valid == true){
					int virPageIndex = victim->virtualPage * PageSize;
					int phyPageIndex = victim->physicalPage * PageSize;
					// copy the victim from main memory into the swapspace 
					kernel->swapSpace->WriteAt(&(kernel->machine->mainMemory[phyPageIndex]), PageSize, victim->virtualPage * PageSize);
					cout << "Reallocating PPN:" << victim->physicalPage << endl;

					// update the victime page
					victim->physicalPage = -1;
					victim->valid = false;

					// update the pageEntry
					pageEntry->physicalPage = phyPageIndex / PageSize;
					pageEntry->valid = true;

					// write the pageEntry from swapspace into main memory
					kernel->swapSpace->ReadAt(&(kernel->machine->mainMemory[phyPageIndex]), PageSize, pageEntry->virtualPage * PageSize);
					if (rand_res == 0 || kernel->entryList->IsInList(pageEntry) )
					{
						kernel->entryList->Append(pageEntry);
					}
					else
					{
						kernel->entryList->Prepend(pageEntry);
					}					
				}
				// if(!kernel->entryList->empty())
				// {
					// Randomly choose an evicted physical page from container
					// int num = kernel->entryList->NumInList();
					// // //cout << "entryList size: " << num << endl;
					// // RandomInit(100);
					// // int rand = RandomNumber() % num;

					// TranslationEntry* evicp = kernel->entryList->RemoveFront();
					// cout << "Print evicp:" << endl << "	VirtualPage: " << evicp->virtualPage << endl;
					// cout  << "	physicalPage: " << evicp->physicalPage << endl;
					// cout  << "	valid: " << evicp->valid << endl;
					// // cout  << "	readOnly: " << evicp->readOnly << endl;
					// // cout  << "	use: " << evicp->use << endl;
					// // cout  << "	dirty: " << evicp->dirty << endl;
					// // TranslationEntry *evicp = kernel->entryList->last->item;
					// // kernel->entryList->Remove(ev);
					// // TranslationEntry* evicp = ev;
					// if(kernel->entryList->IsInList(evicp)){
					// 	cout << "Evicting PPN " << evicp->physicalPage << endl;
					// 	 kernel->entryList->Remove(evicp);

					// }
					
					// cout << "Try to Reallocating PPN:" << evicp->physicalPage << endl;
					// if(evicp->physicalPage != -1 && evicp->valid == true) {
					// 	// kernel->entryList.erase(kernel->entryList.begin() + rand);
					// 	// Fetch the physical page number fo the evicted page
					// 	cout << "Reallocating PPN:" << evicp->physicalPage << endl;
					// 	int phyPageNum = evicp->physicalPage;
					// 	int virPageIndex = evicp->virtualPage;
					// 	phyPageMem = phyPageNum * PageSize;
					// 	cout << "Reallocate PPN: " << phyPageNum << endl;
					// 	cout << "VPN: " << pageEntry->virtualPage << endl;
					// 	// copy evicted physical page data from main memory to swap space
					// 	kernel->swapSpace->WriteAt(&(kernel->machine->mainMemory[phyPageMem]), PageSize, pageEntry->virtualPage * PageSize);

					// 	// update the evictedPage entry
					// 	evicp->physicalPage = -1; 
					// 	evicp->valid = false;						

					// 	// update the page entry
					// 	//pageEntry = &pageEntryList[phyPageNum];
					// 	pageEntry->physicalPage = phyPageNum;
					// 	pageEntry->valid = true;

					// 	// read data from swapspace to main memory
					// 	kernel->swapSpace->ReadAt(&(kernel->machine->mainMemory[phyPageNum * PageSize]), PageSize, pageEntry->virtualPage * PageSize);

					// 	// update the contianer
					// 	kernel->entryList->Append(pageEntry);
						
					// }					
				// }				
			}
			//cout << "Finish PageFault Exception" << endl;

			// /* Modify return point */
			// {
			// /* set previous programm counter (debugging only)*/
			// kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));

			// /* set programm counter to next instruction (all Instructions are 4 byte wide)*/
			// kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);

			// /* set next programm counter for brach execution */
			// kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			// }
			return;
			ASSERTNOTREACHED();	
		}break;


		default:
			cerr << "Unexpected user mode exception" << (int)which << "\n";
	// break;
	}
    ASSERTNOTREACHED();
}


