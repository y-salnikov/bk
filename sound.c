#include "defines.h"
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <libintl.h>
#include "fifo.h"
#include <math.h>
#define _(String) gettext (String)

#define SOUND_EXPONENT	(8+io_sound_freq/20000)
#define SOUND_BUFSIZE   (1<<SOUND_EXPONENT)		/* about 1/43 sec */
#define MAX_SOUND_AGE	~0	/* always play */
#define SPK_VOLUME 0.3
#define SYNTH_VOLUME 0.3
#define COVOX_VOLUME 0.3
#define FIFO_SIZE (SOUND_BUFSIZE*4)

unsigned io_max_sound_age = MAX_SOUND_AGE;
unsigned io_sound_age = MAX_SOUND_AGE;	/* in io_sound_pace's since last change */
unsigned io_sound_bufsize,
         io_sound_freq = 44100;
double io_sound_pace;
double io_sound_count;
extern unsigned io_sound_val, covox_age;
extern unsigned char covox_val;
extern flag_t nflag, fullspeed;
t_fifo *fifo;


void callback(void * dummy, Uint8 * outbuf, int len)
{
	int i;
	int16_t *buf,v;
	static int16_t old_val;
	
	buf=(int16_t*)outbuf;
	for(i=0;i<len/2;i++)
	{
		if(!fifo_is_empty(fifo))
		{
			v=old_val=fifo_read(fifo);
		}
		else
		{
			//fprintf(stderr,"Sound FIFO empty.\n");
			v=old_val;
		}
		buf[i]=v;
	}

}

/* Called after every instruction */
sound_flush()
{
	int16_t v,cvx;
	double spk_age_filter,cvx_age_filter;
	if(ticks>=io_sound_count)
	{
		v= (io_sound_val & 0x40) ? 32767 : -32767;
		spk_age_filter=(1.0-pow(io_sound_age/1000.0,4));
		if(spk_age_filter<0) spk_age_filter=0.0;
		cvx_age_filter=(1.0-pow(covox_age/1000.0,4));
		if(cvx_age_filter<0) cvx_age_filter=0.0;
		cvx=32767-(covox_val<<8);
		v=(v*SPK_VOLUME*spk_age_filter)+(cvx*COVOX_VOLUME*cvx_age_filter)+(synth_next()*SYNTH_VOLUME);
	
		if(fifo_free(fifo)>0)
		{
			fifo_write(fifo,v);
		}
		else
		{
			fprintf(stderr,"Sound FIFO overrun.\n");
		}
		io_sound_count+=io_sound_pace;
		io_sound_age++;
		covox_age++;
	}
	
}

void sound_finish()
{
	/* release the write thread so it can terminate */
	SDL_PauseAudio(1);
}

SDL_AudioSpec desired;

sound_init()
{
	static init_done = 0;
	int iarg, i;
	if (!nflag)
		return;
	if (fullspeed)
	{
		io_max_sound_age = 2 * SOUND_BUFSIZE;
		/* otherwise UINT_MAX */
	}
	if (init_done)
	{
		io_sound_age = io_max_sound_age;
		return;
	}
	fprintf(stderr, _("sound_init called, bufsize=%d\n"),SOUND_BUFSIZE);

	if (-1 == SDL_InitSubSystem(SDL_INIT_AUDIO))
	{
		fprintf(stderr, _("Failed to initialize audio subsystem\n"));
	}

	desired.format =AUDIO_S16SYS ;
	desired.channels = 1;
	desired.freq = io_sound_freq;
	desired.samples = io_sound_bufsize = SOUND_BUFSIZE;
	desired.callback = callback;
	if (-1 == SDL_OpenAudio(&desired, 0))
	{
		fprintf(stderr, _("Failed to initialize sound, freq %d, %d samples\n"), io_sound_freq, SOUND_BUFSIZE);
		nflag = 0;
		return;
	}
	fifo=fifo_init(FIFO_SIZE);
	io_sound_pace = (TICK_RATE/(double)io_sound_freq);

	atexit(sound_finish);
	SDL_PauseAudio(0);
	init_done = 1;
}
