#include <inc/assert.h>

#include <kern/sched.h>
#include <kern/user_environment.h>
#include <kern/memory_manager.h>
#include <kern/command_prompt.h>
#include <kern/trap.h>
#include <kern/kheap.h>
#include <kern/utilities.h>

//void on_clock_update_WS_time_stamps();
extern uint32 isBufferingEnabled();
extern void cleanup_buffers(struct Env* e);
extern inline uint32 pd_is_table_used(struct Env *e, uint32 virtual_address);
extern inline void pd_set_table_unused(struct Env *e, uint32 virtual_address);
extern inline void pd_clear_page_dir_entry(struct Env *e, uint32 virtual_address);
//================

void sched_delete_ready_queues() ;
uint32 isSchedMethodRR(){if(scheduler_method == SCH_RR) return 1; return 0;}
uint32 isSchedMethodMLFQ(){if(scheduler_method == SCH_MLFQ) return 1; return 0;}

//==================================================================================//
//============================== HELPER FUNCTIONS ==================================//
//==================================================================================//
void init_queue(struct Env_Queue* queue)
{
	if(queue != NULL)
	{
		LIST_INIT(queue);
	}
}

int queue_size(struct Env_Queue* queue)
{
	if(queue != NULL)
	{
		return LIST_SIZE(queue);
	}
	else
	{
		return 0;
	}
}

void enqueue(struct Env_Queue* queue, struct Env* env)
{
	if(env != NULL)
	{
		LIST_INSERT_HEAD(queue, env);
	}
}

struct Env* dequeue(struct Env_Queue* queue)
{
	struct Env* envItem = LIST_LAST(queue);
	if (envItem != NULL)
	{
		LIST_REMOVE(queue, envItem);
	}
	return envItem;
}

void remove_from_queue(struct Env_Queue* queue, struct Env* e)
{
	if (e != NULL)
	{
		LIST_REMOVE(queue, e);
	}
}

struct Env* find_env_in_queue(struct Env_Queue* queue, uint32 envID)
{
	struct Env * ptr_env=NULL;
	LIST_FOREACH(ptr_env, queue)
	{
		if(ptr_env->env_id == envID)
		{
			return ptr_env;
		}
	}
	return NULL;
}
//==================================================================================//

//==================================================================================//
//============================= REQUIRED FUNCTIONS =================================//
//==================================================================================//

void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================

	//TODO: [PROJECT 2025 - MS2 - [3] CPU Scheduling MLFQ] Initialize MLFQ
	// Write your code here, remove the panic and write your code
	//panic("sched_init_MLFQ() is not implemented yet...!!");

	//[1] Create the ready queues and initialize them using init_queue()

	//[2] Create the "quantums" array and initialize it by the given quantums in "quantumOfEachLevel[]"

	//[3] Set the CPU quantum by the first level one


    //[1] Create the ready queues and initialize them using init_queue()

   // cprintf("MLFQ initialization Call\n", numOfLevels);

    num_of_ready_queues = numOfLevels;
    env_ready_queues = kmalloc(sizeof(struct Env_Queue) * numOfLevels);
    if (env_ready_queues == NULL) {
        panic("sched_init_MLFQ: memory allocation failed for ready queues");
    }

    quantums = kmalloc(sizeof(uint8) * numOfLevels);
    if (quantums == NULL) {
        panic("memory allocation failed for quantums");
    }

    for (int i = 0; i < numOfLevels; i++) {
        init_queue(&env_ready_queues[i]);
        quantums[i] = quantumOfEachLevel[i];
    }

    kclock_set_quantum(quantums[0]);

    scheduler_status = SCH_STARTED;

	    }

struct Env* fos_scheduler_MLFQ()
{
	//TODO: [PROJECT 2025 - MS2 - [3] CPU Scheduling MLFQ] MLFQ Scheduler
	// Write your code here, remove the panic and write your code
	//panic("fos_scheduler_MLFQ() is not implemented yet...!!");

	//Apply the MLFQ with the specified levels to pick up the next environment
	//Note: the "curenv" (if exist) should be placed in its correct queue

	//Steps:
	//======
	//[1] If the current environment (curenv) exists, place it in the suitable queue

	//[2] Search for the next env in the queues according to their priorities (first is highest)

