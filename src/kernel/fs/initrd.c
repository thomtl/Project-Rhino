#include <rhino/fs/initrd.h>
#include <rhino/mm/pmm.h>
#include <libk/string.h>

initrd_header_t *initrd_header;
initrd_file_header_t *file_headers;
fs_node_t *initrd_root;

fs_node_t *root_nodes;
uint32_t nroot_nodes;

struct dirent* dirent;

static uint32_t initrd_read(fs_node_t *node, uint64_t offset, uint32_t size, uint8_t *buffer)
{
	initrd_file_header_t header = file_headers[node->inode];
   	if (offset > header.length)  return 0;
   	if (offset+size > header.length) size = header.length-offset;
   	memcpy(buffer, (uint8_t*) (uint32_t)(header.offset+offset), size);
   	return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index)
{
  	UNUSED(node);
  	if (index >= nroot_nodes)
    	return 0;
  	strcpy(dirent->name, root_nodes[index].name);
  	dirent->name[strlen(root_nodes[index].name)] = 0; // Make sure the string is NULL-terminated.
  	dirent->inode = root_nodes[index].inode;
  	return dirent;
}

static fs_node_t *initrd_finddir(fs_node_t *node, char *name)
{  
  	UNUSED(node);

  	uint32_t i;
  	for (i = 0; i < nroot_nodes; i++)
    	if (!strcmp(name, root_nodes[i].name))
      		return &root_nodes[i];
  	return 0;
}

void initialise_initrd(uint32_t location){
  	debug_log("[INITRD]: Initializing INITRD\n");

	dirent = kmalloc(sizeof(struct dirent));

  	initrd_header = (initrd_header_t *)location;
  	file_headers = (initrd_file_header_t *) (location+sizeof(initrd_header_t));

  	initrd_root = (fs_node_t*)kmalloc(sizeof(fs_node_t));
  	strcpy(initrd_root->name, "initrd");

  	initrd_root->inode = initrd_root->mask = initrd_root->uid = initrd_root->gid = initrd_root->length = 0;
  	initrd_root->flags = FS_DIRECTORY;
  	initrd_root->read = 0;
  	initrd_root->write = 0;
  	initrd_root->open = 0;
  	initrd_root->close = 0;
  	initrd_root->readdir = &initrd_readdir;
  	initrd_root->finddir = &initrd_finddir;
  	initrd_root->readlink = 0;
  	initrd_root->ptr = 0;
  	initrd_root->impl = 0;

  	root_nodes = (fs_node_t*)kmalloc(sizeof(fs_node_t) * initrd_header->nfiles);

  	nroot_nodes = initrd_header->nfiles;

  	uint32_t i;

  	for (i = 0; i < initrd_header->nfiles; i++)
  	{
    	// Edit the file's header - currently it holds the file offset
    	// relative to the start of the ramdisk. We want it relative to the start
    	// of memory.
    	file_headers[i].offset += location;
    	// Create a new file node.
    	strcpy(root_nodes[i].name, (char*)&file_headers[i].name);
    	root_nodes[i].mask = root_nodes[i].uid = root_nodes[i].gid = 0;
    	root_nodes[i].length = file_headers[i].length;

    	root_nodes[i].inode = i;
    	root_nodes[i].flags = FS_FILE;
    	root_nodes[i].read = &initrd_read;
    	root_nodes[i].write = 0;
    	root_nodes[i].readdir = 0;
    	root_nodes[i].finddir = 0;
    	root_nodes[i].readlink = 0;
    	root_nodes[i].open = 0;
    	root_nodes[i].close = 0;
    	root_nodes[i].impl = 0;
  	}

  	if(vfs_mount("/dev/initrd", initrd_root) == NULL){
    	debug_log("[INITRD]: Failed to mount initrd\n");
    	return;
  	}

  	debug_log("[INITRD]: INITRD Initialized\n");
}
