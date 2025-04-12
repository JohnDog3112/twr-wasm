#ifndef __TWR_SCREEN_H__
#define __TWR_SCREEN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "twr-io.h"

__attribute__((import_name("twrScreenSpawnWindow"))) int twrScreenSpawnWindow(int jsid, const char* title);
twr_ioconsole_t* twr_screen_spawn_window(twr_ioconsole_t* screen, const char* title);

__attribute__((import_name("twrScreenSetWindowLayer"))) void twrScreenSetWindowLayer(int jsid, int window, int layer);
void twr_screen_set_window_layer(twr_ioconsole_t* screen, twr_ioconsole_t* window, int layer);

__attribute__((import_name("twrScreenMoveWindow"))) void twrScreenMoveWindow(int jsid, int window, double x, double y);
void twr_screen_move_window(twr_ioconsole_t* screen, twr_ioconsole_t* window, double x, double y);

__attribute__((import_name("twrScreenCloseWindow"))) void twrScreenCloseWindow(int jsid, int window);
void twr_screen_close_window(twr_ioconsole_t* screen, twr_ioconsole_t* window);


#ifdef __cplusplus
}
#endif

#endif