	//[3] If next env is found: Set the CPU quantum by the quantum of the selected level
	 // [1] Handle current environment if it exists



	//cprintf("scheduler called\n");

	    // [1] Handle current environment if it exists
	    if (curenv != NULL && (curenv->env_status == ENV_READY || curenv->env_status == ENV_RUNNABLE))
	    {

	        int current_level = 0;
	        bool found = 0;

	        if (env_ready_queues != NULL && num_of_ready_queues > 0)
	        {
	            for (int i = 0; i < num_of_ready_queues; i++)
	            {
	                struct Env* found_env = find_env_in_queue(&env_ready_queues[i], curenv->env_id);
	                if (found_env != NULL)
	                {
	                    remove_from_queue(&env_ready_queues[i], curenv);
	                    current_level = i;
	                    found = 1;
	                    break;
	                }
	            }
	        }

	        if (!found) {
	           // cprintf(" debug current env not found in any queue\n");
	        }

	        if (!found)
	        {
	            if (curenv->env_runs > 0)
	            {
	                for (int i = 0; i < num_of_ready_queues - 1; i++)
	                {
	                    if (curenv->env_runs >= quantums[i])
	                    {
	                        current_level = i + 1;
	                       // cprintf(" demoting \n", current_level);
	                    }
	                    else
	                    {
	                        break;
	                    }
	                }
	            }
	        }
	        else
	        {
	            if (curenv->env_runs >= quantums[current_level])
	            {
	                if (current_level < num_of_ready_queues - 1) {
	                    current_level++;
	                }
	            }
	        }
	        curenv->env_runs = 0;

	        if (current_level >= num_of_ready_queues) {
	                  (  current_level, num_of_ready_queues - 1);
	            current_level = num_of_ready_queues - 1;
	        }

	        enqueue(&env_ready_queues[current_level], curenv);
	    }
	    else {
	      // cprintf(" no current env or not runnable\n");
	    }
	    struct Env* next_env = NULL;
	    int selected_level = -1;

	    if (env_ready_queues != NULL && num_of_ready_queues > 0)
	    {
	        for (int i = 0; i < num_of_ready_queues; i++)
	        {
	            int size = queue_size(&env_ready_queues[i]);

	            if (size > 0)
	            {
	                next_env = dequeue(&env_ready_queues[i]);
	                selected_level = i;
	                break;
	            }
	        }
	    }

	    if (next_env != NULL && selected_level >= 0 && selected_level < num_of_ready_queues)
	    {
	        kclock_set_quantum(quantums[selected_level]);
	        next_env->env_runs = 0;
	        return next_env;
	    }

	    return NULL;
}


//==================================================================================//
//==================================================================================//
//==================================================================================//



void
fos_scheduler(void)
{

	chk1();
	scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	if (scheduler_method == SCH_RR)
	{
		// Implement simple round-robin scheduling.
		// Pick next environment from the ready queue,
		// and switch to such environment if found.
		// It's OK to choose the previously running env if no other env
		// is runnable.

		//If the curenv is still exist, then insert it again in the ready queue
		if (curenv != NULL)
		{
			enqueue(&(env_ready_queues[0]), curenv);
		}

		//Pick the next environment from the ready queue
		next_env = dequeue(&(env_ready_queues[0]));

		//Reset the quantum
		//2017: Reset the value of CNT0 for the next clock interval
		kclock_set_quantum(quantums[0]);
		//uint16 cnt0 = kclock_read_cnt0_latch() ;
		//cprintf("CLOCK INTERRUPT AFTER RESET: Counter0 Value = %d\n", cnt0 );

	}
	else if (scheduler_method == SCH_MLFQ)
	{
		next_env = fos_scheduler_MLFQ();
	}


	//temporarily set the curenv by the next env JUST for checking the scheduler
	//Then: reset it again
	struct Env* old_curenv = curenv;
	curenv = next_env ;
	chk2(next_env);
	curenv = old_curenv;

	//cprintf("Scheduler select program '%s'\n", next_env->prog_name);
	if(next_env != NULL)
	{
		env_run(next_env);
	}
	else
	{
		/*2015*///No more envs... curenv doesn't exist any more! return back to command prompt
		curenv = NULL;
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);

		//cprintf("SP = %x\n", read_esp());

		scheduler_status = SCH_STOPPED;
		//cprintf("[sched] no envs - nothing more to do!\n");
		while (1)
			run_command_prompt(NULL);

	}
}

