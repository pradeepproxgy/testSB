#ifndef _SEQUEUE_H_
#define _SEQUEUE_H_

#include "fibo_opencpu.h"
#include "fb_config.h"
#include "voice_play.h"




#define 	maxsize		TTS_BUF_MAX
typedef enum
{
	QUEUE_IS_FULL=-1,	
 	QUEUE_IS_EMPTY=0,
	QUEUE_IS_SUCCESS=1,
}queue_enum;

typedef AudioPlayInfo datatype;
typedef struct 
{
	datatype data[maxsize];
	int front;
	int rear;
	int len;
}sequeue;



void SetNull(sequeue *sq);
int Empty(sequeue *sq);
int GetFront(sequeue *sq, datatype *x);
int InQueue(sequeue *sq, datatype *x);
int DeQueue(sequeue *sq, datatype *x);
int Queue_len(sequeue *sq);

#endif

