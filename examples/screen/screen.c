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
const double ICON_HEIGHT = 90;
#define NUM_ICON_TEXT_LINES 3
const char* ICON_TEXT_FONT = "12px Seriph";
const unsigned long ICON_TEXT_COLOR = 0x000000FF;

const double ICON_TEXT_IMAGE_SPACE = 5.0;
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

   int canvas_resize_event_id;
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

enum IconClickActionType {
   ICON_ACTION_OPEN_APP,
   ICON_ACTION_OPEN_POPUP,
   ICON_ACTION_NONE,
};
struct IconClickActionOpenApp {
   const char* sync_path_str;
   const char* async_path_str;
   const char* init_function;
   size_t init_args_len;
   const long* init_args;
   const char* app_title;
   int bind_window;
};

struct PopupInfoPtr {
   struct PopupWindowInformation* popup_window;
   struct IconClickActionPopup* icon;
};
struct PopupWindowInformation {
   long id;
   struct PopupInfoPtr info_ptr;
   twr_ioconsole_t* window;
   twr_ioconsole_t* canvas;

   struct dVec2 canvas_size;
   void* extra_info;
};

struct IconClickActionPopup {
   const char* title;

   struct PopupWindowInformation* popups;
   size_t arr_len;
   size_t arr_alloc_len;

   void (*animation_event)();
   void (*cleanup)();
   void (*mouse_move)();
   void (*mouse_click)();
   void (*mouse_double_click)();

   void (*key_down)();
   void (*key_up)();
   void (*canvas_resize)();
};

struct IconClickAction {
   enum IconClickActionType type;
   union {
      struct IconClickActionOpenApp open_app;
      struct IconClickActionPopup popup;
   };
};
struct Icon {
   struct IconClickAction action; 
   const char* image_src;
   int image_id;
   int initialized;
   const char* item_name;
   size_t num_text_lines;
   struct IconTextLine text_lines[NUM_ICON_TEXT_LINES];
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
   int is_async;
};
struct GlobalState global_state = {
   .initialized = FALSE,
   .objectID = 0,
};
char* sub_string(const char* str, size_t start, size_t end, size_t end_padding_len, char pad_char) {
   assert(start < end);
   char* res_str = (char*)malloc(end - start + 1 + end_padding_len);
   size_t res_index = 0;
   for (size_t i = start; i < end; i++) {
      res_str[res_index] = str[i];
      res_index++;
   }
   for (size_t i = 0; i < end_padding_len; i++) {
      res_str[res_index] = pad_char;
      res_index++;
   }
   res_str[res_index] = '\0';
   return res_str;
}
void setup_icon_text(struct d2d_draw_seq* ds, struct Icon* icon) {
   for (size_t i = 0; i < icon->num_text_lines; i++) {
      free((void*)(icon->text_lines[i].text));
   }
   d2d_setfont(ds, ICON_TEXT_FONT);
   char* main_text = strdup(icon->item_name);
   struct d2d_text_metrics metrics;
   icon->num_text_lines = 1;
   for (size_t i = 0; i < NUM_ICON_TEXT_LINES; i++) {
      d2d_measuretext(ds, main_text, &metrics);
      if (metrics.width <= ICON_WIDTH) {
         icon->text_lines[i].text = main_text;
         main_text = NULL;
      } else {
         char* new_line = NULL;
         int completed = 0;
         //attempt to split the string on a space
         size_t str_len = strlen(main_text);
         for (size_t j = str_len-1; j >= 0; j--) {
            if (main_text[j] == ' ') {
               char tmp = main_text[j];
               main_text[j] = '\0';
               d2d_measuretext(ds, main_text, &metrics);
               main_text[j] = tmp;
               if (metrics.width <= ICON_WIDTH) {
                  completed = 1;
                  //realloc the string to the reduced size
                  new_line = sub_string(main_text, 0, j, 0, '\0');
                  //do the same with the main_text to get the rest of the text
                  char* tmp = sub_string(main_text, j+1, str_len, 0, '\0');
                  free(main_text);
                  main_text = tmp;
                  break;
               }
            }
         }
         //if it failed to split on a space, split mid-word
         if (!completed) {
            //splits long words:
            // Measurements:
            //    Measure-
            //    meants
            for (size_t j = 0; main_text[j] != '\0'; j++) {
               char tmp1 = main_text[j];
               char tmp2 = main_text[j+1];
               main_text[j] = '-';
               main_text[j+1] = '\0';
               d2d_measuretext(ds, main_text, &metrics);
               main_text[j] = tmp1;
               main_text[j+1] = tmp2;
               if (metrics.width > ICON_WIDTH) {
                  size_t split_index = j - 2;
                  new_line = sub_string(main_text, 0, split_index, 1, '-');
                  char* tmp = sub_string(main_text, split_index, str_len, 0, '\0');
                  free(main_text);
                  main_text = tmp;
               }
            }
         }
         icon->text_lines[i].text = new_line;
      }
      d2d_measuretext(ds, icon->text_lines[i].text, &metrics);
      icon->text_lines[i].offset.x = (ICON_WIDTH - metrics.width)/2.0;
      icon->text_lines[i].offset.y = IMAGE_HEIGHT + ICON_TEXT_IMAGE_SPACE + i*(ICON_HEIGHT - IMAGE_HEIGHT - ICON_TEXT_IMAGE_SPACE)/NUM_ICON_TEXT_LINES;

      if (main_text == NULL) break;
      icon->num_text_lines++;
   }
   if (main_text != NULL) {
      free(main_text);
      main_text = NULL;
   }
}
void init_icon(struct d2d_draw_seq* ds, struct Icon* icon, const char* image_src, const char* item_name) {
   *icon = (struct Icon) {
      .action = {
         .type = ICON_ACTION_NONE,
      },
      .image_src = image_src,
      .image_id = global_state.objectID++,
      .initialized = 0,
      .item_name = item_name,
      .num_text_lines = 0,
   };
   setup_icon_text(ds, icon);
   d2d_load_image_sync_with_con(
      icon->image_src,
      icon->image_id,
      global_state.global_events.icon_image_load_event_id,
      (void*)icon,
      global_state.main_window_info.canvas
   );
}