void sched_init_RR(uint8 quantum)
{
	sched_delete_ready_queues();
	scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;

	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
	env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
	quantums[0] = quantum;
	kclock_set_quantum(quantums[0]);
	init_queue(&(env_ready_queues[0]));
}



void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(CLOCK_INTERVAL_IN_MS);

	init_queue(&env_new_queue);
	init_queue(&env_exit_queue);
}

void sched_delete_ready_queues()
{
	if (env_ready_queues != NULL)
		kfree(env_ready_queues);
	if (quantums != NULL)
	kfree(quantums);
}
void sched_insert_ready(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_READY ;
		enqueue(&(env_ready_queues[0]), env);
	}
}


void sched_remove_ready(struct Env* env)
{
	if(env != NULL)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			struct Env * ptr_env = find_env_in_queue(&(env_ready_queues[i]), env->env_id);
			if (ptr_env != NULL)
			{
				LIST_REMOVE(&(env_ready_queues[i]), env);
				env->env_status = ENV_UNKNOWN;
				return;
			}
		}
	}
}

void sched_insert_new(struct Env* env)
{
	if(env != NULL)
	{
		env->env_status = ENV_NEW ;
		enqueue(&env_new_queue, env);
	}
}
void sched_remove_new(struct Env* env)
{
	if(env != NULL)
	{
		LIST_REMOVE(&env_new_queue, env) ;
		env->env_status = ENV_UNKNOWN;
	}
}

void sched_insert_exit(struct Env* env)
{
	if(env != NULL)
	{
		if(isBufferingEnabled()) {cleanup_buffers(env);}
		env->env_status = ENV_EXIT ;
		enqueue(&env_exit_queue, env);
	}
}
void sched_remove_exit(struct Env* env)
{
	if(env != NULL)
	{
		LIST_REMOVE(&env_exit_queue, env) ;
		env->env_status = ENV_UNKNOWN;
	}
}

void sched_print_all()
{
	struct Env* ptr_env ;
	if (!LIST_EMPTY(&env_new_queue))
	{
		cprintf("\nThe processes in NEW queue are:\n");
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("\nNo processes in NEW queue\n");
	}
	cprintf("================================================\n");
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			cprintf("The processes in READY queue #%d are:\n", i);
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
			}
		}
		else
		{
			cprintf("No processes in READY queue #%d\n", i);
		}
		cprintf("================================================\n");
	}
	if (!LIST_EMPTY(&env_exit_queue))
	{
		cprintf("The processes in EXIT queue are:\n");
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			cprintf("	[%d] %s\n", ptr_env->env_id, ptr_env->prog_name);
		}
	}
	else
	{
		cprintf("No processes in EXIT queue\n");
	}
}

void sched_run_all()
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		sched_remove_new(ptr_env);
		sched_insert_ready(ptr_env);
	}
	/*2015*///if scheduler not run yet, then invoke it!
	if (scheduler_status == SCH_STOPPED)
		fos_scheduler();
}

void sched_kill_all()
{
	struct Env* ptr_env ;
	if (!LIST_EMPTY(&env_new_queue))
	{
		cprintf("\nKILLING the processes in the NEW queue...\n");
		LIST_FOREACH(ptr_env, &env_new_queue)
		{
			cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			sched_remove_new(ptr_env);
			start_env_free(ptr_env);
			cprintf("DONE\n");
		}
	}
	else
	{
		cprintf("No processes in NEW queue\n");
	}
	cprintf("================================================\n");
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			cprintf("KILLING the processes in the READY queue #%d...\n", i);
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
				LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
				start_env_free(ptr_env);
				cprintf("DONE\n");
			}
		}
		else
		{
			cprintf("No processes in READY queue #%d\n",i);
		}
		cprintf("================================================\n");
	}

	if (!LIST_EMPTY(&env_exit_queue))
	{
		cprintf("KILLING the processes in the EXIT queue...\n");
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			cprintf("	killing[%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			sched_remove_exit(ptr_env);
			start_env_free(ptr_env);
			cprintf("DONE\n");
		}
	}
	else
	{
		cprintf("No processes in EXIT queue\n");
	}

	//reinvoke the scheduler since there're no env to return back to it
	curenv = NULL;
	fos_scheduler();
}


