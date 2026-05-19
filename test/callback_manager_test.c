#include <assert.h>
#include <stdio.h>
#include <callbacks/callback_manager.h>

typedef struct {
    int value;
} event_t;

static int counter = 0;

int count_callback(const void *event, void *context) {
    const event_t *evt = (const event_t *)event;
    int *ctr = (int *)context;
    *ctr += evt->value;
    return 0;
}

bool even_filter(const void *event, void *context) {
    (void)context;
    const event_t *evt = (const event_t *)event;
    return (evt->value % 2) == 0;
}

int main(void) {
    callback_manager_t *manager = cb_manager_create(4);
    assert(manager != NULL);
    assert(cb_manager_get_count(manager) == 0);

    callback_config_t cfg1 = {
        .callback = count_callback,
        .filter = NULL,
        .context = &counter,
    };

    callback_config_t cfg2 = {
        .callback = count_callback,
        .filter = even_filter,
        .context = &counter,
    };

    int id1 = cb_manager_add_callback(manager, &cfg1);
    assert(id1 >= 0);

    int id2 = cb_manager_add_callback(manager, &cfg2);
    assert(id2 >= 0);
    assert(cb_manager_get_count(manager) == 2);

    event_t event = { .value = 3 };
    counter = 0;
    int fired = cb_manager_trigger(manager, &event);
    assert(fired == 1);
    assert(counter == 3);

    event.value = 4;
    fired = cb_manager_trigger(manager, &event);
    assert(fired == 2);
    assert(counter == 3 + 8);

    assert(cb_manager_remove_callback(manager, id1) == 0);
    assert(cb_manager_get_count(manager) == 1);

    event.value = 4;
    fired = cb_manager_trigger(manager, &event);
    assert(fired == 1);
    assert(counter == 3 + 8 + 4);

    assert(cb_manager_clear_all(manager) == 0);
    assert(cb_manager_get_count(manager) == 0);
    fired = cb_manager_trigger(manager, &event);
    assert(fired == 0);

    assert(cb_manager_destroy(manager) == 0);

    /* Global default manager wrappers */
    assert(cb_manager_init() == 0);
    assert(cb_has_callbacks() == false);

    counter = 0;
    callback_config_t cfg3 = {
        .callback = count_callback,
        .filter = NULL,
        .context = &counter,
    };
    int id3 = cb_register(&cfg3);
    assert(id3 >= 0);
    assert(cb_get_count() == 1);

    event.value = 2;
    fired = cb_trigger(&event);
    assert(fired == 1);
    assert(counter == 2);

    assert(cb_unregister(id3) == 0);
    assert(cb_get_count() == 0);
    assert(cb_manager_cleanup() == 0);

    printf("All callback manager tests passed.\n");
    return 0;
}
