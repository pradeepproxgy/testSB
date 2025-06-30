#include "voice_play.h"
#include "sequeue.h"
#include <string.h>
#include "fibo_opencpu.h"


//置空队
void SetNull(sequeue *sq)
{
	fibo_textTrace("fn: %s line:SetNUll entered %d\n", __func__, __LINE__);
	sq->front = 0;
	sq->rear = 0;	
} 

//判队空  0非空、1空
int Empty(sequeue *sq)
{
	if(sq->rear == sq->front)
	{
		return QUEUE_IS_EMPTY;
	}
	else
	{
		return QUEUE_IS_SUCCESS;
	}
} 

//取头结点
int GetFront(sequeue *sq, datatype *x)
{
	if(QUEUE_IS_EMPTY==Empty(sq))
	{
		return QUEUE_IS_EMPTY;
	}
	else
	{
		x->play_state = sq->data[(sq->front+1) % maxsize].play_state;
		x->play_type = sq->data[(sq->front+1) % maxsize].play_type;
		strcpy(x->play_text,sq->data[(sq->front+1) % maxsize].play_text);

		return QUEUE_IS_SUCCESS;
	}
} 


//入队(0失败、1成功)
int InQueue(sequeue *sq, datatype *x) 
{
	fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
	if((sq->rear+1) % maxsize == sq->front)
	{
		fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
		return QUEUE_IS_FULL; 
	}
	else
	{
		sq->rear = (sq->rear+1) % maxsize;
//		memset(sq->data[sq->rear].play_text,0,sizeof(sq->data[sq->rear]));
		//strcpy(sq->data[sq->rear].play_text,x->play_text);
		memcpy(sq->data[sq->rear].play_text,x->play_text,strlen(x->play_text));
		sq->data[sq->rear].play_type=x->play_type;
		sq->data[sq->front].play_state=x->play_state;
		fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
		return QUEUE_IS_SUCCESS;
	}
}

//出队
int DeQueue(sequeue *sq, datatype *x)
{
	if(QUEUE_IS_EMPTY==Empty(sq))
	{	
		return QUEUE_IS_EMPTY;
	}
	else
	{
		sq->front = (sq->front+1) % maxsize;
		x->play_state = sq->data[sq->front].play_state;
		//strcpy(x->play_text,sq->data[sq->front].play_text);
		memcpy(x->play_text,sq->data[sq->front].play_text,strlen(sq->data[sq->front].play_text));
		x->play_type=sq->data[sq->front].play_type;
		memset(sq->data[sq->front].play_text,0,sizeof(sq->data[sq->front].play_text));
		sq->data[sq->front].play_state = 0;
                sq->data[sq->front].play_type = 0;
		fibo_textTrace("fn: %s line: %d\n", __func__, __LINE__);
		fibo_textTrace("Dequeued play_text[0] value = %d\n", sq->data[sq->front].play_text[0]);

        // Log the full play_text string if necessary
                fibo_textTrace("Dequeued play_text string = %s\n", sq->data[sq->front].play_text);
		return QUEUE_IS_SUCCESS;
	}
} 


