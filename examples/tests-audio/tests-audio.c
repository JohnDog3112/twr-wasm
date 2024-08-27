#include <twr-audio.h>
#include <stdlib.h>
#include <stdio.h>

#define CHANNELS 2
#define SAMPLE_RATE 48000
#define SECONDS 3
void test() {
   float* audio_buffer = (float*)malloc(sizeof(float) * CHANNELS*SAMPLE_RATE*SECONDS);
   for (long channel = 0; channel < CHANNELS; channel++) {
      for (long i = 0; i < SAMPLE_RATE*SECONDS; i++) {
         audio_buffer[channel*SAMPLE_RATE*SECONDS + i] = ((float)(rand() - RAND_MAX/2))/(((float)RAND_MAX)/2.0);
      }
   }

   
   audio_buffer[0] = 0.95;
   audio_buffer[1] = 0.65;

   audio_buffer[SAMPLE_RATE*SECONDS] = -0.95;
   audio_buffer[SAMPLE_RATE*SECONDS + 1] = -0.65;

   long node_id = twrAudioFromSamples(CHANNELS, SAMPLE_RATE, (float*)audio_buffer, SAMPLE_RATE*SECONDS);
   free(audio_buffer);


   twrPlayAudioNode(node_id);


   #ifdef ASYNC
   long id = twrLoadAudioAsync("ping.mp3");
   printf("async audio id %ld\n", id);
   twr_sleep(4200);
   long playback_id = twrPlayAudioNode(id);
   printf("playback_id: %ld\n", playback_id);
   printf("%ld\n", twrQueryAudioPlaybackPosition(playback_id));
   twr_sleep(200);
   printf("%ld\n", twrQueryAudioPlaybackPosition(playback_id));
   twr_sleep(200);
   printf("%ld\n", twrQueryAudioPlaybackPosition(playback_id));
   #endif


}