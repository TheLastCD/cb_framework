#ifndef CALLBACKS_CALLBACK_MANAGER_H
#define CALLBACKS_CALLBACK_MANAGER_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file callback_manager.h
 * @brief Generic callback manager for application events
 *
 * This module provides a reusable callback system suitable for integration into
 * different projects. Callbacks are registered against a manager instance and
 * triggered with a generic event pointer. The user can implement custom filters
 * and context management without depending on a specific message type.
 */

/**
 * @typedef callback_fn
 * @brief Callback function signature
 *
 * @param event The generic event or message payload that triggered the callback
 * @param context User-provided context data (can be NULL)
 * @return 0 on success, negative on failure
 */
typedef int (*callback_fn)(const void *event, void *context);

/**
 * @typedef callback_filter_fn
 * @brief Filter used to decide if a callback should execute
 *
 * @param event The generic event or message payload
 * @param context User-provided context data
 * @return true if callback should execute, false otherwise
 */
typedef bool (*callback_filter_fn)(const void *event, void *context);

/**
 * @struct callback_config_t
 * @brief Configuration for a callback registration
 */
typedef struct {
    callback_fn callback;          /**< Function to execute when event is triggered */
    callback_filter_fn filter;     /**< Optional filter to gate callback execution */
    void *context;                 /**< User-provided context passed to callback/filter */
} callback_config_t;

/**
 * @struct callback_manager_t
 * @brief Opaque callback manager handle
 */
typedef struct callback_manager_t callback_manager_t;

/**
 * Create a callback manager instance.
 *
 * @param max_callbacks Maximum number of callbacks to support.
 * @return Pointer to manager on success, NULL on failure.
 */
callback_manager_t *cb_manager_create(size_t max_callbacks);

/**
 * Destroy a callback manager and free associated resources.
 *
 * @param manager Manager pointer returned from cb_manager_create.
 * @return 0 on success, -1 on failure.
 */
int cb_manager_destroy(callback_manager_t *manager);

/**
 * Register a callback with a callback manager.
 *
 * @param manager Manager instance.
 * @param config Callback registration configuration.
 * @return Callback ID (>= 0) on success, -1 on failure.
 */
int cb_manager_add_callback(callback_manager_t *manager, const callback_config_t *config);

/**
 * Unregister a callback from a manager.
 *
 * @param manager Manager instance.
 * @param callback_id Callback ID returned from cb_manager_add_callback.
 * @return 0 on success, -1 on failure.
 */
int cb_manager_remove_callback(callback_manager_t *manager, int callback_id);

/**
 * Trigger callbacks for a given event.
 *
 * @param manager Manager instance.
 * @param event Generic event pointer passed to callbacks.
 * @return Number of callbacks executed, or -1 on error.
 */
int cb_manager_trigger(callback_manager_t *manager, const void *event);

/**
 * Get the number of registered callbacks.
 *
 * @param manager Manager instance.
 * @return Number of active callbacks, or 0 if manager is NULL.
 */
size_t cb_manager_get_count(const callback_manager_t *manager);

/**
 * Unregister all callbacks from a manager.
 *
 * @param manager Manager instance.
 * @return 0 on success, -1 on failure.
 */
int cb_manager_clear_all(callback_manager_t *manager);

/**
 * Check whether the manager has any callbacks registered.
 *
 * @param manager Manager instance.
 * @return true if any callbacks exist, false otherwise.
 */
bool cb_manager_has_callbacks(const callback_manager_t *manager);

/**
 * @name Global default manager wrappers
 *
 * These wrappers maintain the original function names while forwarding to a
 * default manager instance. Use the manager-based API directly for reusable
 * integration in other projects.
 */
/*@{*/

#define CB_DEFAULT_MANAGER_CAPACITY 64

int cb_manager_init(void);
int cb_manager_cleanup(void);
int cb_register(const callback_config_t *config);
int cb_unregister(int callback_id);
int cb_trigger(const void *event);
size_t cb_get_count(void);
int cb_clear_all(void);
bool cb_has_callbacks(void);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif // CALLBACKS_CALLBACK_MANAGER_H
