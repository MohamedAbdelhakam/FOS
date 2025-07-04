#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

//NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
////////////////////////////////////////////////////////////////////
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

void init_KHeap()
{
    LIST_INIT(&free_list);

    free_block_array[0].va =KERNEL_HEAP_START;
    free_block_array[0].size=KERNEL_HEAP_MAX - KERNEL_HEAP_START;
    LIST_INSERT_HEAD(&free_list, &free_block_array[0]);
}
bool f_init=0;

struct alloc_block {
    uint32 va;
    uint32 size;
};

struct alloc_block alloc_list[P_Size];
int alloc_inx=0;

void add_alloc_block(uint32 va, uint32 size) {
    alloc_list[alloc_inx].va=va;
    alloc_list[alloc_inx].size=size;
    alloc_inx++;
}



uint32 alloc_size(uint32 va){
    for(int i=0; i<alloc_inx; i++){
        if(alloc_list[i].va==va){
            uint32 sz = alloc_list[i].size;
            alloc_list[i].va =0;
            alloc_list[i].size=0;
            return sz;
        }
    }
    return 0;
}

uint32 alloc_size_2(uint32 va)
{
	for(int i=0; i<alloc_inx;i++){

		if(alloc_list[i].va==va){
			uint32 sz=alloc_list[i].size;
			return sz;
		}
	}
	return 0;
}
uint32 modify_alloc_size(uint32 va,uint32 new_size)
{
	for(int i=0; i<alloc_inx; i++){
	        if(alloc_list[i].va == va){
	            alloc_list[i].size=new_size;
	            return new_size;
	        }
	    }
	    return 0;
}
//********************************************************************

void* kmalloc_bf(unsigned int size)
{
	if(f_init==0)
	{
		init_KHeap();
		f_init=1;
	}
	size=ROUNDUP(size,PAGE_SIZE);

	if(LIST_EMPTY(&free_list)){
		return NULL;
	}

	struct free_block *Element;
	struct free_block *Best_=NULL;

	uint32 Curr_Fragment=0;
	uint32 Min_Fragment=0xFFFFFFFF;
	LIST_FOREACH(Element,&free_list)
	{
		if(Element->size>=size)
		{
			Curr_Fragment=(uint32)Element->size-size;

			if(Curr_Fragment<Min_Fragment)
			{
				Min_Fragment=Curr_Fragment;
				Best_=Element;

				if(Curr_Fragment== 0)
				{
				   break;
				}
			}
		}
	}

	int r0=0;
	if(Best_==NULL)
	{
		return NULL;
	}
	uint32 MAX=(Best_->va)+size;
	for(uint32 va=Best_->va;va<MAX;va+=PAGE_SIZE)
	{
		struct Frame_Info *ptr_FI;
		r0=allocate_frame(&ptr_FI);

		r0=map_frame(ptr_page_directory,ptr_FI,(uint32 *)va,(PERM_PRESENT|PERM_WRITEABLE));
		ptr_FI->va=va;
	}

	if(Min_Fragment>0){
		struct free_block *remain=new_free_block();
		uint32 remain_VA =Best_->va+size;
		remain->va =remain_VA;
		remain->size =Best_->size-size;

		LIST_INSERT_AFTER(&free_list,Best_,remain);
	}
	uint32 val =Best_->va;
	LIST_REMOVE(&free_list,Best_);
	add_alloc_block(val,size);
	return (void*)val;

}
void* kmalloc_ff(unsigned int size)
{
	if(f_init==0)
	{
		init_KHeap();
		f_init=1;
	}
	size=ROUNDUP(size,PAGE_SIZE);

	if(LIST_EMPTY(&free_list)){

		return NULL;
	}

	struct free_block *Element;
	struct free_block *Best_=NULL;

	uint32 currFragmentation=0;
	LIST_FOREACH(Element,&free_list)
	{
		if(Element->size>=size)
		{
			currFragmentation=(uint32)Element->size-size;
			Best_=Element;
			break;
		}
	}
	int r1=0;
	if(Best_==NULL)
	{
		return NULL;
	}
	uint32 MAX=(Best_->va)+size;
	for(uint32 va=Best_->va;va<MAX;va+=PAGE_SIZE)
	{
		struct Frame_Info *ptr_FI;
		r1=allocate_frame(&ptr_FI);

		r1=map_frame(ptr_page_directory,ptr_FI,(uint32 *)va,(PERM_PRESENT|PERM_WRITEABLE));
		ptr_FI->va=va;
	}
	if(currFragmentation>0){
		struct free_block *remain=new_free_block();
		uint32 remain_VA = Best_->va+size;
		remain->va =remain_VA;
		remain->size =Best_->size-size;

		LIST_INSERT_AFTER(&free_list,Best_,remain);
	}
	uint32 val=Best_->va;
	LIST_REMOVE(&free_list,Best_);
	add_alloc_block(val,size);
	return (void*)val;
}
void* kmalloc(unsigned int size)
{
	if(isKHeapPlacementStrategyBESTFIT())
	{
		return kmalloc_bf(size);
	}


	if(isKHeapPlacementStrategyFIRSTFIT())
	{
		return kmalloc_ff(size);
	}
	return NULL;
}

