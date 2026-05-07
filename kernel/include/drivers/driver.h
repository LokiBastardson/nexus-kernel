#ifndef _KERNEL_DRIVERS_DRIVER_H
#define _KERNEL_DRIVERS_DRIVER_H

#include <types.h>

#define DRIVER_NAME_MAX  64
#define MAX_DRIVERS      64

typedef enum {
    DRIVER_TYPE_CHAR,
    DRIVER_TYPE_BLOCK,
    DRIVER_TYPE_NETWORK,
    DRIVER_TYPE_DISPLAY,
    DRIVER_TYPE_INPUT,
    DRIVER_TYPE_STORAGE,
    DRIVER_TYPE_OTHER
} driver_type_t;

typedef enum {
    DRIVER_STATE_UNLOADED,
    DRIVER_STATE_LOADED,
    DRIVER_STATE_ACTIVE,
    DRIVER_STATE_ERROR
} driver_state_t;

struct driver;

typedef int  (*driver_init_fn)(struct driver *drv);
typedef void (*driver_cleanup_fn)(struct driver *drv);
typedef ssize_t (*driver_read_fn)(struct driver *drv, void *buffer, size_t size);
typedef ssize_t (*driver_write_fn)(struct driver *drv, const void *buffer, size_t size);
typedef int  (*driver_ioctl_fn)(struct driver *drv, uint32_t cmd, void *arg);

typedef struct driver_ops {
    driver_init_fn    init;
    driver_cleanup_fn cleanup;
    driver_read_fn    read;
    driver_write_fn   write;
    driver_ioctl_fn   ioctl;
} driver_ops_t;

typedef struct driver {
    char            name[DRIVER_NAME_MAX];
    driver_type_t   type;
    driver_state_t  state;
    uint32_t        major;
    uint32_t        minor;
    driver_ops_t   *ops;
    void           *private_data;
    struct driver  *next;
} driver_t;

void      driver_manager_init(void);
int       driver_register(driver_t *drv);
int       driver_unregister(const char *name);
driver_t *driver_find(const char *name);
driver_t *driver_find_by_type(driver_type_t type);
int       driver_load(const char *name);
int       driver_unload(const char *name);
void      driver_list_all(void);

#endif
