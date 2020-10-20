//
// Created by cao on 2020/5/28.
//

#include <event.h>
#include <signal.h>

void signal_cb(int fd, short event, void* argc) {
    event_base* base = (event_base*) argc;
    timeval delay = {2, 0};
    printf("caught interrupt signal, exiting cleanly in two seconds\n");
    event_base_loopexit(base, &delay);
}

void timeout_cb(int fd, short event, void* argc){
    printf("timeout \n");
}



int main(int argc, char* argv[]) {

    event_base* base = event_init();
    event* signal_event = evsignal_new(base, SIGINT, signal_cb, base);
    event_add(signal_event, NULL);

    timeval tv = { 1, 0 };
    event* timeout_event = evtimer_new(base, timeout_cb, base);
    event_add(timeout_event, &tv);

    event_new()

    event_base_dispatch(base);
    event_free(timeout_event);
    event_free(signal_event);
    event_base_free(base);

    return 0;
}