__attribute__((export_name("init")))
int init(int is_async) {
   printf("a\n");
   if (global_state.initialized) {
      return FALSE;
   }
   global_state.initialized = TRUE;
   global_state.is_async = is_async;

   printf("b\n");
   global_state.screen = twr_get_console("screen");

   printf("c\n");
   struct MainWindowInfo* main_window_info = &global_state.main_window_info;
   main_window_info->window = twr_screen_spawn_window(global_state.screen, "screen spawner");
   printf("d\n");

   main_window_info->canvas = twr_window_get_draw_canvas(main_window_info->window);
   twr_set_std2d_con(main_window_info->canvas);
   printf("e\n");

   main_window_info->events = (struct MainWindowEvents) {
      .animation_event_id = twr_register_callback("mainWindowAnimationLoop"),
      .mouse_move_event_id = twr_register_callback("mainWindowMouseEvent"),
      .mouse_click_event_id = twr_register_callback("mainWindowMouseEvent"),
      .mouse_double_click_event_id = twr_register_callback("mainWindowMouseEvent"),
      .keyboard_press_event_id = twr_register_callback("mainWindowKeyboardEvent"),

      .canvas_resize_event_id = twr_register_callback("mainWindowDrawCanvasResize"),
   };
   printf("f\n");

   d2d_register_event_with_con(
      D2D_ANIMATION_FRAME,
      main_window_info->events.animation_event_id,
      NULL,
      main_window_info->canvas
   );
   d2d_register_event_with_con(
      D2D_MOUSE_MOVE,
      main_window_info->events.mouse_move_event_id,
      NULL,
      main_window_info->canvas
   );
   d2d_register_event_with_con(
      D2D_MOUSE_CLICK,
      main_window_info->events.mouse_click_event_id,
      NULL,
      main_window_info->canvas
   );
   d2d_register_event_with_con(
      D2D_MOUSE_DBLCLICK,
      main_window_info->events.mouse_double_click_event_id,
      NULL,
      main_window_info->canvas
   );
   d2d_register_event_with_con(
      D2D_KEY_DOWN,
      main_window_info->events.keyboard_press_event_id,
      NULL,
      main_window_info->canvas
   );
   d2d_register_event_with_con(
      D2D_CANVAS_RESIZE,
      main_window_info->events.canvas_resize_event_id,
      NULL,
      main_window_info->canvas
   );
   printf("g\n");

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
   icons[0].action = (struct IconClickAction){
      .type = ICON_ACTION_OPEN_APP,
      .open_app = {
         .app_title = "Pong",
         .async_path_str = "../pong/entry-point-a.wasm",
         .sync_path_str = "../pong/entry-point.wasm",
         .init_args_len = 0,
         .init_args = NULL,
         .init_function = "initMenu",
         .bind_window = TRUE,
      }
   };
   init_icon(ds, &icons[1], "icons/window_example.png", "Window Example");
   icons[1].action = (struct IconClickAction) {
      .type = ICON_ACTION_OPEN_APP,
      .open_app = {
         .app_title = "Window Example",
         .async_path_str = "../window/window-a.wasm",
         .sync_path_str = "../window/window.wasm",
         .init_args_len = 0,
         .init_args = NULL,
         .init_function = "init",
         .bind_window = TRUE,
      }
   };

   long* app_opener_args = (long*)malloc(sizeof(long) * 1);
   app_opener_args[0] = is_async;
   init_icon(ds, &icons[2], "icons/app_opener.png", "App Opener");
   icons[2].action = (struct IconClickAction) {
      .type = ICON_ACTION_OPEN_APP,
      .open_app = {
         .app_title = "App Opener",
         .async_path_str = "./screen-a.wasm",
         .sync_path_str = "./screen.wasm",
         .init_args_len = 1,
         .init_args = app_opener_args,
         .init_function = "init",
         .bind_window = FALSE,
      }
   };

   d2d_end_draw_sequence(ds);
   
   
   return TRUE;
}

