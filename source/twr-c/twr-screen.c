#include "twr-screen.h"
#include "twr-crt.h"

twr_ioconsole_t* twr_screen_spawn_window(twr_ioconsole_t* screen, const char* title) {
   return twr_jscon(
      twrScreenSpawnWindow(
         __twr_get_jsid(screen), 
         title
      )
   );
}

void twr_screen_set_window_layer(twr_ioconsole_t* screen, twr_ioconsole_t* window, int layer) {
   twrScreenSetWindowLayer(
      __twr_get_jsid(screen),
      __twr_get_jsid(window),
      layer
   );
}

void twr_screen_move_window(twr_ioconsole_t* screen, twr_ioconsole_t* window, double x, double y) {
   twrScreenMoveWindow(
      __twr_get_jsid(screen),
      __twr_get_jsid(window),
      x,
      y
   );
}

void twr_screen_close_window(twr_ioconsole_t* screen, twr_ioconsole_t* window) {
   twrScreenCloseWindow(
      __twr_get_jsid(screen),
      __twr_get_jsid(window)
   );
}