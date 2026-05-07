#ifndef _KERNEL_FS_VFS_H
#define _KERNEL_FS_VFS_H

#include <types.h>

#define VFS_FILE      0x01
#define VFS_DIRECTORY 0x02
#define VFS_CHARDEV   0x04
#define VFS_BLOCKDEV  0x08
#define VFS_PIPE      0x10
#define VFS_SYMLINK   0x20
#define VFS_MOUNTPOINT 0x40

#define VFS_NAME_MAX  128
#define VFS_PATH_MAX  1024
#define VFS_MAX_OPEN_FILES 256

struct vfs_node;

typedef ssize_t (*read_fn_t)(struct vfs_node *, off_t, size_t, uint8_t *);
typedef ssize_t (*write_fn_t)(struct vfs_node *, off_t, size_t, const uint8_t *);
typedef int     (*open_fn_t)(struct vfs_node *, uint32_t flags);
typedef int     (*close_fn_t)(struct vfs_node *);
typedef struct vfs_node *(*readdir_fn_t)(struct vfs_node *, uint32_t index);
typedef struct vfs_node *(*finddir_fn_t)(struct vfs_node *, const char *name);
typedef int     (*create_fn_t)(struct vfs_node *, const char *name, uint32_t type);
typedef int     (*unlink_fn_t)(struct vfs_node *, const char *name);
typedef int     (*mkdir_fn_t)(struct vfs_node *, const char *name, mode_t mode);

typedef struct vfs_ops {
    read_fn_t    read;
    write_fn_t   write;
    open_fn_t    open;
    close_fn_t   close;
    readdir_fn_t readdir;
    finddir_fn_t finddir;
    create_fn_t  create;
    unlink_fn_t  unlink;
    mkdir_fn_t   mkdir;
} vfs_ops_t;

typedef struct vfs_node {
    char         name[VFS_NAME_MAX];
    uint32_t     type;
    uint32_t     permissions;
    uint32_t     uid;
    uint32_t     gid;
    ino_t        inode;
    size_t       size;
    uint32_t     flags;
    struct vfs_node *parent;
    struct vfs_node *children;
    struct vfs_node *next_sibling;
    struct vfs_node *mount_point;
    vfs_ops_t   *ops;
    void        *private_data;
} vfs_node_t;

typedef struct vfs_mount {
    char             path[VFS_PATH_MAX];
    char             fs_type[32];
    vfs_node_t      *root;
    struct vfs_mount *next;
} vfs_mount_t;

void        vfs_init(void);
vfs_node_t *vfs_get_root(void);
ssize_t     vfs_read(vfs_node_t *node, off_t offset, size_t size, uint8_t *buffer);
ssize_t     vfs_write(vfs_node_t *node, off_t offset, size_t size, const uint8_t *buffer);
int         vfs_open(vfs_node_t *node, uint32_t flags);
int         vfs_close(vfs_node_t *node);
vfs_node_t *vfs_readdir(vfs_node_t *node, uint32_t index);
vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name);
int         vfs_create(vfs_node_t *parent, const char *name, uint32_t type);
int         vfs_unlink(vfs_node_t *parent, const char *name);
int         vfs_mkdir(vfs_node_t *parent, const char *name, mode_t mode);
int         vfs_mount(const char *path, const char *fs_type, vfs_node_t *root);
vfs_node_t *vfs_resolve_path(const char *path);

#endif