void sched_new_env(struct Env* e)
{
	//add the given env to the scheduler NEW queue
	if (e!=NULL)
	{
		sched_insert_new(e);
	}
}
/*
void sched_new_env(uint32 envId)
{
	struct Env* e= NULL;
	envid2env(envId, &e, 0);

	if(e == 0) return;

	sched_insert_new(e);
}
 */

void sched_run_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	LIST_FOREACH(ptr_env, &env_new_queue)
	{
		if(ptr_env->env_id == envId)
		{
			sched_remove_new(ptr_env);
			sched_insert_ready(ptr_env);

			/*2015*///if scheduler not run yet, then invoke it!
			if (scheduler_status == SCH_STOPPED)
			{
				fos_scheduler();
			}
			break;
		}
	}
	//	cprintf("ready queue:\n");
	//	LIST_FOREACH(ptr_env, &env_ready_queue)
	//	{
	//		cprintf("%s - %d\n", ptr_env->prog_name, ptr_env->env_id);
	//	}

}

void sched_exit_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	int found = 0;
	if (!found)
	{
		LIST_FOREACH(ptr_env, &env_new_queue)
				{
			if(ptr_env->env_id == envId)
			{
				sched_remove_new(ptr_env);
				found = 1;
				//			return;
			}
				}
	}
	if (!found)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			if (!LIST_EMPTY(&(env_ready_queues[i])))
			{
				ptr_env=NULL;
				LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
				{
					if(ptr_env->env_id == envId)
					{
						LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
						found = 1;
						break;
					}
				}
			}
			if (found)
				break;
		}
	}
	if (!found)
	{
		if (curenv->env_id == envId)
		{
			ptr_env = curenv;
			found = 1;
		}
	}

	if (found)
	{
		sched_insert_exit(ptr_env);

		//If it's the curenv, then reinvoke the scheduler as there's no meaning to return back to an exited env
		if (curenv->env_id == envId)
		{
			curenv = NULL;
			fos_scheduler();
		}
	}
}

/*2018*/
void sched_exit_all_ready_envs()
{
	struct Env* ptr_env=NULL;
	for (int i = 0 ; i < num_of_ready_queues ; i++)
	{
		if (!LIST_EMPTY(&(env_ready_queues[i])))
		{
			ptr_env=NULL;
			LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
			{
				LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
				sched_insert_exit(ptr_env);
			}
		}
	}
}

/*2015*/
void sched_kill_env(uint32 envId)
{
	struct Env* ptr_env=NULL;
	int found = 0;
	if (!found)
	{
		LIST_FOREACH(ptr_env, &env_new_queue)
					{
			if(ptr_env->env_id == envId)
			{
				cprintf("killing[%d] %s from the NEW queue...", ptr_env->env_id, ptr_env->prog_name);
				sched_remove_new(ptr_env);
				start_env_free(ptr_env);
				cprintf("DONE\n");
				found = 1;
				//			return;
			}
					}
	}
	if (!found)
	{
		for (int i = 0 ; i < num_of_ready_queues ; i++)
		{
			if (!LIST_EMPTY(&(env_ready_queues[i])))
			{
				ptr_env=NULL;
				LIST_FOREACH(ptr_env, &(env_ready_queues[i]))
				{
					if(ptr_env->env_id == envId)
					{
						cprintf("killing[%d] %s from the READY queue #%d...", ptr_env->env_id, ptr_env->prog_name, i);
						LIST_REMOVE(&(env_ready_queues[i]), ptr_env);
						start_env_free(ptr_env);
						cprintf("DONE\n");
						found = 1;
						break;
						//return;
					}
				}
			}
			if (found)
				break;
		}
	}
	if (!found)
	{
		ptr_env=NULL;
		LIST_FOREACH(ptr_env, &env_exit_queue)
		{
			if(ptr_env->env_id == envId)
			{
				cprintf("killing[%d] %s from the EXIT queue...", ptr_env->env_id, ptr_env->prog_name);
				sched_remove_exit(ptr_env);
				start_env_free(ptr_env);
				cprintf("DONE\n");
				found = 1;
				//return;
			}
		}
	}

	if (!found)
	{
		if (curenv->env_id == envId)
		{
			ptr_env = curenv;
			assert(ptr_env->env_id == ENV_RUNNABLE);
			cprintf("killing a RUNNABLE environment [%d] %s...", ptr_env->env_id, ptr_env->prog_name);
			start_env_free(ptr_env);
			cprintf("DONE\n");
			found = 1;
		}
	}
	//If it's the curenv, then reset it and reinvoke the scheduler
	//as there's no meaning to return back to a killed env
	if (curenv->env_id == envId)
	{
		//lcr3(K_PHYSICAL_ADDRESS(ptr_page_directory));
		lcr3(phys_page_directory);
		curenv = NULL;
		fos_scheduler();
	}

}


