#include <unistd.h>
#include <stdio.h>
#include "message_loop.h"

void handle_message(int what, void *obj) {
    static int index = 0;
    int *k = (int*)obj;
    printf("[%8d]handle_message what:%d, obj:%d\n", index++, what, *k);
}

int main(int argc, char *argv[])
{
    MessageLoop *loop = MessageLoop_Create(handle_message);
    LoopTimer *timer1, *timer2, *timer3;
    int i, v, t = 300, p=600, q=900;
    if (loop) {
        v = 0;
        timer1 = MessageLoop_AppendTimer(loop, handle_message, 111, &t, 2, 0);
        timer2 = MessageLoop_AppendTimer(loop, handle_message, 222, &p, 3, 0);
        timer3 = MessageLoop_AppendTimer(loop, handle_message, 333, &q, 5, 0);
        usleep(2000*1000);

        for (i=0; i<100; i++) {
            MessageLoop_AppendMessage(loop, v, &i, 0);
            usleep(500);
        }
        MessageLoop_RemoveTimer(timer1);
        MessageLoop_RemoveTimer(timer2);
        timer1 = MessageLoop_AppendTimer(loop, handle_message, 444, &t, 2, 0);

        v = 1;
        for (i=0; i<100; i++) {
            MessageLoop_AppendMessage(loop, v, &i, 1);
            usleep(2000);
        }

        MessageLoop_Destroy(loop);
    }
    return 0;
}
