#include <types.h>
#include <fs/vfs.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <memory/kheap.h>

#define MAX_MOUNTS 16

static vfs_node_t  *vfs_root;
static vfs_mount_t *mount_list;
static uint32_t     mount_count;

void vfs_init(void)
{
    vfs_root = NULL;
    mount_list = NULL;
    mount_count = 0;

    kprintf("[VFS] Virtual file system initialized\n");
}

vfs_node_t *vfs_get_root(void)
{
    return vfs_root;
}

ssize_t vfs_read(vfs_node_t *node, off_t offset, size_t size, uint8_t *buffer)
{
    if (!node || !node->ops || !node->ops->read)
        return -1;
    return node->ops->read(node, offset, size, buffer);
}

ssize_t vfs_write(vfs_node_t *node, off_t offset, size_t size, const uint8_t *buffer)
{
    if (!node || !node->ops || !node->ops->write)
        return -1;
    return node->ops->write(node, offset, size, buffer);
}

int vfs_open(vfs_node_t *node, uint32_t flags)
{
    if (!node || !node->ops || !node->ops->open)
        return -1;
    return node->ops->open(node, flags);
}

int vfs_close(vfs_node_t *node)
{
    if (!node || !node->ops || !node->ops->close)
        return -1;
    return node->ops->close(node);
}

vfs_node_t *vfs_readdir(vfs_node_t *node, uint32_t index)
{
    if (!node || !node->ops || !node->ops->readdir)
        return NULL;
    return node->ops->readdir(node, index);
}

vfs_node_t *vfs_finddir(vfs_node_t *node, const char *name)
{
    if (!node || !node->ops || !node->ops->finddir)
        return NULL;
    return node->ops->finddir(node, name);
}

int vfs_create(vfs_node_t *parent, const char *name, uint32_t type)
{
    if (!parent || !parent->ops || !parent->ops->create)
        return -1;
    return parent->ops->create(parent, name, type);
}

int vfs_unlink(vfs_node_t *parent, const char *name)
{
    if (!parent || !parent->ops || !parent->ops->unlink)
        return -1;
    return parent->ops->unlink(parent, name);
}

int vfs_mkdir(vfs_node_t *parent, const char *name, mode_t mode)
{
    if (!parent || !parent->ops || !parent->ops->mkdir)
        return -1;
    return parent->ops->mkdir(parent, name, mode);
}

int vfs_mount(const char *path, const char *fs_type, vfs_node_t *root)
{
    if (!path || !fs_type || !root)
        return -1;

    if (mount_count >= MAX_MOUNTS) {
        kprintf("[VFS] ERROR: Maximum mount points reached\n");
        return -1;
    }

    vfs_mount_t *mount = (vfs_mount_t *)kmalloc(sizeof(vfs_mount_t));
    if (!mount)
        return -1;

    strncpy(mount->path, path, VFS_PATH_MAX - 1);
    strncpy(mount->fs_type, fs_type, 31);
    mount->root = root;
    mount->next = mount_list;
    mount_list = mount;
    mount_count++;

    if (strcmp(path, "/") == 0)
        vfs_root = root;

    kprintf("[VFS] Mounted '%s' filesystem at '%s'\n", fs_type, path);
    return 0;
}

vfs_node_t *vfs_resolve_path(const char *path)
{
    if (!path || !vfs_root)
        return NULL;

    if (strcmp(path, "/") == 0)
        return vfs_root;

    vfs_node_t *current = vfs_root;
    char pathbuf[VFS_PATH_MAX];
    strncpy(pathbuf, path, VFS_PATH_MAX - 1);

    char *token = pathbuf;
    if (*token == '/')
        token++;

    while (*token && current) {
        char *next_sep = strchr(token, '/');
        if (next_sep)
            *next_sep = '\0';

        if (strlen(token) > 0) {
            current = vfs_finddir(current, token);
            if (!current)
                return NULL;
        }

        if (next_sep)
            token = next_sep + 1;
        else
            break;
    }

    return current;
}
