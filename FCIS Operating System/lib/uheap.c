#include <inc/lib.h>

// malloc()
//	This function use NEXT FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
#define P_Size 1024
struct free_block
{
    uint32 va;
    uint32 size;
    LIST_ENTRY(free_block) prev_next_info;
};
struct free_block free_block_array[P_Size];

LIST_HEAD(free_block_list, free_block);

struct free_block_list free_list;

struct free_block initial;

int next_free_block_index=1;

struct free_block* new_free_block(){
    if(next_free_block_index>=P_Size){
        return NULL;
    }
    return &free_block_array[next_free_block_index++];
}

struct free_block *next_ptr=NULL;

void init_UHeap()
{
    LIST_INIT(&free_list);

    free_block_array[0].va =USER_HEAP_START;
    free_block_array[0].size=USER_HEAP_MAX -USER_HEAP_START ;
    LIST_INSERT_HEAD(&free_list, &free_block_array[0]);
    next_ptr=LIST_FIRST(&free_list);
}
bool f_init=0;

uint8 U_Arr[(USER_HEAP_MAX-USER_HEAP_START)/PAGE_SIZE]={0};
uint32 G_No_Alloc=0;

void* U_Add_Alloc[P_Size];
uint32 US_Alloc[P_Size];
uint32 U_inx[P_Size];
//----------------------------------------------------------------

void* malloc(uint32 size)
{
	//TODO: [PROJECT 2025 - MS2 - [4] User Heap] malloc() [User Side]
	// Write your code here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");
	if(sys_isUHeapPlacementStrategyNEXTFIT())
	{

	if(f_init==0)
	{
		init_UHeap();
		f_init=1;
	}
	size=ROUNDUP(size,PAGE_SIZE);


	if(LIST_EMPTY(&free_list)){
		return NULL;
	}
	struct free_block *Element;

	uint32 currFragmentation=0;
	bool loop=0;
	struct free_block *temp=next_ptr;
	do
	{
		if(temp->size>=size)
		{
			currFragmentation=(uint32)temp->size-size;
			break;
		}
		temp=temp->prev_next_info.le_next;
		if(temp==NULL)
		{
			loop=1;
			temp=LIST_FIRST(&free_list);
		}
	}while(temp!=next_ptr);
	if(loop&&temp==next_ptr)
	{
		return NULL;
	}
	if(temp!=NULL)
	{
		if(currFragmentation>0){
			struct free_block *remain=new_free_block();
			uint32 remain_VA = temp->va+size;
			remain->va =remain_VA;
			remain->size =temp->size-size;

			LIST_INSERT_AFTER(&free_list,temp,remain);
		}
		next_ptr=temp->prev_next_info.le_next;
		uint32 val=temp->va;
		LIST_REMOVE(&free_list,temp);
		sys_allocateMem(temp->va,size);

		int k;
		for(k=0; k<P_Size;k++)
		{
		    if(U_Add_Alloc[k]==0)
		    {
		    	U_Add_Alloc[k]=(void*)val;
		        US_Alloc[k]=size;
		        U_inx[k]=(val-USER_HEAP_START)/PAGE_SIZE;
		        if(k==G_No_Alloc){
		        	G_No_Alloc++;
		        }
		        break;
		    }
		}

		uint32 no_pages=size/PAGE_SIZE;
		uint32 inx= (val - USER_HEAP_START) / PAGE_SIZE;
		for(int j=inx; j<(inx+no_pages); j++){
			U_Arr[j] = 1;
		}
		return (void*)val;
	}
	return NULL;
	}
	return NULL;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void* virtual_address)
{
	unsigned int S_Free= 0;
	unsigned int found_inx=0;

	if((virtual_address<(void *)(USER_HEAP_START)) || (virtual_address>(void *)USER_HEAP_MAX))
	{
		panic("--- Invalid add\n");
		return;
	}

	for(unsigned int ii=0; ii<P_Size; ii++)
	{
		if((void*)U_Add_Alloc[ii]==virtual_address)
		{
			S_Free= US_Alloc[ii];
			unsigned int number_of_pages=ROUNDUP(S_Free, PAGE_SIZE)/PAGE_SIZE;
			found_inx=1;

			for(int iii=U_inx[ii]; iii<(number_of_pages+U_inx[ii]); iii++) {
				U_Arr[iii] = 0;
			}

			sys_freeMem((uint32)virtual_address,(uint32)S_Free);
			U_Add_Alloc[ii]=0;
			US_Alloc[ii]=0;
			U_inx[ii]= 0;

			struct free_block* free_b1=new_free_block();
			if(free_b1==NULL)
			{
				panic("--- no space\n");
				return;
			}

			free_b1->va =(uint32)virtual_address;
			free_b1->size=S_Free;
			struct free_block* It1=LIST_FIRST(&free_list);
			struct free_block* Pre1=NULL;

			while((It1!=NULL) && (It1->va<free_b1->va))
			{
				Pre1=It1;
				It1= LIST_NEXT(It1);
			}

			if(Pre1==NULL)
			{
				LIST_INSERT_HEAD(&free_list,free_b1);
			}
			else
			{
				LIST_INSERT_AFTER(&free_list,Pre1,free_b1);
			}
			if (Pre1 == NULL) {
			    if (next_ptr == NULL) {
			        next_ptr = LIST_FIRST(&free_list);
			    }
			} else {
			    if (next_ptr == NULL || free_b1->va < next_ptr->va) {
			        next_ptr = free_b1;
			    }
			}
			struct free_block* Next1=LIST_NEXT(free_b1);
			if(Next1!=NULL && ((free_b1->va+free_b1->size)==Next1->va))
			{
				free_b1->size+=Next1->size;
				LIST_REMOVE(&free_list,Next1);
				next_free_block_index--;
			}

			if((Pre1!=NULL) && ((Pre1->va+Pre1->size)==free_b1->va))
			{
				Pre1->size+=free_b1->size;
				LIST_REMOVE(&free_list,free_b1);
				if((next_ptr==free_b1) || (next_ptr==Next1)){
				    next_ptr=LIST_FIRST(&free_list);
				}
				next_free_block_index--;
			}
			return;
		}
	}

	if(found_inx==0)
	{
		panic("--- unalloc\n");
		return;
	}


	//--------------------------------------------------

}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	//TODO: [PROJECT 2025 - BONUS3] User Heap Realloc [User Side]
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");

	return NULL;
}



//==================================================================================//
//============================= OTHER FUNCTIONS ====================================//
//==================================================================================//

void expand(uint32 newSize)
{
}

void shrink(uint32 newSize)
{
}

void freeHeap(void* virtual_address)
{
	return;
}


//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//[] Free Shared Variable [User Side]
	// Write your code here, remove the panic and write your code
	//panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()

}

void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//[[6] Shared Variables: Creation] smalloc() [User Side]
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return 0;
}

void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//[[6] Shared Variables: Get] sget() [User Side]
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	return NULL;
}

