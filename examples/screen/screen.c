#include "twr-window.h"
#include "twr-draw2d.h"
#include "twr-crt.h"
#include "twr-screen.h"

#define TRUE 1
#define FALSE 0



struct dVec2 {
   double x, y;
};

struct MainWindowEvents {
   int animationEventID;
   int mouseMoveEventID;
   int mouseClickEventID;
   int mouseDoubleClickEventID;
   int keyboardPressEventID;
};
struct MainWindowInfo {
   twr_ioconsole_t* window;
   twr_ioconsole_t* canvas;

   struct dVec2 canvas_size;
   struct MainWindowEvents events;
};
struct GlobalState {
   twr_ioconsole_t* screen;
   struct MainWindowInfo main_window_info;

   int objectID;
   int initialized;
};
struct GlobalState global_state = {
   .initialized = FALSE,
   .objectID = 0,
};

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
      .animationEventID = twr_register_callback("mainWindowAnimationLoop"),
      .keyboardPressEventID = twr_register_callback("mainWindowKeyboardEvent"),
   };

   main_window_info->canvas_size = (struct dVec2){
      .x = io_get_prop(main_window_info->canvas, "canvasWidth"),
      .y = io_get_prop(main_window_info->canvas, "canvasHeight")
   };
   
   return TRUE;
}

__attribute__((export_name("mainWindowDrawCanvasResize")))
int main_window_draw_canvas_resize(int event_id, int width, int height) {
   global_state.main_window_info.canvas_size = (struct dVec2){
      .x = width,
      .y = height
   };
}


struct Icon {
   const char* image_src;
   int image_id;
   int loaded_image;
   const char* item_name;
};


__attribute__((export_name("iconImageLoadEvent")))
void icon_image_load_event(int event_id) {

}

__attribute__((export_name("mainWindowAnimationLoop")))
void main_window_animation_loop(int event_id, int delta_t) {
   d2d_load_image_with_con("")
}

__attribute__((export_name("mainWindowMouseEvent")))
void main_window_mouse_event(int event_id, int x, int y, int button) {

}

__attribute__((export_name("mainWindowKeyboardEvent")))
void main_window_keyboard_event(int event_id, int key) {

}