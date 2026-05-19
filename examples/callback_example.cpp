#include <cstdio>
#include <callbacks/callback_manager.h>

struct Event {
    const char *type;
    int id;
};

int cpp_callback(const void *event, void *context) {
    (void)context;
    const Event *evt = static_cast<const Event *>(event);
    std::printf("C++ callback event type=%s id=%d\n", evt->type, evt->id);
    return 0;
}

bool cpp_filter(const void *event, void *context) {
    (void)context;
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
