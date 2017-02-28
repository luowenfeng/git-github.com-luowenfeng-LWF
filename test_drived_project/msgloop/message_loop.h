#ifndef __MESSAGE_LOOP_H__
#define __MESSAGE_LOOP_H__

typedef struct stMessageLoop    MessageLoop;
typedef struct stLoopTimer      LoopTimer;
typedef void(*Message_Handle)(int what, void* obj);

MessageLoop* MessageLoop_Create(Message_Handle handle);
void MessageLoop_Destroy(MessageLoop *loop);
void MessageLoop_AppendMessage(MessageLoop *loop, int what, void *obj, int flush);

LoopTimer* MessageLoop_AppendTimer(MessageLoop *loop, Message_Handle handle, int what, void* obj, int interval_ms, int once);
void MessageLoop_RemoveTimer(LoopTimer* timer);

#endif // __MESSAGE_LOOP_H__
