#include <rhino/multitasking/task.h>
#include <rhino/arch/x86/drivers/screen.h>
#include <rhino/arch/x86/drivers/fpu.h>
#include <rhino/arch/x86/timer.h>
#include <rhino/arch/x86/tss.h>
#include <libk/string.h>
#include <rhino/mm/hmm.h>
#include <rhino/multitasking/scheduler.h>
static task_t *runningTask;
task_t tasks[MAX_TASKS];
task_t* taskArray = tasks;

/**
   @brief Searches the tasks array for the last filles element.
   @return returns the index of the final element.
 */
static uint32_t getFinalElement(){
  for(uint32_t i = 0; i < MAX_TASKS; i++){
    if(tasks[i].used == false) return i;
  }
  return 0;
}

/**
   @brief Get a task with pid.
   @param args List of args.  args[0] is the pid to search.
   @return returns a pointer to the task.
 */
static task_t* getTaskForPid(uint32_t pid){
  for(uint32_t i = 0; i < MAX_TASKS; i++){
    if(tasks[i].pid.pid == pid) return &(tasks[i]);
  }
  return NULL;
}

/**
   @brief Initialize tasking.
 */
void initTasking(){
  for(uint32_t i = 0; i < MAX_TASKS; i++){
    memset(&tasks[i], 0, sizeof(task_t));
    tasks[i].used = false;
    tasks[i].state = TASK_ACTIVE;
    tasks[i].pid.path = strdup("/");

    tasks[i].fd_table.refs = 1;
    tasks[i].fd_table.length = 0;
    tasks[i].fd_table.capacity = 4;
    tasks[i].fd_table.entries = kmalloc(sizeof(fs_node_t*) * tasks[i].fd_table.capacity);
    tasks[i].fd_table.modes = kmalloc(sizeof(int) * tasks[i].fd_table.capacity);
    tasks[i].fd_table.offsets = kmalloc(sizeof(uint64_t) * tasks[i].fd_table.capacity);
  }


  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(tasks[0].regs.cr3)::"%eax");
  asm volatile("pushfl; movl (%%esp), %%eax; movl %%eax, %0; popfl;":"=m"(tasks[0].regs.eflags)::"%eax");
  tasks[0].pid.pid = 0;
  tasks[0].used = true;

  runningTask = &tasks[0];
}

/**
   @brief Create a user task. responsibilty of the caller to set esp
   @param args List of args.  args[0] the pointer to where to locate the task.  args[1] is a pointer to the main function. args[2] is the EFLAGS register. args[3] is the cr3 register.
 */
task_t* createTask(void(*main)(), uint32_t flags, uintptr_t pagedir){
  task_t* task = &tasks[getFinalElement()];
  task->regs.eax = 0;
  task->regs.ebx = 0;
  task->regs.ecx = 0;
  task->regs.edx = 0;
  task->regs.esi = 0;
  task->regs.edi = 0;
  task->regs.eflags = flags;
  task->regs.eip = (uint32_t) main;
  task->regs.cr3 = pagedir;
  task->pid.pid = tasks[getFinalElement() - 1].pid.pid + 1;
  task->res.frameIndex = 0;
  task->regs.ss = 0x23; // KERNEL RING 0 VALUE is 0x10
  task->regs.cs = 0x1B; // KERNEL RING 0 VALUE is 0x08
  task->regs.ds = 0x23; // KERNEL RING 0 VALUE is 0x10
  task->used = true;
  return task;
}

/**
   @brief Kill a task.
   @param args List of args.  args[0] is the pid of the task to kill.
 */
void kill(uint32_t pid){
  if(pid == 0){
    kill_kern();
    return;
  }
  task_t* task = task_for_pid(pid);
  task->regs.esp -= 0x1000;
  pmm_free_block((void*)task->regs.esp);
  for(uint32_t i = task->res.frameIndex; i > 0; i--) pmm_free_block(task->res.frames[i]);
  vmm_free_dir((pdirectory*)((uint32_t)task->regs.cr3 + KERNEL_VBASE));
  task->res.frameIndex = 0;
  task->used = false;

  task->fd_table.refs--;
  if(task->fd_table.refs == 0){
    for(uint32_t i = 0; i < task->fd_table.length; i++){
      if(task->fd_table.entries[i]){
        close_fs(task->fd_table.entries[i]);
        task->fd_table.entries[i] = NULL;
      }
    }
  }
}

void kill_kern(){
  block_task(TASK_BLOCKED);
}

