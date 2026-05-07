#include <types.h>
#include <drivers/driver.h>
#include <lib/string.h>
#include <lib/stdio.h>

static driver_t *driver_list;
static uint32_t  driver_count;

void driver_manager_init(void)
{
    driver_list = NULL;
    driver_count = 0;
    kprintf("[DRV] Driver manager initialized\n");
}

int driver_register(driver_t *drv)
{
    if (!drv || !drv->name[0])
        return -1;

    if (driver_count >= MAX_DRIVERS) {
        kprintf("[DRV] ERROR: Maximum drivers reached\n");
        return -1;
    }

    if (driver_find(drv->name)) {
        kprintf("[DRV] ERROR: Driver '%s' already registered\n", drv->name);
        return -1;
    }

    drv->state = DRIVER_STATE_LOADED;
    drv->next = driver_list;
    driver_list = drv;
    driver_count++;

    kprintf("[DRV] Registered driver '%s' (type=%d)\n", drv->name, drv->type);
    return 0;
}

int driver_unregister(const char *name)
{
    if (!name)
        return -1;

    driver_t *prev = NULL;
    driver_t *drv = driver_list;

    while (drv) {
        if (strcmp(drv->name, name) == 0) {
            if (drv->state == DRIVER_STATE_ACTIVE && drv->ops && drv->ops->cleanup)
                drv->ops->cleanup(drv);

            if (prev)
                prev->next = drv->next;
            else
                driver_list = drv->next;

            drv->state = DRIVER_STATE_UNLOADED;
            driver_count--;

            kprintf("[DRV] Unregistered driver '%s'\n", name);
            return 0;
        }
        prev = drv;
        drv = drv->next;
    }

    return -1;
}

driver_t *driver_find(const char *name)
{
    driver_t *drv = driver_list;
    while (drv) {
        if (strcmp(drv->name, name) == 0)
            return drv;
        drv = drv->next;
    }
    return NULL;
}

driver_t *driver_find_by_type(driver_type_t type)
{
    driver_t *drv = driver_list;
    while (drv) {
        if (drv->type == type)
            return drv;
        drv = drv->next;
    }
    return NULL;
}

int driver_load(const char *name)
{
    driver_t *drv = driver_find(name);
    if (!drv)
        return -1;

    if (drv->state == DRIVER_STATE_ACTIVE)
        return 0;

    if (drv->ops && drv->ops->init) {
        int ret = drv->ops->init(drv);
        if (ret != 0) {
            drv->state = DRIVER_STATE_ERROR;
            kprintf("[DRV] ERROR: Failed to initialize driver '%s'\n", name);
            return ret;
        }
    }

    drv->state = DRIVER_STATE_ACTIVE;
    kprintf("[DRV] Driver '%s' loaded and active\n", name);
    return 0;
}

int driver_unload(const char *name)
{
    driver_t *drv = driver_find(name);
    if (!drv)
        return -1;

    if (drv->state != DRIVER_STATE_ACTIVE)
        return 0;

    if (drv->ops && drv->ops->cleanup)
        drv->ops->cleanup(drv);

    drv->state = DRIVER_STATE_LOADED;
    kprintf("[DRV] Driver '%s' unloaded\n", name);
    return 0;
}

void driver_list_all(void)
{
    kprintf("Registered drivers:\n");
    driver_t *drv = driver_list;
    while (drv) {
        const char *state_str;
        switch (drv->state) {
        case DRIVER_STATE_UNLOADED: state_str = "UNLOADED"; break;
        case DRIVER_STATE_LOADED:   state_str = "LOADED";   break;
        case DRIVER_STATE_ACTIVE:   state_str = "ACTIVE";   break;
        case DRIVER_STATE_ERROR:    state_str = "ERROR";    break;
        default:                    state_str = "UNKNOWN";  break;
        }
        kprintf("  %-16s type=%d state=%s\n", drv->name, drv->type, state_str);
        drv = drv->next;
    }
}
