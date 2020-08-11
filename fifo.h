typedef struct fifo_struct
{
	int16_t* buffer;
	size_t lenght;  // must be 2^n
	unsigned int head;
	unsigned int tail;
}t_fifo;

t_fifo* fifo_init(unsigned int length);
unsigned int fifo_free(t_fifo *fifo);
char fifo_is_empty(t_fifo *fifo);
int16_t fifo_read(t_fifo *fifo);
void fifo_write(t_fifo *fifo,int16_t c);
