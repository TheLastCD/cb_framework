# Quick Start: Attaching Custom Callbacks

## TL;DR - 5 Steps

### Step 1: Include Headers
```c
#include <callbacks/callback_manager.h>
#include <msg/msg.h>
```

### Step 2: Create Callback Function
```c
int my_handler(const Msg *msg, void *context) {
    printf("Response from 0x%02X: type=%u\n", 
           msg->hdr.Requester, msg->bdy.BodyType);
    return 0;  // Return 0 on success
}
```

### Step 3: Initialize
```c
int main(void) {
    cb_manager_init();
    // ... rest of code ...
    cb_manager_cleanup();
}
```

### Step 4: Register Callback
```c
callback_config_t config = {
    .message_type = bdy_PING,          // Message type to match
    .response_type = bdy_CONFIRM,      // Response type to match
    .on_response = my_handler,         // Your function
    .filter = NULL,                    // No filter
    .context = NULL                    // No context data
};

int callback_id = cb_register(&config);
if (callback_id < 0) {
    printf("Failed to register\n");
}
```

### Step 5: Trigger on Message Reception
```c
// When you receive a message
Msg received_msg = {0};
if (decode_msg(&received_msg, buffer, buffer_size) > 0) {
    // Trigger callbacks
    int count = cb_trigger(&received_msg);
    printf("Fired %d callbacks\n", count);
    
    free_msg(&received_msg);
}
```

## Common Patterns

### Pattern A: Simple Logging
```c
int log_response(const Msg *msg, void *ctx) {
    printf("[LOG] Message from 0x%02X\n", msg->hdr.Requester);
    return 0;
}

callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = log_response,
    .filter = NULL,
    .context = NULL
};

cb_register(&cfg);
```

### Pattern B: With Statistics
```c
typedef struct {
    int count;
    int total;
} stats_t;

int update_stats(const Msg *msg, void *ctx) {
    stats_t *stats = (stats_t *)ctx;
    stats->count++;
    printf("Success rate: %d/%d\n", stats->count, stats->total);
    return 0;
}

stats_t my_stats = {0, 10};

callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = update_stats,
    .filter = NULL,
    .context = &my_stats
};

cb_register(&cfg);
```

### Pattern C: With Filter
```c
// Only process messages from server
bool from_server(const Msg *msg, void *ctx) {
    return msg->hdr.Requester == 0x42;
}

int server_handler(const Msg *msg, void *ctx) {
    printf("Server response received\n");
    return 0;
}

callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = server_handler,
    .filter = from_server,  // Add filter
    .context = NULL
};

cb_register(&cfg);
```

### Pattern D: Multiple Handlers
```c
// Handler 1: Logging
int log_handler(const Msg *msg, void *ctx) {
    printf("Response received\n");
    return 0;
}

// Handler 2: Metrics
int metrics_handler(const Msg *msg, void *ctx) {
    // Record metrics
    return 0;
}

// Register both
callback_config_t cfg1 = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = log_handler,
    .filter = NULL,
    .context = NULL
};

callback_config_t cfg2 = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = metrics_handler,
    .filter = NULL,
    .context = NULL
};

int id1 = cb_register(&cfg1);
int id2 = cb_register(&cfg2);
// Both execute when message arrives
```

### Pattern E: Combined Filter + Context
```c
typedef struct {
    int success;
    int fail;
} server_stats_t;

// Filter: Only server (0x42)
bool from_server(const Msg *msg, void *ctx) {
    return msg->hdr.Requester == 0x42;
}

// Handler: Track stats
int track_stats(const Msg *msg, void *ctx) {
    server_stats_t *stats = (server_stats_t *)ctx;
    
    if (msg->bdy.ReturnType == bdy_CONFIRM) {
        stats->success++;
    } else {
        stats->fail++;
    }
    
    printf("Server stats: %d OK, %d FAIL\n", 
           stats->success, stats->fail);
    return 0;
}

server_stats_t server_stats = {0};

callback_config_t cfg = {
    .message_type = bdy_PING,
    .response_type = bdy_CONFIRM,
    .on_response = track_stats,
    .filter = from_server,
    .context = &server_stats
};

cb_register(&cfg);

// Send ping, receive response, callback fires automatically!
```

## Message Types

Available message types to filter on:
```c
bdy_NONE      // No operation
bdy_READ      // Read request
bdy_WRITE     // Write request
bdy_COPY      // Copy request
bdy_PING      // Ping/health check
```

## Response Types

Available response types:
```c
bdy_IGNORE    // No response needed
bdy_CONFIRM   // Positive confirmation
bdy_MSG       // Message response
```

## Unregister / Cleanup

### Remove Single Callback
```c
int callback_id = cb_register(&config);
// ... later ...
cb_unregister(callback_id);
```

### Remove All Callbacks
```c
cb_clear_all();
```

### Proper Cleanup
```c
int main(void) {
    cb_manager_init();
    
    int id = cb_register(&config);
    // ... use callbacks ...
    
    cb_unregister(id);
    cb_manager_cleanup();
    return 0;
}
```

## Error Handling

```c
int callback_id = cb_register(&config);
if (callback_id < 0) {
    printf("Registration failed\n");
    return 1;
}

// Check count
if (cb_get_count() >= 64) {
    printf("Too many callbacks\n");
}

// Check if type has callbacks
if (cb_has_callbacks_for_type(bdy_PING)) {
    printf("PING callbacks registered\n");
}
```

## Complete Example

```c
#include <callbacks/callback_manager.h>
#include <msg/msg.h>
#include <stdio.h>

// Callback function
int on_response(const Msg *msg, void *ctx) {
    printf("Response from 0x%02X\n", msg->hdr.Requester);
    return 0;
}

int main(void) {
    // Initialize
    cb_manager_init();
    
    // Register
    callback_config_t cfg = {
        .message_type = bdy_PING,
        .response_type = bdy_CONFIRM,
        .on_response = on_response,
        .filter = NULL,
        .context = NULL
    };
    
    int cb_id = cb_register(&cfg);
    
    // Simulate message reception
    Msg msg = {0};
    msg.hdr.Requester = 0x42;
    msg.bdy.BodyType = bdy_PING;
    msg.bdy.ReturnType = bdy_CONFIRM;
    msg.msg_buff = NULL;
    
    cb_trigger(&msg);  // Callback fires!
    
    // Cleanup
    cb_unregister(cb_id);
    cb_manager_cleanup();
    
    return 0;
}
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Callback not firing | Check init was called, verify types match |
| Multiple firing | Intentional - all matching callbacks execute |
| No response | Ensure decode_msg succeeded before cb_trigger |
| Memory warning | Max 64 callbacks - unregister unused ones |

## API Reference (Quick)

| Function | Use |
|----------|-----|
| `cb_manager_init()` | Start: call first |
| `cb_register()` | Add callback |
| `cb_trigger()` | Execute matching |
| `cb_unregister()` | Remove callback |
| `cb_get_count()` | Check count |
| `cb_clear_all()` | Remove all |
| `cb_manager_cleanup()` | End: call last |

## See Also

- Full documentation: [CallbackFramework.md](../docs/CallbackFramework.md)
- Architecture: [include/callbacks/README.md](callbacks/README.md)
- Example: [examples/callback_example.c](../examples/callback_example.c)
- Tests: [test/callbacks/test_callback_manager.c](../test/callbacks/test_callback_manager.c)