void clock_interrupt_handler()
{
	//cputchar('i');

	if(isPageReplacmentAlgorithmLRU())
	{
		update_WS_time_stamps();
	}
	//cprintf("Clock Handler\n") ;
	fos_scheduler();
}
/*
void on_clock_update_WS_time_stamps()
{
	//cprintf("Updating time stamps\n");

	struct Env *curr_env_ptr = NULL;

	LIST_FOREACH(curr_env_ptr, &env_ready_queue)
	{
		int i ;
		for (i = 0 ; i < PAGE_WS_MAX_SIZE; i++)
		{
			if( curr_env_ptr->ptr_pageWorkingSet[i].empty != 1)
			{
				//update the time if the page was referenced
				uint32 page_va = curr_env_ptr->ptr_pageWorkingSet[i].virtual_address ;
				uint32 perm = pt_get_page_permissions(curr_env_ptr, page_va) ;
				if (perm & PERM_USED)
				{
					curr_env_ptr->ptr_pageWorkingSet[i].time_stamp = time ;
					pt_set_page_permissions(curr_env_ptr, page_va, 0 , PERM_USED) ;
				}
			}
		}

		{
			//uint32 eflags_val = read_eflags();
			//write_eflags(eflags_val & (~FL_IF));

			//if((curr_env_ptr->pageFaultsCounter-old_pf_counter) > 0)
			{
				//cprintf("[%s]: last clock # of PFs  = %d, timenow = %d\n", curr_env_ptr->prog_name ,curr_env_ptr->pageFaultsCounter-old_pf_counter, time);
			}
			//mydblchk += curr_env_ptr->pageFaultsCounter-old_pf_counter;
			old_pf_counter = curr_env_ptr->pageFaultsCounter;

			//eflags_val = read_eflags();
			//write_eflags(eflags_val | (FL_IF));
		}
	}

}
 */
void update_WS_time_stamps()
{
	struct Env *curr_env_ptr = curenv;

	if(curr_env_ptr != NULL)
	{
		{
			int i ;
			for (i = 0 ; i < (curr_env_ptr->page_WS_max_size); i++)
			{
				if( curr_env_ptr->ptr_pageWorkingSet[i].empty != 1)
				{
					//update the time if the page was referenced
					uint32 page_va = curr_env_ptr->ptr_pageWorkingSet[i].virtual_address ;
					uint32 perm = pt_get_page_permissions(curr_env_ptr, page_va) ;
					uint32 oldTimeStamp = curr_env_ptr->ptr_pageWorkingSet[i].time_stamp;

					if (perm & PERM_USED)
					{
						curr_env_ptr->ptr_pageWorkingSet[i].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pt_set_page_permissions(curr_env_ptr, page_va, 0 , PERM_USED) ;
					}
					else
					{
						curr_env_ptr->ptr_pageWorkingSet[i].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}

		{
			int t ;
			for (t = 0 ; t < __TWS_MAX_SIZE; t++)
			{
				if( curr_env_ptr->__ptr_tws[t].empty != 1)
				{
					//update the time if the page was referenced
					uint32 table_va = curr_env_ptr->__ptr_tws[t].virtual_address;
					uint32 oldTimeStamp = curr_env_ptr->__ptr_tws[t].time_stamp;

					if (pd_is_table_used(curr_env_ptr, table_va))
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2) | 0x80000000;
						pd_set_table_unused(curr_env_ptr, table_va);
					}
					else
					{
						curr_env_ptr->__ptr_tws[t].time_stamp = (oldTimeStamp>>2);
					}
				}
			}
		}
	}
}