void switch_context(task_t *old, task_t *new, registers_t *regs){
  old->regs.eax = regs->eax;
  old->regs.ebx = regs->ebx;
  old->regs.ecx = regs->ecx;
  old->regs.edx = regs->edx;
  old->regs.esi = regs->esi;
  old->regs.edi = regs->edi;
  old->regs.ebp = regs->ebp;
  old->regs.eflags = regs->eflags;
  old->regs.eip = regs->eip;
  old->regs.esp = regs->esp;
  old->regs.ss = regs->ss;
  old->regs.cs = regs->cs;
  old->regs.ds = regs->ds;
  asm volatile("movl %%cr3, %%eax; movl %%eax, %0;":"=m"(old->regs.cr3)::"%eax");
  save_fpu_context(old);

  regs->eax = new->regs.eax;
  regs->ebx = new->regs.ebx;
  regs->ecx = new->regs.ecx;
  regs->edx = new->regs.edx;
  regs->esi = new->regs.esi;
  regs->edi = new->regs.edi;
  regs->ebp = new->regs.ebp;
  regs->eflags = new->regs.eflags;
  BIT_SET(regs->eflags, 9);
  regs->eip = new->regs.eip;
  regs->esp = new->regs.esp;
  regs->ss = new->regs.ss;
  regs->cs = new->regs.cs;
  regs->ds = new->regs.ds;
  //asm("mov %0, %%cr3":: "r"(new->regs.cr3));
  vmm_switch_pdirectory((pdirectory*)((uint32_t)new->regs.cr3 + KERNEL_VBASE));
  restore_fpu_context(new);
}


/**
   @brief get a pointer to the current task.
   @return returns a pointer to the location of the current task.
 */
task_t* get_running_task(){
  return runningTask;
}

/**
   @brief returns the pid of the current task.
   @return returns an integer that is equal to the pid of the current task.
 */
uint32_t get_current_pid(){
  return runningTask->pid.pid;
}


/**
   @brief Get the task for the pid.
   @param args List of args.  args[0] is the pid to search.
   @return returns a pointer to the task with pid in args[0].
 */
task_t* task_for_pid(uint32_t pid){
  return getTaskForPid(pid);
}

/**
   @brief waits until task with pid has exited.
   @param args List of args.  args[0] is the pid of the task to wait for.
 */
void waitpid(uint32_t pid){
  asm volatile("movl %%esp, %%eax; movl %%eax, %0;":"=m"(get_tss()->esp0)::"%eax");
  asm("sti");
  task_t* task = task_for_pid(pid);
  while(task->used == true);
  asm("cli");
}

/**
   @brief starts a period of garanteed atomicity for the task.
 */
void start_task_atomic(){
  disable_scheduling();
}

/**
   @brief stops a period of garanteed atomicity for the task.
 */
void end_task_atomic(){
  enable_scheduling();
}

void task_register_frame(task_t* task, void* frame){
  if(task->res.frameIndex == TASK_MAX_FRAMES) PANIC_M("Task has tried to allocate more than the max frames\n");
  task->res.frames[task->res.frameIndex] = frame;
  task->res.frameIndex++;
}

void task_set_argv(pid_t pid, uint32_t argv){
  task_for_pid(pid)->argv = argv;
}

uint32_t task_get_argv(pid_t pid){
  return task_for_pid(pid)->argv;
}

void task_set_argc(pid_t pid, uint32_t argc){
  task_for_pid(pid)->argc = argc;
}

uint32_t task_get_argc(pid_t pid){
  return task_for_pid(pid)->argc;
}

task_t* get_task_array(){
  return taskArray;
}

void set_running_task(task_t* task){
  runningTask = task;
}

void block_task(uint32_t reason){
  get_running_task()->state = reason;
}

void unblock_task(){
  get_running_task()->state = TASK_ACTIVE;
}

void task_set_working_directory(task_t* task, char* dir){
  char* clone = strdup(dir);
  if(clone == NULL) return;
  char* backup = task->pid.path;
  task->pid.path = clone;
  kfree(backup);
}
char* task_get_working_directory(task_t* task){
  return task->pid.path;
}

uint32_t task_append_fd(task_t* task, fs_node_t* node){
  for(uint32_t i = 0; i < task->fd_table.length; i++){
    if(!task->fd_table.entries[i]){
      task->fd_table.entries[i] = node;

      task->fd_table.modes[i] = 0;
      task->fd_table.offsets[i] = 0;
      return i;
    }
  }

  if(task->fd_table.length == task->fd_table.capacity){
    task->fd_table.capacity *= 2;

    task->fd_table.entries = krealloc(task->fd_table.entries, sizeof(fs_node_t*) * task->fd_table.capacity);
    task->fd_table.modes = krealloc(task->fd_table.modes, sizeof(int) * task->fd_table.capacity);
    task->fd_table.offsets = krealloc(task->fd_table.offsets, sizeof(uint64_t) * task->fd_table.capacity);
  }

  task->fd_table.entries[task->fd_table.length] = node;
  task->fd_table.modes[task->fd_table.length] = 0;
  task->fd_table.offsets[task->fd_table.length] = 0;

  task->fd_table.length++;
  return task->fd_table.length-1;
}