void kfree(void* virtual_address)
{
	uint32 va=(uint32)virtual_address;
	uint32 size=alloc_size(va);
	if(size==0){
		return;
	}

	for(uint32 Addr=va; Addr<(va+size); Addr+=PAGE_SIZE){
		unmap_frame(ptr_page_directory,(void*)Addr);
	}

	struct free_block *block=new_free_block();
	if(block==NULL){
		return;
	}

	block->va=va;
	block->size=size;

	struct free_block *it;
	bool insrt=0;

	LIST_FOREACH(it,&free_list) {
		if(va<it->va){
			LIST_INSERT_BEFORE(&free_list,it,block);
			insrt=1;
			break;
		}
	}

	if(!insrt){
		LIST_INSERT_TAIL(&free_list,block);
	}

	struct free_block *current,*next;
	LIST_FOREACH(current,&free_list){
		next=LIST_NEXT(current);
		if(next&&(current->va+current->size)==next->va){
			current->size+=next->size;
			LIST_REMOVE(&free_list,next);
		}
	}
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	struct Frame_Info *ptr_FI=to_frame_info(physical_address);
	uint32 Offset=physical_address&0x00000FFF;

	if((ptr_FI!=NULL) && (ptr_FI->references==1))
	{
		return (unsigned int)((ptr_FI->va)+Offset);
	}
	return 0;
	//cprintf("** physical_address: %d\n",physical_address);
	//cprintf("** Offset: %d\n",Offset);
}

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	uint32 *P_T=NULL;
	int r2=get_page_table(ptr_page_directory,(const void*)virtual_address,&P_T);

	if((r2==TABLE_IN_MEMORY) &&(P_T!=NULL)){
		uint32 PT_E=P_T[PTX(virtual_address)];

		if(PT_E & PERM_PRESENT){
			return (PT_E&0xFFFFF000)|(virtual_address&0xFFF);
		}
	}
	return 0;
	//cprintf("-- virtual_address: %d\n",virtual_address);
	//cprintf("-- PT_E: %d\n",PT_E);
}


//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().

void *krealloc(void *virtual_address, uint32 new_size)
{
	if(virtual_address==NULL)
	{
		return kmalloc(new_size);
	}

	if(new_size==0){
		kfree(virtual_address);
		return NULL;
	}
	uint32 va=(uint32)virtual_address;
	uint32 oldd_size=alloc_size_2(va);

	if(oldd_size==0){
		return NULL;
	}
	new_size=ROUNDUP(new_size,PAGE_SIZE);

	if(new_size<=oldd_size)
	{
		return virtual_address;
	}
	struct free_block *it2;
	bool f_realloc=0;
	LIST_FOREACH(it2,&free_list){
		if((va+oldd_size)==it2->va){

			if(it2->size>=(new_size-oldd_size))
			{
				uint32 Size_needed=new_size-oldd_size;
				uint32 MAX =va+new_size;
				for(uint32 Addr = va+oldd_size; Addr<MAX; Addr+=PAGE_SIZE)
				{
					struct Frame_Info *ptr_FI1;
					allocate_frame(&ptr_FI1);
					map_frame(ptr_page_directory,ptr_FI1,(uint32 *)Addr,(PERM_PRESENT|PERM_WRITEABLE));
				}

				if(it2->size ==Size_needed){
					LIST_REMOVE(&free_list,it2);
				}else{
					it2->va +=Size_needed;
					it2->size -=Size_needed;
				}
				modify_alloc_size(va,new_size);
				f_realloc=1;
				break;
			}
		}
	}
	if(f_realloc)
	{
		return (uint32*) va;
	}
	void* new_block_=kmalloc(new_size);
	if(new_block_==NULL) {
		return NULL;
	}

	uint8* old_block_data=(uint8*)virtual_address;
	uint8* new_block_data=(uint8*)new_block_;
	for(uint32 i = 0; i<oldd_size; i++) {
		new_block_data[i]=old_block_data[i];
	}
	kfree(virtual_address);
	return new_block_;
}
