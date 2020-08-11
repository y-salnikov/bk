#include <stddef.h>
#include <stdlib.h>
#include "fifo.h"

t_fifo* fifo_init(unsigned int lenght)
{
	t_fifo* fifo;
	fifo=malloc(sizeof(fifo));
	fifo->buffer=malloc(lenght*sizeof(int16_t));
	if(fifo->buffer==NULL) return NULL;
	fifo->head=0;
	fifo->tail=0;
	fifo->lenght=lenght;
	return fifo;
}

unsigned int fifo_free(t_fifo *fifo)
{
	if(fifo->head>=fifo->tail)
		return fifo->lenght-(fifo->head - fifo->tail)-1;
	else
		return (fifo->tail - fifo->head)-1;
}

char fifo_is_empty(t_fifo *fifo)
{
	return fifo->head==fifo->tail;
}

int16_t fifo_read(t_fifo *fifo)
{
	int16_t c;
	c=fifo->buffer[fifo->tail];
	fifo->tail=(fifo->tail+1) & (fifo->lenght-1);
	return c;
}

void fifo_write(t_fifo *fifo,int16_t c)
{
	
	fifo->buffer[(fifo->head) & (fifo->lenght-1)]=c;
	fifo->head=(fifo->head+1) & (fifo->lenght-1);
}
