#pragma once

#include <stdint.h>
#include <rhino/common.h>

#define MAX_TASKS 256
typedef struct {
  uint32_t eax, ebx, ecx, edx, esi, edi, esp, ebp, eip, eflags;
  uintptr_t cr3;
} task_registers_t;

typedef struct {
  uint32_t pid;
  uint32_t user;
  uint32_t flags;
  char *path;
} pid_t;

typedef struct Task {
  task_registers_t regs;
  bool used;
  pid_t pid;
} task_t;

void initTasking();

void createTask(void(*)(), uint32_t, uintptr_t);

task_t* get_running_task();
uint32_t get_current_pid();
task_t* task_for_pid(uint32_t pid);

task_t* fork(void);
task_t* fork_sys(uint32_t eip);
void exec(task_t* task, void(*main)());
void yield();
void kill(uint32_t pid);
void waitpid(uint32_t pid);


void start_task_atomic();
void end_task_atomic();



extern void switchTask(task_registers_t *old, task_registers_t *new);