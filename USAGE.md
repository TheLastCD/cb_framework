# Callback Manager Usage

This document explains how to use the generic callback manager provided by `include/callbacks/callback_manager.h`.

## Build

From the project root:

```bash
make
```

This produces:

- `build/callback_manager.o`

## API Overview

The callback manager provides a reusable instance-based API and a default global manager wrapper for quick integration.

### Key types

- `callback_manager_t` - opaque manager handle
- `callback_fn` - callback function signature
- `callback_filter_fn` - optional filter function signature
- `callback_config_t` - registration configuration

### Core API

- `callback_manager_t *cb_manager_create(size_t max_callbacks);`
- `int cb_manager_destroy(callback_manager_t *manager);`
- `int cb_manager_add_callback(callback_manager_t *manager, const callback_config_t *config);`
- `int cb_manager_remove_callback(callback_manager_t *manager, int callback_id);`
- `int cb_manager_trigger(callback_manager_t *manager, const void *event);`
- `size_t cb_manager_get_count(const callback_manager_t *manager);`
- `int cb_manager_clear_all(callback_manager_t *manager);`
- `bool cb_manager_has_callbacks(const callback_manager_t *manager);`

### Default global manager wrappers

For simple cases, the library also exposes a global default manager via wrappers:

- `cb_manager_init()`
- `cb_manager_cleanup()`
- `cb_register()`
- `cb_unregister()`
- `cb_trigger()`
- `cb_get_count()`
- `cb_clear_all()`
- `cb_has_callbacks()`

These wrappers operate on a single default manager created with `CB_DEFAULT_MANAGER_CAPACITY`.

## Example usage

### C example

```c
#include <callbacks/callback_manager.h>
#include <stdio.h>

int my_callback(const void *event, void *context) {
    const char *message = (const char *)event;
    printf("Callback received event: %s\n", message);
    return 0;
}

bool my_filter(const void *event, void *context) {
    const char *message = (const char *)event;
    return message && message[0] != '\0';
}

int main(void) {
    callback_manager_t *manager = cb_manager_create(32);
    if (!manager) {
        return 1;
    }

    callback_config_t config = {
        .callback = my_callback,
        .filter = my_filter,
        .context = NULL,
    };

    int callback_id = cb_manager_add_callback(manager, &config);
    if (callback_id < 0) {
        cb_manager_destroy(manager);
        return 1;
    }

    const char *event = "hello world";
    int fired = cb_manager_trigger(manager, event);
    printf("Callbacks executed: %d\n", fired);

    cb_manager_remove_callback(manager, callback_id);
    cb_manager_destroy(manager);
    return 0;
}
```

### Typed C event example

```c
typedef struct {
    const char *type;
    int id;
} event_t;

int log_event(const void *event, void *context) {
    const event_t *evt = (const event_t *)event;
    printf("Event type=%s id=%d\n", evt->type, evt->id);
    return 0;
}

bool only_even_id(const void *event, void *context) {
    const event_t *evt = (const event_t *)event;
    return (evt->id % 2) == 0;
}

callback_config_t event_cfg = {
    .callback = log_event,
    .filter = only_even_id,
    .context = NULL,
};
int event_cb = cb_manager_add_callback(manager, &event_cfg);

event_t evt = { .type = "SENSOR_UPDATE", .id = 4 };
cb_manager_trigger(manager, &evt);
```

### C++ example

```cpp
#include <cstdio>
#include <callbacks/callback_manager.h>

struct Event {
    const char *type;
    int id;
};

int cpp_callback(const void *event, void *context) {
    const Event *evt = static_cast<const Event *>(event);
    std::printf("C++ callback event type=%s id=%d\n", evt->type, evt->id);
    return 0;
}

bool cpp_filter(const void *event, void *context) {
    const Event *evt = static_cast<const Event *>(event);
    return (evt->id % 2) == 1;
}

int main() {
    callback_manager_t *manager = cb_manager_create(16);
    if (!manager) {
        return 1;
    }

    callback_config_t config = {
        .callback = cpp_callback,
        .filter = cpp_filter,
        .context = nullptr,
    };
    int id = cb_manager_add_callback(manager, &config);
    if (id < 0) {
        cb_manager_destroy(manager);
        return 1;
    }

    Event event = {"CPLUSPLUS_EVENT", 7};
    cb_manager_trigger(manager, &event);

    cb_manager_remove_callback(manager, id);
    cb_manager_destroy(manager);
    return 0;
}
```

### 5. Unregister and cleanup

```c
cb_manager_remove_callback(manager, callback_id);
cb_manager_destroy(manager);
```

## Using the default manager

```c
cb_manager_init();

callback_config_t config = {
    .callback = my_callback,
    .filter = my_filter,
    .context = NULL,
};
int callback_id = cb_register(&config);
if (callback_id < 0) {
    cb_manager_cleanup();
    return -1;
}

cb_trigger("hello world");
cb_unregister(callback_id);
cb_manager_cleanup();
```

## Integration notes

- `event` is a generic `const void *` pointer. Cast it to your application-specific type inside callbacks and filters.
- `context` is user-defined state passed to both callback and filter.
- Callbacks execute synchronously in registration order.
- Filters run before callbacks and must return `true` to allow execution.
- `cb_manager_trigger()` counts callbacks as executed when they return a non-negative value.

## Best practices

- Keep callback execution fast.
- Use `context` rather than global variables.
- Unregister callbacks when they are no longer needed.
- Use separate manager instances for isolated subsystems.

## Error handling

- `cb_manager_create()` returns `NULL` on failure.
- `cb_manager_add_callback()` returns `-1` on invalid arguments or full capacity.
- `cb_manager_trigger()` returns `-1` for invalid manager or event pointers.
- `cb_manager_remove_callback()` returns `-1` for invalid callback IDs.

## Running tests

Use the provided make target:

```bash
make test
```

This will build and run `build/test_callback_manager`.
