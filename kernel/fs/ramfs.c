#include <types.h>
#include <fs/ramfs.h>
#include <fs/vfs.h>
#include <memory/kheap.h>
#include <lib/string.h>
#include <lib/stdio.h>

static ino_t next_inode = 1;

static ssize_t ramfs_read(vfs_node_t *node, off_t offset, size_t size, uint8_t *buffer)
{
    ramfs_file_t *file = (ramfs_file_t *)node->private_data;
    if (!file || !buffer)
        return -1;

    if (offset >= (off_t)file->size)
        return 0;

    size_t available = file->size - offset;
    if (size > available)
        size = available;

    memcpy(buffer, file->data + offset, size);
    return (ssize_t)size;
}

static ssize_t ramfs_write(vfs_node_t *node, off_t offset, size_t size, const uint8_t *buffer)
{
    ramfs_file_t *file = (ramfs_file_t *)node->private_data;
    if (!file || !buffer)
        return -1;

    size_t new_size = offset + size;
    if (new_size > file->capacity) {
        size_t new_cap = new_size * 2;
        if (new_cap > RAMFS_MAX_FILE_SIZE)
            new_cap = RAMFS_MAX_FILE_SIZE;
        if (new_size > new_cap)
            return -1;

        uint8_t *new_data = (uint8_t *)kmalloc(new_cap);
        if (!new_data)
            return -1;

        if (file->data && file->size > 0)
            memcpy(new_data, file->data, file->size);

        if (file->data)
            kfree(file->data);

        file->data = new_data;
        file->capacity = new_cap;
    }

    memcpy(file->data + offset, buffer, size);
    if (new_size > file->size)
        file->size = new_size;

    node->size = file->size;
    return (ssize_t)size;
}

static int ramfs_open(vfs_node_t *node, uint32_t flags)
{
    (void)node;
    (void)flags;
    return 0;
}

static int ramfs_close(vfs_node_t *node)
{
    (void)node;
    return 0;
}

static vfs_node_t *ramfs_readdir(vfs_node_t *node, uint32_t index)
{
    if (!(node->type & VFS_DIRECTORY))
        return NULL;

    vfs_node_t *child = node->children;
    uint32_t i = 0;
    while (child) {
        if (i == index)
            return child;
        child = child->next_sibling;
        i++;
    }
    return NULL;
}

static vfs_node_t *ramfs_finddir(vfs_node_t *node, const char *name)
{
    if (!(node->type & VFS_DIRECTORY))
        return NULL;

    vfs_node_t *child = node->children;
    while (child) {
        if (strcmp(child->name, name) == 0)
            return child;
        child = child->next_sibling;
    }
    return NULL;
}

static vfs_node_t *ramfs_create_node(const char *name, uint32_t type)
{
    vfs_node_t *node = (vfs_node_t *)kmalloc(sizeof(vfs_node_t));
    if (!node)
        return NULL;

    memset(node, 0, sizeof(vfs_node_t));
    strncpy(node->name, name, VFS_NAME_MAX - 1);
    node->type = type;
    node->inode = next_inode++;
    node->permissions = 0755;

    static vfs_ops_t ramfs_ops = {
        .read    = ramfs_read,
        .write   = ramfs_write,
        .open    = ramfs_open,
        .close   = ramfs_close,
        .readdir = ramfs_readdir,
        .finddir = ramfs_finddir,
        .create  = NULL,
        .unlink  = NULL,
        .mkdir   = NULL,
    };
    node->ops = &ramfs_ops;

    if (type & VFS_FILE) {
        ramfs_file_t *file = (ramfs_file_t *)kmalloc(sizeof(ramfs_file_t));
        if (file) {
            file->data = NULL;
            file->size = 0;
            file->capacity = 0;
            node->private_data = file;
        }
    }

    return node;
}

static int ramfs_do_create(vfs_node_t *parent, const char *name, uint32_t type)
{
    if (!(parent->type & VFS_DIRECTORY))
        return -1;

    if (ramfs_finddir(parent, name))
        return -1;

    vfs_node_t *node = ramfs_create_node(name, type);
    if (!node)
        return -1;

    node->parent = parent;
    node->next_sibling = parent->children;
    parent->children = node;

    return 0;
}

static int ramfs_do_unlink(vfs_node_t *parent, const char *name)
{
    if (!(parent->type & VFS_DIRECTORY))
        return -1;

    vfs_node_t *prev = NULL;
    vfs_node_t *child = parent->children;

    while (child) {
        if (strcmp(child->name, name) == 0) {
            if (prev)
                prev->next_sibling = child->next_sibling;
            else
                parent->children = child->next_sibling;

            if (child->private_data) {
                ramfs_file_t *file = (ramfs_file_t *)child->private_data;
                if (file->data)
                    kfree(file->data);
                kfree(file);
            }
            kfree(child);
            return 0;
        }
        prev = child;
        child = child->next_sibling;
    }

    return -1;
}

static int ramfs_do_mkdir(vfs_node_t *parent, const char *name, mode_t mode)
{
    if (!(parent->type & VFS_DIRECTORY))
        return -1;

    if (ramfs_finddir(parent, name))
        return -1;

    vfs_node_t *dir = ramfs_create_node(name, VFS_DIRECTORY);
    if (!dir)
        return -1;

    dir->permissions = mode;
    dir->parent = parent;
    dir->next_sibling = parent->children;
    parent->children = dir;

    /* Set the mkdir/create/unlink ops on this new dir too */
    static vfs_ops_t ramfs_dir_ops = {
        .read    = ramfs_read,
        .write   = ramfs_write,
        .open    = ramfs_open,
        .close   = ramfs_close,
        .readdir = ramfs_readdir,
        .finddir = ramfs_finddir,
        .create  = ramfs_do_create,
        .unlink  = ramfs_do_unlink,
        .mkdir   = ramfs_do_mkdir,
    };
    dir->ops = &ramfs_dir_ops;

    return 0;
}

vfs_node_t *ramfs_init(void)
{
    vfs_node_t *root = ramfs_create_node("/", VFS_DIRECTORY);
    if (!root)
        return NULL;

    static vfs_ops_t ramfs_root_ops = {
        .read    = ramfs_read,
        .write   = ramfs_write,
        .open    = ramfs_open,
        .close   = ramfs_close,
        .readdir = ramfs_readdir,
        .finddir = ramfs_finddir,
        .create  = ramfs_do_create,
        .unlink  = ramfs_do_unlink,
        .mkdir   = ramfs_do_mkdir,
    };
    root->ops = &ramfs_root_ops;

    kprintf("[RAMFS] RAM filesystem initialized\n");
    return root;
}
