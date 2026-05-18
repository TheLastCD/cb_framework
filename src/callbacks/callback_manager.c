#include <callbacks/callback_manager.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_CALLBACKS 64

/**
 * @struct callback_entry_t
 * @brief Internal structure to store a registered callback
 */
typedef struct {
    bool active;
    bdy_type message_type;
    bdr_ret response_type;
    callback_fn on_response;
    callback_filter_fn filter;
    void *context;
} callback_entry_t;

/**
 * @struct callback_registry_t
 * @brief Global callback registry
 */
typedef struct {
    callback_entry_t callbacks[MAX_CALLBACKS];
    int count;
    bool initialized;
} callback_registry_t;

static callback_registry_t registry = {0};

/**
 * Helper function to find an unused callback slot
 */
static int find_free_slot(void) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!registry.callbacks[i].active) {
            return i;
        }
    }
    return -1;
}

int cb_manager_init(void) {
    if (registry.initialized) {
        printf("[Callback Manager] Already initialized\n");
        return 0;
    }

    memset(&registry, 0, sizeof(registry));
    registry.initialized = true;
    registry.count = 0;

    printf("[Callback Manager] Initialized with max %d callbacks\n", MAX_CALLBACKS);
    return 0;
}

int cb_manager_cleanup(void) {
    if (!registry.initialized) {
        return -1;
    }

    cb_clear_all();
    registry.initialized = false;

    printf("[Callback Manager] Cleaned up\n");
    return 0;
}

int cb_register(const callback_config_t *config) {
    if (!registry.initialized) {
        printf("[Callback Manager] Not initialized\n");
        return -1;
    }

    if (!config || !config->on_response) {
        printf("[Callback Manager] Invalid configuration\n");
        return -1;
    }

    if (registry.count >= MAX_CALLBACKS) {
        printf("[Callback Manager] Callback registry full (%d)\n", MAX_CALLBACKS);
        return -1;
    }

    int slot = find_free_slot();
    if (slot < 0) {
        printf("[Callback Manager] No free slots available\n");
        return -1;
    }

    callback_entry_t *entry = &registry.callbacks[slot];
    entry->active = true;
    entry->message_type = config->message_type;
    entry->response_type = config->response_type;
    entry->on_response = config->on_response;
    entry->filter = config->filter;
    entry->context = config->context;

    registry.count++;

    printf("[Callback Manager] Registered callback %d (MsgType:%u, RespType:%u)\n",
           slot, config->message_type, config->response_type);

    return slot;
}

int cb_unregister(int callback_id) {
    if (!registry.initialized) {
        printf("[Callback Manager] Not initialized\n");
        return -1;
    }

    if (callback_id < 0 || callback_id >= MAX_CALLBACKS) {
        printf("[Callback Manager] Invalid callback ID: %d\n", callback_id);
        return -1;
    }

    if (!registry.callbacks[callback_id].active) {
        printf("[Callback Manager] Callback %d not active\n", callback_id);
        return -1;
    }

    registry.callbacks[callback_id].active = false;
    registry.count--;

    printf("[Callback Manager] Unregistered callback %d\n", callback_id);
    return 0;
}

int cb_trigger(const Msg *msg) {
    if (!registry.initialized) {
        printf("[Callback Manager] Not initialized\n");
        return -1;
    }

    if (!msg) {
        printf("[Callback Manager] Invalid message\n");
        return -1;
    }

    int triggered_count = 0;

    for (int i = 0; i < MAX_CALLBACKS; i++) {
        callback_entry_t *entry = &registry.callbacks[i];

        if (!entry->active) {
            continue;
        }

        // Check if message type matches
        if (entry->message_type != msg->bdy.BodyType) {
            continue;
        }

        // Check if response type matches
        if (entry->response_type != msg->bdy.ReturnType) {
            continue;
        }

        // Check filter if provided
        if (entry->filter != NULL) {
            if (!entry->filter(msg, entry->context)) {
                printf("[Callback Manager] Filter rejected callback %d\n", i);
                continue;
            }
        }

        // Execute callback
        printf("[Callback Manager] Triggering callback %d (MsgType:%u, RespType:%u)\n",
               i, msg->bdy.BodyType, msg->bdy.ReturnType);

        int result = entry->on_response(msg, entry->context);
        
        if (result == 0) {
            printf("[Callback Manager] Callback %d executed successfully\n", i);
            triggered_count++;
        } else {
            printf("[Callback Manager] Callback %d returned error: %d\n", i, result);
        }
    }

    return triggered_count;
}

int cb_get_count(void) {
    if (!registry.initialized) {
        return 0;
    }
    return registry.count;
}

int cb_clear_all(void) {
    if (!registry.initialized) {
        printf("[Callback Manager] Not initialized\n");
        return -1;
    }

    for (int i = 0; i < MAX_CALLBACKS; i++) {
        registry.callbacks[i].active = false;
    }

    registry.count = 0;

    printf("[Callback Manager] Cleared all callbacks\n");
    return 0;
}

bool cb_has_callbacks_for_type(bdy_type message_type) {
    if (!registry.initialized) {
        return false;
    }

    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (registry.callbacks[i].active && 
            registry.callbacks[i].message_type == message_type) {
            return true;
        }
    }

    return false;
}
