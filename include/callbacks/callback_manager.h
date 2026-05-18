#ifndef CALLBACKS_CALLBACK_MANAGER_H
#define CALLBACKS_CALLBACK_MANAGER_H

#include <msg/msg.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @file callback_manager.h
 * @brief Callback framework for message responses
 * 
 * This module provides a customizable callback system that allows applications to:
 * - Register callback functions to be invoked when specific message types are received
 * - Trigger callbacks when responses arrive for sent messages
 * - Associate user data with callbacks for context passing
 * 
 * Example:
 *   - Send a PING message
 *   - Receiver responds with positive confirmation
 *   - Callback is triggered automatically
 */

/**
 * @typedef callback_fn
 * @brief Callback function signature
 * 
 * @param msg The received message that triggered the callback
 * @param context User-provided context data (can be NULL)
 * @return 0 on success, negative on failure
 */
typedef int (*callback_fn)(const Msg *msg, void *context);

/**
 * @typedef callback_filter_fn
 * @brief Filter function to determine if a callback should execute
 * 
 * Allows fine-grained control over when callbacks execute
 * 
 * @param msg The message to evaluate
 * @param context User-provided context data
 * @return true if callback should execute, false otherwise
 */
typedef bool (*callback_filter_fn)(const Msg *msg, void *context);

/**
 * @struct callback_config_t
 * @brief Configuration for a message callback
 */
typedef struct {
    bdy_type message_type;        /**< Message type to match (e.g., bdy_PING) */
    bdr_ret response_type;        /**< Response type to match (e.g., bdy_CONFIRM) */
    callback_fn on_response;      /**< Function to call when response is received */
    callback_filter_fn filter;    /**< Optional filter function (NULL = no filter) */
    void *context;                /**< User-provided context passed to callbacks */
} callback_config_t;

/**
 * Initialize the callback manager
 * Must be called before any other callback functions
 * 
 * @return 0 on success, -1 on failure
 */
int cb_manager_init(void);

/**
 * Cleanup the callback manager and free resources
 * 
 * @return 0 on success, -1 on failure
 */
int cb_manager_cleanup(void);

/**
 * Register a callback for a specific message type and response type
 * 
 * @param config Callback configuration
 * @return ID of the registered callback (>= 0) on success, -1 on failure
 * 
 * Example:
 *   callback_config_t config = {
 *       .message_type = bdy_PING,
 *       .response_type = bdy_CONFIRM,
 *       .on_response = my_ping_callback,
 *       .filter = NULL,
 *       .context = my_data
 *   };
 *   int callback_id = cb_register(&config);
 */
int cb_register(const callback_config_t *config);

/**
 * Unregister a previously registered callback
 * 
 * @param callback_id The ID returned from cb_register
 * @return 0 on success, -1 on failure
 */
int cb_unregister(int callback_id);

/**
 * Trigger all matching callbacks for a received message
 * Usually called internally by message processing code
 * 
 * @param msg The received message
 * @return Number of callbacks executed (0 or more), -1 on error
 */
int cb_trigger(const Msg *msg);

/**
 * Get the number of registered callbacks
 * 
 * @return Number of currently registered callbacks
 */
int cb_get_count(void);

/**
 * Clear all registered callbacks
 * 
 * @return 0 on success, -1 on failure
 */
int cb_clear_all(void);

/**
 * Query if a specific message type has callbacks
 * 
 * @param message_type The message type to check
 * @return true if callbacks exist for this type, false otherwise
 */
bool cb_has_callbacks_for_type(bdy_type message_type);

#endif // CALLBACKS_CALLBACK_MANAGER_H
