#include "twr-window.h"
#include "twr-draw2d.h"
#include "twr-crt.h"
#include "twr-screen.h"
#include "math.h"
#include<stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

const double IMAGE_WIDTH = 50;
const double IMAGE_HEIGHT = 50;
const double ICON_X_PADDING = 10;
const double ICON_Y_PADDING = 10;
const double ICON_WIDTH = 70;
const double ICON_HEIGHT = 70;

const double IMAGE_X_OFFSET = (ICON_WIDTH - IMAGE_WIDTH)/2.0;
const double IMAGE_Y_OFFSET = 0.0;


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
struct IconTextLine {
   struct dVec2 offset;
   const char* text;
};
struct Icon {
   const char* image_src;
   int image_id;
   int initialized;
   const char* item_name;
   size_t num_text_lines;
   struct IconTextLine text_lines[3];
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
void setup_icon_text(struct d2d_draw_seq* ds, struct Icon* icon) {
   for (size_t i = 0; i < icon->num_text_lines; i++) {
      free(icon->text_lines[i].text);
   }
   char* main_text = strdup(icon->item_name);
   struct d2d_text_metrics metrics;
   d2d_measuretext(ds, main_text, &metrics);

   // for (size_t i = 0; i < )

}
void init_icon(struct d2d_draw_seq* ds, struct Icon* icon, const char* image_src, const char* item_name) {
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

   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, main_window_info->canvas);

   struct Icon* icons = global_state.icons;
   init_icon(ds, &icons[0], "icons/pong.png", "Pong");
   init_icon(ds, &icons[1], "icons/window_example.png", "Window Example");
   init_icon(ds, &icons[2], "icons/app_opener.png", "App Opener");

   d2d_end_draw_sequence(ds);
   
   
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

__attribute__((export_name("mainWindowAnimationLoop")))
void main_window_animation_loop(int event_id, int delta_t) {
   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, global_state.main_window_info.canvas);
   struct MainWindowInfo* main_window_info = &global_state.main_window_info;
   struct dVec2 canvas_size = main_window_info->canvas_size;

   d2d_setfillstylergba(ds, 0xFF00FFFF);
   d2d_fillrect(ds, 0, 0, canvas_size.x, canvas_size.y);


   int icons_per_row = ceil(canvas_size.x/ICON_WIDTH);

   int row = 0;
   int column = 0;
   struct Icon* icons = global_state.icons;
   for (int i = 0; i < NUM_ICONS; i++) {
      double x = row*ICON_WIDTH + (row-1)*ICON_X_PADDING;
      double y = column*ICON_HEIGHT + (column-1)*ICON_Y_PADDING;

      d2d_drawimage_ex(ds, icons[i].image_id, 0, 0, 0, 0, x+IMAGE_X_OFFSET, y+IMAGE_Y_OFFSET, IMAGE_WIDTH, IMAGE_HEIGHT);


   }

   d2d_end_draw_sequence(ds);
}

__attribute__((export_name("mainWindowMouseEvent")))
void main_window_mouse_event(int event_id, int x, int y, int button) {

}

__attribute__((export_name("mainWindowKeyboardEvent")))
void main_window_keyboard_event(int event_id, int key) {

}