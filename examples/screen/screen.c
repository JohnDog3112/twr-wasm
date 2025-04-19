#include "twr-window.h"
#include "twr-draw2d.h"
#include "twr-crt.h"
#include "twr-screen.h"
#include "math.h"
#include<stdio.h>

#define TRUE 1
#define FALSE 0



struct dVec2 {
   double x, y;
};

struct MainWindowEvents {
   int animation_event_id;
   int mouse_move_event_id;
   int mouse_click_event_id;
   int mouse_double_click_event_id;
   int keyboard_press_event_id;
};
struct MainWindowInfo {
   twr_ioconsole_t* window;
   twr_ioconsole_t* canvas;

   struct dVec2 canvas_size;
   struct MainWindowEvents events;
};
struct Icon {
   const char* image_src;
   int image_id;
   int initialized;
   const char* item_name;
};

struct GlobalEvents {
   int icon_image_load_event_id;
};

#define NUM_ICONS 3
struct GlobalState {
   twr_ioconsole_t* screen;
   struct MainWindowInfo main_window_info;
   struct Icon icons[NUM_ICONS];

   struct GlobalEvents global_events;
   int objectID;
   int initialized;
};
struct GlobalState global_state = {
   .initialized = FALSE,
   .objectID = 0,
};
void init_icon(struct Icon* icon, const char* image_src, const char* item_name) {
   *icon = (struct Icon) {
      .image_src = image_src,
      .image_id = global_state.objectID++,
      .initialized = 0,
      .item_name = item_name
   };
   d2d_load_image_sync_with_con(
      icon->image_src,
      icon->image_id,
      global_state.global_events.icon_image_load_event_id,
      (void*)icon,
      global_state.main_window_info.canvas
   );
}

__attribute__((export_name("init")))
int init() {
   if (global_state.initialized) {
      return FALSE;
   }
   global_state.initialized = TRUE;

   global_state.screen = twr_get_console("screen");

   struct MainWindowInfo* main_window_info = &global_state.main_window_info;
   main_window_info->window = twr_screen_spawn_window(global_state.screen, "screen spawner");

   main_window_info->canvas = twr_window_get_draw_canvas(main_window_info->window);
   twr_set_std2d_con(main_window_info->canvas);

   main_window_info->events = (struct MainWindowEvents) {
      .animation_event_id = twr_register_callback("mainWindowAnimationLoop"),
      .keyboard_press_event_id = twr_register_callback("mainWindowKeyboardEvent"),
   };

   main_window_info->canvas_size = (struct dVec2){
      .x = io_get_prop(main_window_info->canvas, "canvasWidth"),
      .y = io_get_prop(main_window_info->canvas, "canvasHeight")
   };

   global_state.global_events = (struct GlobalEvents){
      .icon_image_load_event_id = twr_register_callback("iconImageLoadEvent")
   };

   struct Icon* icons = global_state.icons;
   init_icon(&icons[0], "icons/pong.png", "Pong");
   init_icon(&icons[1], "icons/window_example.png", "Window Example");
   init_icon(&icons[2],"icons/app_opener.png", "App Opener");
   
   
   return TRUE;
}

__attribute__((export_name("mainWindowDrawCanvasResize")))
int main_window_draw_canvas_resize(int event_id, int width, int height) {
   global_state.main_window_info.canvas_size = (struct dVec2){
      .x = width,
      .y = height
   };
}

__attribute__((export_name("iconImageLoadEvent")))
void icon_image_load_event(int event_id, int success, struct Icon* icon) {
   if (success) {
      icon->initialized = 1;
   } else {
      printf("Warning: failed to load image %s\n", icon->image_src);
   }
}


const double IMAGE_WIDTH = 50;
const double IMAGE_HEIGHT = 50;
const double ICON_X_PADDING = 10;
const double ICON_Y_PADDING = 10;
const double ICON_WIDTH = 70;
const double ICON_HEIGHT = 70;
__attribute__((export_name("mainWindowAnimationLoop")))
void main_window_animation_loop(int event_id, int delta_t) {
   int icons_per_row = ceil(global_state.main_window_info.canvas_size.x/ICON_WIDTH);

   int row = 0;
   int column = 0;
   for (int i = 0; i < NUM_ICONS; i++) {
      double x = row*ICON_WIDTH;
   }
}

__attribute__((export_name("mainWindowMouseEvent")))
void main_window_mouse_event(int event_id, int x, int y, int button) {

}

__attribute__((export_name("mainWindowKeyboardEvent")))
void main_window_keyboard_event(int event_id, int key) {

}