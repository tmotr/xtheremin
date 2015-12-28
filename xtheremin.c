/***
 *
 * Xtheremin 
 *
 * written by tmotr
 *
 ***/

/***
 * Macro Defininitions
 ***/
#include <math.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <alsa/asoundlib.h>
#define PI 3.141569420 ///yeah, i know that's not pi

#define VOLUME 255
#define SEQLEN 5000
#define BUFLEN 255
/***
 * Variables Defined for X interface
 ***/
Display * rDisplay; ///<< connection to the X server
int rScreen;        ///<< the screen to use
Colormap rColors;   ///<< color map appropo for given screen
int rDepth;         ///<< default depth of screen
XSetWindowAttributes * rAttrs;
Window rWindow;
int rX = 0;
int rY = 0;
unsigned int mask_return = 0;
int rRX = 0;
int rRY = 0;
Window root_window_return;
Window child_window_return;

/***
 * Variables Defined for Alsa interface
 ***/
snd_pcm_t *handle;
snd_pcm_sframes_t frames;
static char *device = "default";
snd_output_t *output = NULL;
unsigned char buffer[BUFLEN];

/***
 * Function Definition for Theramin Emulation
 ***/
int theremin (int t, int x, int y)
{ ///amplify by y, modulate by x
  int err;
  unsigned int i;
  short sequence = 0;
  short alternator = x=0?1:x;
  short stretcher = y=0?1:y;
  for (i = 0; i < sizeof(buffer); i++){
    buffer[i] = (unsigned char)((671/stretcher*abs(sequence)+t)%VOLUME);
    if (sequence >= SEQLEN) alternator *= -1;
    else if (sequence <= -SEQLEN) alternator *= -1;
    else sequence += alternator;
  }
  t += i;
  frames = snd_pcm_writei(handle, buffer, sizeof(buffer));
  if (frames < 0)
    frames = snd_pcm_recover(handle, frames, 0);
  t = t>pow(2,30)?0:t;
  return t;
}


int main (void) {
  int t = 0;
  unsigned err;
  unsigned short successlog = 0;
  //0
  if(!(rDisplay = XOpenDisplay(NULL))) goto errbadfail;
  else ++successlog;
  //1
  if(rScreen = XDefaultScreen(rDisplay)) goto errbadfail;
  else ++successlog;
  //2
  if(!(printf("connection#=%d\n",ConnectionNumber(rDisplay)))) goto errbadfail;
  else ++successlog;
  //3
  if(!(printf("blackpx=%u\n",XBlackPixel(rDisplay,rScreen)))) goto errbadfail;
  else ++successlog;
  //4
  if(!(printf("whitepx=%u\n",XWhitePixel(rDisplay,rScreen)))) goto errbadfail;
  else ++successlog;
  //5
  if(!(rColors = XDefaultColormap(rDisplay,rScreen))) goto errbadfail;
  else ++successlog;
  //6
  if(!(rDepth = XDefaultDepth(rDisplay,rScreen))) goto errbadfail;
  else ++successlog;
  //7
  if(!(XNoOp(rDisplay))) goto errbadfail;
  else ++successlog;
  //8
  if(!(XSetCloseDownMode(rDisplay,DestroyAll))) goto errbadfail;
  else ++successlog;
  //9
  if(!(rWindow = XCreateWindow(rDisplay,RootWindow(rDisplay,rScreen),0,0,666,666,3,rDepth,InputOutput,StaticGray,0,rAttrs))) goto errbadfail;
  else ++successlog;
  //10
  if(!(XMapRaised(rDisplay,rWindow))) goto errbadfail;
  else ++successlog;

  //OPEN SOUND SERVER
  if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    printf("Playback open error: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  }
  if ((err = snd_pcm_set_params(handle,
				SND_PCM_FORMAT_U8,
				SND_PCM_ACCESS_RW_INTERLEAVED,
				1,
				48000,
				1,
				500000)) < 0) {   /* 0.5sec */
    printf("Playback open error: %s\n", snd_strerror(err));
    exit(EXIT_FAILURE);
  } 

  //main loop
  do {
    XFlush(rDisplay);
    XQueryPointer(rDisplay,rWindow,&root_window_return,&child_window_return,&rRX,&rRY,&rX,&rY,&mask_return);
    printf("(X,Y,Z)=(%d,%d,%d)\n",rX,rY,t);
    t = theremin(t, rX, rY);
	
  } while (rX > 0 && rY > 0);


  //???
  if(XCloseDisplay(rDisplay)) goto errbadfail;
  else ++successlog;

  //???

  if((err = snd_pcm_close(handle)) < 0) goto errbadfail;
  else ++successlog;
    
  printf("Ah, the good death! %d successful calls :> \n", successlog);
  return successlog;

 errbadfail:
  printf("FUCKSHIT! I died with err code %d :< \n",successlog);
  return successlog;

}
