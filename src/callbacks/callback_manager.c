#include <callbacks/callback_manager.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct callback_entry_t {
    bool active;
    callback_fn callback;
    callback_filter_fn filter;
    void *context;
};

struct callback_manager_t {
    struct callback_entry_t *callbacks;
    size_t capacity;
    size_t count;
};

static callback_manager_t *g_default_manager = NULL;

static int find_free_slot(callback_manager_t *manager) {
    if (!manager) {
        return -1;
    }

    for (size_t i = 0; i < manager->capacity; i++) {
        if (!manager->callbacks[i].active) {
            return (int)i;
        }
    }

    return -1;
}

callback_manager_t *cb_manager_create(size_t max_callbacks) {
    if (max_callbacks == 0) {
        return NULL;
    }

    callback_manager_t *manager = (callback_manager_t *)malloc(sizeof(callback_manager_t));
    if (!manager) {
        return NULL;
    }

    manager->callbacks = (struct callback_entry_t *)calloc(max_callbacks, sizeof(struct callback_entry_t));
    if (!manager->callbacks) {
        free(manager);
        return NULL;
    }

    manager->capacity = max_callbacks;
    manager->count = 0;
    return manager;
}

int cb_manager_destroy(callback_manager_t *manager) {
    if (!manager) {
        return -1;
    }

    free(manager->callbacks);
    free(manager);
    return 0;
}

int cb_manager_add_callback(callback_manager_t *manager, const callback_config_t *config) {
    if (!manager || !config || !config->callback) {
        return -1;
    }

    if (manager->count >= manager->capacity) {
        return -1;
    }

    int slot = find_free_slot(manager);
    if (slot < 0) {
        return -1;
    }

    struct callback_entry_t *entry = &manager->callbacks[slot];
    entry->active = true;
    entry->callback = config->callback;
    entry->filter = config->filter;
    entry->context = config->context;

    manager->count++;
    return slot;
}

int cb_manager_remove_callback(callback_manager_t *manager, int callback_id) {
    if (!manager || callback_id < 0 || (size_t)callback_id >= manager->capacity) {
        return -1;
    }

    struct callback_entry_t *entry = &manager->callbacks[callback_id];
    if (!entry->active) {
        return -1;
    }

    entry->active = false;
    manager->count--;
    return 0;
}

int cb_manager_trigger(callback_manager_t *manager, const void *event) {
    if (!manager || !event) {
        return -1;
    }

    int executed_count = 0;

    for (size_t i = 0; i < manager->capacity; i++) {
        struct callback_entry_t *entry = &manager->callbacks[i];
        if (!entry->active) {
            continue;
        }

        if (entry->filter && !entry->filter(event, entry->context)) {
            continue;
        }

        int result = entry->callback(event, entry->context);
        if (result >= 0) {
            executed_count++;
        }
    }

    return executed_count;
}

size_t cb_manager_get_count(const callback_manager_t *manager) {
    return manager ? manager->count : 0;
}

int cb_manager_clear_all(callback_manager_t *manager) {
    if (!manager) {
        return -1;
    }

    for (size_t i = 0; i < manager->capacity; i++) {
        manager->callbacks[i].active = false;
    }

    manager->count = 0;
    return 0;
}

bool cb_manager_has_callbacks(const callback_manager_t *manager) {
    return manager ? manager->count > 0 : false;
}

int cb_manager_init(void) {
    if (g_default_manager) {
        return 0;
    }

    g_default_manager = cb_manager_create(CB_DEFAULT_MANAGER_CAPACITY);
    return g_default_manager ? 0 : -1;
}

int cb_manager_cleanup(void) {
    if (!g_default_manager) {
        return -1;
    }

    int result = cb_manager_destroy(g_default_manager);
    g_default_manager = NULL;
    return result;
}

int cb_register(const callback_config_t *config) {
    if (!g_default_manager) {
        return -1;
    }
    return cb_manager_add_callback(g_default_manager, config);
}

int cb_unregister(int callback_id) {
    if (!g_default_manager) {
        return -1;
    }
    return cb_manager_remove_callback(g_default_manager, callback_id);
}

int cb_trigger(const void *event) {
    if (!g_default_manager) {
        return -1;
    }
    return cb_manager_trigger(g_default_manager, event);
}

size_t cb_get_count(void) {
    return cb_manager_get_count(g_default_manager);
}

int cb_clear_all(void) {
    if (!g_default_manager) {
        return -1;
    }
    return cb_manager_clear_all(g_default_manager);
}

bool cb_has_callbacks(void) {
    return cb_manager_has_callbacks(g_default_manager);
}
