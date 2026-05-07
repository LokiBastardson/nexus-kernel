#ifndef _KERNEL_FS_RAMFS_H
#define _KERNEL_FS_RAMFS_H

#include <types.h>
#include <fs/vfs.h>

#define RAMFS_MAX_FILE_SIZE (64 * 1024)
#define RAMFS_MAX_FILES     128

typedef struct ramfs_file {
    uint8_t *data;
    size_t   size;
    size_t   capacity;
} ramfs_file_t;

vfs_node_t *ramfs_init(void);

#endif
