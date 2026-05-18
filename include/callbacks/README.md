# Callback Framework

## Overview

The Callback Framework is a customizable event handling system that allows you to automatically trigger functions when specific message types receive responses. It's designed to decouple message sending from response handling, making the code more modular and maintainable.

## Folder Structure

```
callbacks/
├── include/callbacks/
│   └── callback_manager.h      # Public API and data structures
├── src/callbacks/
│   └── callback_manager.c      # Implementation
├── test/callbacks/
│   └── test_callback_manager.c # Comprehensive test suite
└── examples/
    └── callback_example.c      # Real-world usage example
```

## Key Features

- **Flexible Registration**: Register callbacks for any combination of message type and response type
- **Filter Functions**: Use optional filter functions to further refine when callbacks execute
- **Context Data**: Pass arbitrary user data through the `context` parameter
- **Multiple Callbacks**: Register multiple callbacks for the same message type
- **Thread-Safe Queries**: Check callback status at runtime
- **Maximum 64 Callbacks**: Configurable limit to prevent excessive memory usage

## Quick Start

### 1. Initialize
```c
cb_manager_init();
```

### 2. Define Callback Function
```c
int my_callback(const Msg *msg, void *context) {
    // Handle the message
    return 0;
}
```

### 3. Register Callback
```c
callback_config_t config = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = my_callback,
    .filter = NULL,
    .context = NULL
};
int cb_id = cb_register(&config);
```

### 4. Trigger Callbacks
```c
int count = cb_trigger(&received_message);
```

### 5. Cleanup
```c
cb_unregister(cb_id);
cb_manager_cleanup();
```

## API Summary

### Core Functions

| Function | Purpose |
|----------|---------|
| `cb_manager_init()` | Initialize callback system |
| `cb_manager_cleanup()` | Cleanup and free resources |
| `cb_register()` | Register a new callback |
| `cb_unregister()` | Unregister a callback |
| `cb_trigger()` | Execute matching callbacks |

### Query Functions

| Function | Purpose |
| `cb_get_count()` | Get number of registered callbacks |
| `cb_has_callbacks_for_type()` | Check if type has callbacks |
| `cb_clear_all()` | Unregister all callbacks |

## Data Structures

### callback_config_t

```c
typedef struct {
    bdy_type message_type;        // Message type to match
    bdr_ret response_type;        // Response type to match
    callback_fn on_response;      // Function to call
    callback_filter_fn filter;    // Optional filter function
    void *context;                // User context data
} callback_config_t;
```

### callback_fn

```c
typedef int (*callback_fn)(const Msg *msg, void *context);
```

### callback_filter_fn

```c
typedef bool (*callback_filter_fn)(const Msg *msg, void *context);
```

## Usage Patterns

### Pattern 1: Simple Callback
```c
int my_handler(const Msg *msg, void *context) {
    printf("Message received: type=%u\n", msg->bdy.BodyType);
    return 0;
}

callback_config_t cfg = {
    .message_type = bdy_READ,
    .response_type = bdy_CONFIRM,
    .on_response = my_handler,
    .filter = NULL,
    .context = NULL
};
cb_register(&cfg);
```

### Pattern 2: With Context Data
```c
typedef struct { int count; } stats_t;

int handler_with_stats(const Msg *msg, void *context) {
    stats_t *stats = (stats_t *)context;
    stats->count++;
    return 0;
}

stats_t stats = {0};
callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = handler_with_stats,
    .filter = NULL,
    .context = &stats
};
cb_register(&cfg);
```

### Pattern 3: With Filter Function
```c
bool only_from_server(const Msg *msg, void *ctx) {
    return msg->hdr.Requester == 0x42;
}

callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = my_callback,
    .filter = only_from_server,
    .context = NULL
};
cb_register(&cfg);
```

### Pattern 4: Multiple Callbacks
```c
// Callback 1: Logging
int log_callback(const Msg *msg, void *ctx) {
    printf("Message logged\n");
    return 0;
}

// Callback 2: Metrics
int metrics_callback(const Msg *msg, void *ctx) {
    // Update metrics
    return 0;
}

// Register both
callback_config_t cfg1 = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = log_callback,
    .filter = NULL,
    .context = NULL
};

callback_config_t cfg2 = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = metrics_callback,
    .filter = NULL,
    .context = NULL
};

cb_register(&cfg1);
cb_register(&cfg2);
```

## Integration Points

### With Message Processing

In your message processing code, call `cb_trigger()` when messages are received:

```c
// In message handler
void process_message(const uint8_t *buffer, size_t size) {
    Msg msg = {0};
    
    if (decode_msg(&msg, buffer, size) > 0) {
        // Trigger callbacks for this message
        int callbacks_fired = cb_trigger(&msg);
        
        // Continue with normal processing
        // ...
        
        free_msg(&msg);
    }
}
```

### With Message Sending

You can register callbacks before sending messages:

```c
// Register callback
callback_config_t cfg = {...};
int cb_id = cb_register(&cfg);

// Send message
send_message(&msg);

// Later, when response arrives:
// cb_trigger(&response_msg) will automatically call your callback
```

## Testing

Run the callback tests:

```bash
./test_callbacks          # Run callback tests only
ctest                     # Run all tests including callbacks
```

For details, see [test_callback_manager.c](../test/callbacks/test_callback_manager.c)

## Examples

See [examples/callback_example.c](../../examples/callback_example.c) for a complete real-world example showing:
- Ping monitoring with statistics
- Filter functions to track server responses only
- Multiple callbacks (success and failure handlers)

## Best Practices

1. **Always initialize and cleanup**
   ```c
   cb_manager_init();
   // ... use callbacks ...
   cb_manager_cleanup();
   ```

2. **Check return values**
   ```c
   if (cb_register(&config) < 0) {
       printf("Failed to register callback\n");
   }
   ```

3. **Keep callbacks fast** - They execute synchronously

4. **Use filters for optimization** - Reduces unnecessary callback execution

5. **Use context for state** - Avoid global variables

6. **Unregister when done** - Frees up callback slots

## Limitations

- Maximum 64 simultaneous callbacks
- Callbacks execute synchronously (blocking)
- No built-in callback priorities
- No persistence (lost on restart)

## Performance

- Registration: O(1) time
- Unregistration: O(1) time
- Triggering: O(n) where n = number of registered callbacks
- Memory per callback: ~32 bytes

## Troubleshooting

### Callback not being triggered
- Check that `cb_manager_init()` was called
- Verify message type matches registered type
- Verify response type matches registered type
- If using a filter, check filter logic
- Use `cb_get_count()` to verify callback is registered

### Multiple callbacks executing
- This is normal - all matching callbacks will execute
- Use filters to narrow down execution scope

### "Registry full" error
- Maximum 64 callbacks reached
- Unregister unused callbacks or increase MAX_CALLBACKS
- Check for callback registration leaks

## Architecture

The callback system uses a simple registry-based design:

1. **Registration** - Callbacks stored in static array indexed by ID
2. **Triggering** - Linear search through registry to find matches
3. **Filtering** - Optional filter functions called before execution
4. **Context** - User data passed as void pointer

This design provides:
- Simple, predictable behavior
- O(1) registration/unregistration
- No dynamic memory allocation
- No hidden dependencies
