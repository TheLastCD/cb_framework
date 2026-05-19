#include <stdio.h>
#include <callbacks/callback_manager.h>

typedef struct {
    const char *type;
    int id;
} event_t;

int log_event(const void *event, void *context) {
    (void)context;
    const event_t *evt = (const event_t *)event;
    printf("Logged event: type=%s id=%d\n", evt->type, evt->id);
    return 0;
}

bool filter_id_even(const void *event, void *context) {
    (void)context;
    const event_t *evt = (const event_t *)event;
    return (evt->id % 2) == 0;
}

int main(void) {
    callback_manager_t *manager = cb_manager_create(16);
    if (!manager) {
        return 1;
    }

    callback_config_t config = {
        .callback = log_event,
        .filter = filter_id_even,
        .context = NULL,
    };

    int callback_id = cb_manager_add_callback(manager, &config);
    if (callback_id < 0) {
        cb_manager_destroy(manager);
        return 1;
    }

    event_t event = { .type = "SENSOR_UPDATE", .id = 5 };
    cb_manager_trigger(manager, &event); // filter blocks this event

    event.id = 6;
    cb_manager_trigger(manager, &event); // callback executes

    cb_manager_remove_callback(manager, callback_id);
    cb_manager_destroy(manager);
    return 0;
}
