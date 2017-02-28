#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>

#include "cirular_list.h"
#include "message_loop.h"

#define MAX_MESSAGE_COUNT 1024
#define MAX_TIMER_COUNT 256

typedef struct stMessage        Message;
struct stMessage {
    int what;
    void *obj;

    int needs_to_be_done;
};

struct stLoopTimer {
    int what;
    void *obj;
    Message_Handle handle;

    unsigned int next_ms;
    unsigned int interval_ms;
    int once;

    int active;

    LoopTimer *prev;
    LoopTimer *next;

    MessageLoop *loop;
};

struct stMessageLoop {
    char name[256];

    CircularList *cList;
    CircularListNode *read;
    CircularListNode *write;

    pthread_t thread;
    Message_Handle handle;

    int quit;
    int flush;


    LoopTimer timers[MAX_TIMER_COUNT];
    int timer_idx;

    LoopTimer *timer_head;
    LoopTimer *timer_rear;
};

static unsigned int gettime_ms() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

static void* message_loop_func(void *info) {

    MessageLoop *loop = (MessageLoop*)info;
    Message *msg;
    LoopTimer *timer;
    while (1) {
        while (1) {
            msg = (Message *)loop->read->data;
            if (!msg->needs_to_be_done)
                break;

            loop->handle(msg->what, msg->obj);
            msg->needs_to_be_done = 0;
            if (loop->quit) break;

            loop->read = loop->read->next;
        }
        if (loop->quit) break;

        timer = loop->timer_head;
        while (timer) {
            if (timer->active && gettime_ms() >= timer->next_ms) {
                timer->handle(timer->what, timer->obj);
                if (loop->quit) break;

                if (timer->once) {
                    timer->active = 0;
                }
                else {
                    timer->next_ms = gettime_ms() + timer->interval_ms;
                }
            }
            timer = timer->next;
        }

        if (loop->quit) break;
        usleep(1000);
    }
    return NULL;
}

MessageLoop* MessageLoop_Create(Message_Handle handle) {
    MessageLoop *loop = NULL;
    int ok = 0, ret;
    pthread_attr_t attr;

    do {
        loop = (MessageLoop*)malloc(sizeof(MessageLoop));
        if (!loop) break;

        memset(loop, 0, sizeof(MessageLoop));
        loop->cList = CircularList_Create(MAX_MESSAGE_COUNT, sizeof(Message));
        if (!loop->cList) {
            printf("CircularList_Create failed in %s", __FUNCTION__);
            break;
        }

        loop->read   = loop->cList->nodes;
        loop->write  = loop->cList->nodes;
        loop->handle = handle;
        loop->quit   = 0;

        pthread_attr_init(&attr);
        ret = pthread_create(&(loop->thread), &attr, message_loop_func, loop);
        if (ret != 0) {
            printf("pthread_create failed in %s", __FUNCTION__);
            break;
        }

        ok = 1;
    } while(0);

    if (!ok) {
        MessageLoop_Destroy(loop);
        loop = NULL;
    }

    return loop;
}

void MessageLoop_Destroy(MessageLoop *loop) {
    if (loop) {
        loop->quit = 1;
        if (loop->thread)
            pthread_join(loop->thread, NULL);

        if (loop->cList)
            CircularList_Destroy(loop->cList);

        free(loop);
    }
}

void MessageLoop_AppendMessage(MessageLoop *loop, int what, void *obj, int flush) {
    Message *msg;
    if (!loop) return;

    msg = (Message*)loop->write->data;
    msg->needs_to_be_done = 1;
    msg->what = what;
    msg->obj = obj;
    if (flush) {
        CircularListNode *read = loop->read;
        while (read != loop->write) {
            msg = (Message*)read->data;
            msg->needs_to_be_done = 0;
            read = read->next;
        }
        loop->read = loop->write;
    }

    loop->write = loop->write->next;
}

LoopTimer* MessageLoop_AppendTimer(MessageLoop *loop, Message_Handle handle, int what, void *obj, int interval_ms, int once) {
    int i, idx;
    LoopTimer* timer = NULL;
    if (!loop) return NULL;

    for (i=0, idx=loop->timer_idx; i<MAX_TIMER_COUNT; i++, idx++) {
        if (!loop->timers[idx].active) {
            timer = loop->timers + idx;
            break;
        }
    }

    if (!timer) return NULL;

    timer->loop         = loop;
    timer->active       = 1;
    timer->once         = once;
    timer->interval_ms  = interval_ms;
    timer->handle       = handle;
    timer->what         = what;
    timer->obj          = obj;
    timer->next_ms      = gettime_ms() + interval_ms;

    if (loop->timer_head == NULL) {
        loop->timer_head = timer;
        loop->timer_rear = timer;
    }
    else {
        loop->timer_rear->next = timer;
        timer->prev = loop->timer_rear;
        loop->timer_rear = timer;
    }

    return timer;
}

void MessageLoop_RemoveTimer(LoopTimer* timer) {
    if (timer) {
        timer->active = 0;
        if (timer == timer->loop->timer_head) {
            timer->loop->timer_head = timer->loop->timer_head->next;
        }

        if (timer->prev) {
            timer->prev->next = timer->next;
            timer->next = NULL;
        }

        if (timer->next) {
            timer->next->prev = timer->prev;
            timer->prev = NULL;
        }
    }
}