__attribute__((export_name("mainWindowDrawCanvasResize")))
void main_window_draw_canvas_resize(int event_id, void* extraPtr, int width, int height) {
   global_state.main_window_info.canvas_size = (struct dVec2){
      .x = width,
      .y = height
   };
}

__attribute__((export_name("iconImageLoadEvent")))
void icon_image_load_event(int event_id, void* extraPtr, int success, struct Icon* icon) {
   if (success) {
      icon->initialized = 1;
   } else {
      printf("Warning: failed to load image %s\n", icon->image_src);
   }
}
__attribute__((export_name("mainWindowAnimationLoop")))
void main_window_animation_loop(int event_id, void* extraPtr, int delta_t) {
   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, global_state.main_window_info.canvas);
   struct MainWindowInfo* main_window_info = &global_state.main_window_info;
   struct dVec2 canvas_size = main_window_info->canvas_size;

   d2d_setfillstylergba(ds, 0xFF00FFFF);
   d2d_fillrect(ds, 0, 0, canvas_size.x, canvas_size.y);


   int row = 0;
   int column = 0;
   struct Icon* icons = global_state.icons;
   for (size_t i = 0; i < NUM_ICONS; i++) {
      double x = row*(ICON_WIDTH + ICON_X_PADDING);
      if (x + ICON_WIDTH > global_state.main_window_info.canvas_size.x && row != 0) {
         row = 0;
         column++;
         x = row*(ICON_WIDTH + ICON_X_PADDING);
      }
      double y = column*(ICON_HEIGHT + ICON_Y_PADDING);
      row++;

      if (icons[i].initialized) {
         d2d_drawimage_ex(ds, icons[i].image_id, 0, 0, 0, 0, x+IMAGE_X_OFFSET, y+IMAGE_Y_OFFSET, IMAGE_WIDTH, IMAGE_HEIGHT);
      } else {
         d2d_setfillstylergba(ds, 0xFF0000FF);
         d2d_fillrect(ds, x + IMAGE_X_OFFSET, y + IMAGE_Y_OFFSET, IMAGE_WIDTH, IMAGE_HEIGHT);
      }

      d2d_setfont(ds, ICON_TEXT_FONT);
      d2d_setfillstylergba(ds, ICON_TEXT_COLOR);
      
      d2d_setcanvaspropstring(ds, "textBaseline", "top");
      for (size_t j = 0; j < icons[i].num_text_lines; j++) {
         d2d_filltext(
            ds,
            icons[i].text_lines[j].text,
            x + icons[i].text_lines[j].offset.x,
            y + icons[i].text_lines[j].offset.y
         );
      }
   }

   d2d_end_draw_sequence(ds);
}

__attribute__((import_name("spawnApplication")))
void spawn_application(const char* title, const char* path, const char* init_func, const long* init_args, size_t init_args_len, int bind_window);

__attribute__((export_name("mainWindowMouseEvent")))
void main_window_mouse_event(int event_id, void* extraPtr, int mouse_x, int mouse_y, int button) {
   int row = 0;
   int column = 0;
   for (size_t i = 0; i < NUM_ICONS; i++) {
      double x = row*(ICON_WIDTH + ICON_X_PADDING);
      if (x + ICON_WIDTH > global_state.main_window_info.canvas_size.x && row != 0) {
         row = 0;
         column++;
         x = row*(ICON_WIDTH + ICON_X_PADDING);
      }
      double y = column*(ICON_HEIGHT + ICON_Y_PADDING);
      row++;

      if (
         x <= mouse_x && mouse_x <= x+ICON_WIDTH
         && y <= mouse_y && mouse_y <= y+ICON_HEIGHT
         && event_id == global_state.main_window_info.events.mouse_double_click_event_id
         && button == 0
      ) {
         // printf("Clicked on: %s, with button %d\n", global_state.icons[i].item_name, button);
         struct Icon* icon = &global_state.icons[i];
         switch (icon->action.type) {
            case ICON_ACTION_NONE:
               //do nothing
            break;
            
            case ICON_ACTION_OPEN_APP:
            {
               struct IconClickActionOpenApp* action = &icon->action.open_app;
               spawn_application(
                  action->app_title,
                  global_state.is_async ? action->async_path_str : action->sync_path_str,
                  action->init_function,
                  action->init_args,
                  action->init_args_len,
                  action->bind_window
               );
            }
            break;

            case ICON_ACTION_OPEN_POPUP:
               //TODO
               twr_screen_spawn_window(
                  global_state.screen,
                  "popup"
               );
            break;
         }
      }
   }
}

__attribute__((export_name("mainWindowKeyboardEvent")))
void main_window_keyboard_event(int event_id, void* extraPtr, int key) {

}

__attribute__((export_name("popupWindowAnimationEvent")))
void popup_window_animation_event(int event_id, void* extraPtr, int deltaT) {
   for (size_t i = 0; i < NUM_ICONS; i++) {
      struct Icon* icon = &global_state.icons[i];
      if (icon->action.type == ICON_ACTION_OPEN_POPUP) {

      }
   }
}