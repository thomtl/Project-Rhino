#include <rhino/user/program.h>
#include <rhino/arch/x86/drivers/screen.h>
#include <rhino/fs/vfs.h>
#include <rhino/fs/initrd.h>
#include <rhino/mm/hmm.h>
#include <rhino/common.h>
#include <libk/string.h>
#include <rhino/multitasking/task.h>
extern task_t* taskArray;
/**
   @brief Loads a program from the INITRD into Memory.
   @param args List of args.  args[0] is the filename of the file to search.  args[1] is the binary type of the file.
   @return returns a pointer to a struct which contains info over the loaded program, in case of an error it will return 0.
 */
loaded_program_t* load_program(char* filename, uint8_t type){
    if(type == PROGRAM_BINARY_TYPE_BIN){
      fs_node_t *node = 0;
      node = finddir_fs(fs_root, filename);
      if(node == 0){
        kprint_err("program.c: Could not find program on the INITRD\n");
        return 0;
      }
      if((node->flags & 0x7) == FS_DIRECTORY){
        kprint_err("program.c: Selected File is a Directory\n");
        return 0;
      }
      loaded_program_t* header = kmalloc(sizeof(loaded_program_t));
      uint32_t* buf = kmalloc(node->length);
      header->base = (uint32_t*)buf;
      header->end = (uint32_t*)(buf + node->length);
      header->len = node->length;
      header->type = type;
      read_fs(node, 0, node->length, (uint8_t*)buf);
      return header;
    } else {
      kprint_err("program.c: ELF and A.OUT types are not yet implemented!\n");
      return 0;
    }
}
/**
   @brief Calls a program in memory.
   @param args List of args.  args[0] is the address to call.
   @return returns nothing.
 */
void run_program(void *address){
  //call_module_t prog = (call_module_t) address;
  //prog();

  createTask(address, taskArray[0].regs.eflags, taskArray[0].regs.cr3);
  return;
}

/**
   @brief Frees a program in memory.
   @param args List of args.  args[0] is the address of the program info struct.
   @return returns nothing.
 */
void free_program(loaded_program_t* header){
  kfree(header->base);
  kfree(header);
}
