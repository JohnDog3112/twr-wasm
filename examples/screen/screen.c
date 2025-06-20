#include "twr-window.h"
#include "twr-draw2d.h"
#include "twr-crt.h"
#include "twr-screen.h"
#include "math.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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

/// @brief Simple struct for storing an x and y value as doubles
struct dVec2 {
   double x, y;
};

//enum for the type of cursor currently set for a window
enum CursorType {
   CURSOR_DEFAULT,
   CURSOR_POINTER,
   CURSOR_TEXT,
   CURSOR_NOT_ALLOWED
};
//the name of the cursor, corresponds to the CursorType enum variant in the same position
const char* CURSOR_NAMES[20] = {
   "default",
   "pointer",
   "text",
   "not-allowed"
};

/// @brief Struct used to store the ID's of the main events used for the screen spawner
struct MainWindowEvents {
   int animation_event_id;
   int mouse_move_event_id;
   int mouse_click_event_id;
   int mouse_double_click_event_id;
   int keyboard_press_event_id;

   int canvas_resize_event_id;
};
/// @brief Information about the main screen spawner window
struct MainWindowInfo {
   /// @brief The console for it's window
   twr_ioconsole_t* window;
   /// @brief Console for it's draw canvas
   twr_ioconsole_t* canvas;
   /// @brief The current cursor type being used
   enum CursorType cursor_type;

   /// @brief the size of the canvas
   struct dVec2 canvas_size;
   /// @brief The ID's of the events used
   struct MainWindowEvents events;
};
/// @brief Used to store the text and offset for a given text line
struct IconTextLine {
   struct dVec2 offset;
   const char* text;
};

/// @brief Types of actions an Icon can have
enum IconClickActionType {
   /// @brief Opens some other application
   ICON_ACTION_OPEN_APP,
   /// @brief Opens a popup
   ICON_ACTION_OPEN_POPUP,
   /// @brief Does nothing
   ICON_ACTION_NONE,
};
/// @brief Information for an icon that opens an app
struct IconClickActionOpenApp {
   /// @brief Path for the WASM application for a sync app
   const char* sync_path_str;
   /// @brief Path for the WASM application for an async app
   const char* async_path_str;
   /// @brief The name of the app's initialization function
   const char* init_function;
   /// @brief Number of arguments used for initialization
   size_t init_args_len;
   /// @brief An array of arguments to provide for initialization
   const long* init_args;
   /// @brief The title for the application
   const char* app_title;
   /// @brief If it should spawn a window and bind it to the application
   int bind_window;
};
struct PopupWindowInformation;
/// @brief handlers used for popups. Each function is called with the popup + the event properties
struct IconClickActionPopupFunctions {
   void (*animation_event)(struct PopupWindowInformation* popup, int delta_t);
   void (*cleanup)(struct PopupWindowInformation* popup);
   void (*mouse_move)(struct PopupWindowInformation* popup, int x, int y);
   void (*mouse_click)(struct PopupWindowInformation* popup, int x, int y, int button);
   void (*mouse_double_click)(struct PopupWindowInformation* popup, int x, int y, int button);

   void (*key_down)(struct PopupWindowInformation* popup, int key);
   void (*key_up)(struct PopupWindowInformation* popup, int key);
   void (*canvas_resize)(struct PopupWindowInformation* popup, int width, int height);
};
/// @brief ID's for the event registrations used for each popup
struct IconClickActionPopupFunctionEventIDs {
   long animation_event;
   long mouse_move;
   long mouse_click;
   long mouse_double_click;

   long key_down;
   long key_up;
   long canvas_resize;
};

/// @brief Information about an opened popup
struct PopupWindowInformation {
   /// @brief the ID of the popup
   long id;
   /// @brief The icon the popup was opened with
   struct IconClickActionPopup* icon;
   /// @brief The window console for the popup
   twr_ioconsole_t* window;
   /// @brief The draw canvas for the window
   twr_ioconsole_t* canvas;
   /// @brief The current cursor type being used
   enum CursorType cursor_type;

   /// @brief The size of the canvas
   struct dVec2 canvas_size;
   /// @brief The list of event id's used for callbacks
   struct IconClickActionPopupFunctionEventIDs event_ids;
   /// @brief Pointer for any extra data used for a popup
   void* extra_info;
};

/// @brief Contains the information for an Icon that opens a popup
struct IconClickActionPopup {
   /// @brief Title of the popup that'll be opened
   const char* title;

   /// @brief Array of opened popup instances
   struct PopupWindowInformation** popups;
   /// @brief Number of popup instances
   size_t arr_len;
   /// @brief Max amount the array can hold without being expanded
   size_t arr_alloc_len;

   /// @brief function to initialize a popup when it's opened
   void (*init_new_popup)(struct PopupWindowInformation* popup);
   /// @brief Collection of event handlers for the popups
   struct IconClickActionPopupFunctions functions;
};

/// @brief Tagged union of Icon Action types (Nothing, Open App, and Popup)
struct IconClickAction {
   enum IconClickActionType type;
   union {
      struct IconClickActionOpenApp open_app;
      struct IconClickActionPopup popup;
   };
};
/// @brief Struct for representing Icon's in the App Opener
struct Icon {
   /// @brief The action that should happen when the icon is selected
   struct IconClickAction action; 
   /// @brief url of the Icon's image
   const char* image_src;
   /// @brief ID for the loaded image
   int image_id;
   /// @brief whether it's been initialized yet
   int initialized;
   /// @brief Name of the icon
   const char* item_name;

   /// @brief Name of the icon split among 1 or more lines
   size_t num_text_lines;
   struct IconTextLine text_lines[NUM_ICON_TEXT_LINES];
};

/// @brief list of global events not having to do with a screen/window/canvas
struct GlobalEvents {
   int icon_image_load_event_id;
};

#define NUM_ICONS 4
/// @brief Global state that is shared among everything
struct GlobalState {
   /// @brief The screen everything is setup on
   twr_ioconsole_t* screen;
   /// @brief the main screen spawner window's information
   struct MainWindowInfo main_window_info;
   /// @brief Array of the icons
   struct Icon icons[NUM_ICONS];

   /// @brief Global events
   struct GlobalEvents global_events;
   
   /// @brief The ID the next created object should be given
   int objectID;
   /// @brief Whether everything has been initialized
   int initialized;
   /// @brief Whether or not this is running as async
   int is_async;
};
/// @brief Initial global state
struct GlobalState global_state = {
   .initialized = FALSE,
   .objectID = 0,
};
/// @brief Takes in a string and copies a substring out of it with some right padding
/// @param str The string that should be split
/// @param start The starting index of the substring (inclusive)
/// @param end The ending index of the substring (inclusive)
/// @param end_padding_len How much padding should be added to the end of the string
/// @param pad_char The string that should be used for the padding
/// @return A malloc'd copy of the given substring
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
/// @brief Sets up the text Icon text that's displayed by splitting the name among 1 or more lines.
/// @param ds The ds for some canvas so it can measure text width for splitting
/// @param icon The Icon to setup
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
/// @brief Initializes an Icon. Includes things such as loading the image and setting up the display text.
/// @param ds The ds for some canvas for measuring text width
/// @param icon The Icon to be initialized. (overrides any filled out fields)
/// @param image_src The URL of the image that should be loaded
/// @param item_name The name of the icon
void init_icon(struct d2d_draw_seq* ds, struct Icon* icon, const char* image_src, const char* item_name) {
   //initialize the icon
   *icon = (struct Icon) {
      .action = {
         //Default to no action, programmer can override later
         .type = ICON_ACTION_NONE,
      },
      .image_src = image_src,
      .image_id = global_state.objectID++,
      .initialized = 0,
      .item_name = item_name,
      .num_text_lines = 0,
   };
   //setup the Icon's display text
   setup_icon_text(ds, icon);
   //Start loading the image. When it's loaded or failed, 
   //    it will call the given function and update the Icon's initialized field.
   d2d_load_image_sync_with_con(
      icon->image_src,
      icon->image_id,
      global_state.global_events.icon_image_load_event_id,
      (void*)icon,
      global_state.main_window_info.canvas
   );
}




void app_search_popup_init(struct PopupWindowInformation* popup);
void app_search_popup_cleanup(struct PopupWindowInformation* popup);
void app_search_popup_keydown_event(struct PopupWindowInformation* popup, int ch);
void app_search_popup_animation_event(struct PopupWindowInformation* popup, int delta_t);
void app_search_popup_mouse_move_event(struct PopupWindowInformation* popup, int x, int y);
void app_search_popup_mouse_dblclick_event(struct PopupWindowInformation* popup, int x, int y, int button);



__attribute__((export_name("init")))
//Main function to initialize everything
// Spawns the app spawner and setups event handling, Icons, etc.
int init(int is_async) {
   //ensure that this function hasn't already been ran
   if (global_state.initialized) {
      return FALSE;
   }
   //set initialization and async properties
   global_state.initialized = TRUE;
   global_state.is_async = is_async;

   //Load the screen console for future use
   global_state.screen = twr_get_console("screen");

   //spawn a new window for the App Spawner
   struct MainWindowInfo* main_window_info = &global_state.main_window_info;
   main_window_info->window = twr_screen_spawn_window(global_state.screen, "screen spawner");
   
   //grab the app spawner canvas and set it as the default d2d canvas
   main_window_info->canvas = twr_window_get_draw_canvas(main_window_info->window);
   twr_set_std2d_con(main_window_info->canvas);

   main_window_info->cursor_type = CURSOR_DEFAULT;

   //Register the callbacks for all of the app spawner events
   main_window_info->events = (struct MainWindowEvents) {
      .animation_event_id = twr_register_callback("mainWindowAnimationLoop"),
      .mouse_move_event_id = twr_register_callback("mainWindowMouseEvent"),
      .mouse_click_event_id = twr_register_callback("mainWindowMouseEvent"),
      .mouse_double_click_event_id = twr_register_callback("mainWindowMouseEvent"),
      .keyboard_press_event_id = twr_register_callback("mainWindowKeyboardEvent"),

      .canvas_resize_event_id = twr_register_callback("mainWindowDrawCanvasResize"),
   };
   
   //register all of the app spawner events
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

   //load the initial app spawner canvas size
   main_window_info->canvas_size = (struct dVec2){
      .x = io_get_prop(main_window_info->canvas, "canvasWidth"),
      .y = io_get_prop(main_window_info->canvas, "canvasHeight")
   };

   //register a callback for the image loaded event
   global_state.global_events = (struct GlobalEvents){
      .icon_image_load_event_id = twr_register_callback("iconImageLoadEvent")
   };

   //setup a ds for initializing the icons
   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, main_window_info->canvas);

   //grab a pointer to the icons
   struct Icon* icons = global_state.icons;
   //load each icon one by one. Starts by initializing it and then setting the action properties.
   init_icon(ds, &icons[0], "icons/pong.png", "Pong");
   icons[0].action = (struct IconClickAction){
      //specifies that it should open an app when clicked
      .type = ICON_ACTION_OPEN_APP,
      //therefor, we expand the .open_app part of the union (don't expand the popup one)
      .open_app = {
         //This opens Pong so we'll just call it "Pong"
         .app_title = "Pong",
         //Pong has a different WASM file for async vs. sync so we specify both
         .async_path_str = "../pong/entry-point-a.wasm",
         .sync_path_str = "../pong/entry-point.wasm",
         //It's initialization function doesn't take any args so we can leave these parts blank
         .init_args_len = 0,
         .init_args = NULL,
         //To initialize it, the exported "initMenu" function needs to be called 
         .init_function = "initMenu",
         //When it's setup, it should have a window directly bound to it.
         // In comparison, this example open's it's own window so one shouldn't be opened for it.
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

   //This opens another instance of this example and it behaves a bit differently
   // To start off with, it opens it's own window so it doesn't need one bound.
   // Secondly, it takes in an argument of whether or not it's async

   //So here, we initialize an array of length 1 with a true or false argument of whether it's async
   long* app_opener_args = (long*)malloc(sizeof(long) * 1);
   //set it to the same value this example was opened with
   app_opener_args[0] = is_async;
   init_icon(ds, &icons[2], "icons/app_opener.png", "App Opener");
   icons[2].action = (struct IconClickAction) {
      .type = ICON_ACTION_OPEN_APP,
      .open_app = {
         .app_title = "App Opener",
         .async_path_str = "./screen-a.wasm",
         .sync_path_str = "./screen.wasm",
         //We have one argument
         .init_args_len = 1,
         //And we provide the pointer to the allocated array storing it
         .init_args = app_opener_args,
         .init_function = "init",
         //And then we specify that it shouldn't bind a window
         .bind_window = FALSE,
      }
   };

   //This icon opens a popup with a search menu.
   // Unlike opening an application, the window is bound to this application.
   init_icon(ds, &icons[3], "icons/test_popup.png", "Test Popup");
   icons[3].action = (struct IconClickAction) {
      //Specify that this should open a popup
      .type = ICON_ACTION_OPEN_POPUP,
      //Then, expand the popup section of the union (don't expand the open_app section)
      .popup = {
         //Specify the title of the popup window
         .title = "Test Popup",
         
         //Setup the initial popup instance array to be empty and NULL
         .popups = NULL,
         .arr_len = 0,
         .arr_alloc_len = 0,
         
         //specify a function for initializing the popup
         .init_new_popup = app_search_popup_init,
         //then, add functions for all of the events it should handle
         // Each function is provided with the event's contents + the instance of the popup it was called for
         //Any event without a handler should be set to NULL
         // Otherwise you get some undefined behavior
         .functions = {
            .animation_event = app_search_popup_animation_event,
            .canvas_resize = NULL,
            .cleanup = app_search_popup_cleanup,
            .key_down = app_search_popup_keydown_event,
            .key_up = NULL,
            .mouse_click = NULL,
            .mouse_double_click = app_search_popup_mouse_dblclick_event,
            .mouse_move = app_search_popup_mouse_move_event,
         }
      }
   };

   d2d_end_draw_sequence(ds);
   
   
   return TRUE;
}

__attribute__((export_name("mainWindowDrawCanvasResize")))
///Handle the updating of the main canvas size
void main_window_draw_canvas_resize(int event_id, void* extraPtr, int width, int height) {
   global_state.main_window_info.canvas_size = (struct dVec2){
      .x = width,
      .y = height
   };
}

__attribute__((export_name("iconImageLoadEvent")))
//handle the loading of an Icon's image. If it's successfull, it'll set the icon to initialized
void icon_image_load_event(int event_id, int success, struct Icon* icon) {
   if (success) {
      icon->initialized = 1;
   } else {
      printf("Warning: failed to load image %s\n", icon->image_src);
   }
}
__attribute__((export_name("mainWindowAnimationLoop")))
//Draws the main screen spawner window
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

void run_popup_action(struct IconClickActionPopup* icon_popup);
void run_icon_action(struct Icon* icon);


void set_main_window_mouse_cursor(enum CursorType cursor_type) {
   if (global_state.main_window_info.cursor_type != cursor_type) {
      global_state.main_window_info.cursor_type = cursor_type;

      d2d_set_mouse_cursor_with_con(
         CURSOR_NAMES[(size_t)cursor_type],
         global_state.main_window_info.canvas
      );
   }
}
__attribute__((export_name("mainWindowMouseEvent")))
//Handles mouse events (mainly double click) for icons
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
      ) {
         // printf("Clicked on: %s, with button %d\n", global_state.icons[i].item_name, button);

         if (
            event_id == global_state.main_window_info.events.mouse_double_click_event_id
            && button == 0
         ) {
            run_icon_action(&global_state.icons[i]);
         }
         
         if (event_id == global_state.main_window_info.events.mouse_move_event_id) {
            set_main_window_mouse_cursor(CURSOR_POINTER);
         } else {
            set_main_window_mouse_cursor(CURSOR_DEFAULT);
         }
      }
   }
}
/// @brief Runs the action associated with the given icon
/// @param icon The icon to run the action of
void run_icon_action(struct Icon* icon) {
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
         run_popup_action(&icon->action.popup);
      break;
   }
}

__attribute__((export_name("mainWindowKeyboardEvent")))
void main_window_keyboard_event(int event_id, void* extraPtr, int key) {

}

/// @brief Creates the information for a popup within the given icon
/// @param icon_popup The icon to create new popup information for
/// @return Returns a pointer to the newly created info
struct PopupWindowInformation* create_new_popup_info(struct IconClickActionPopup* icon_popup) {
   //allocate memory for the popup info
   struct PopupWindowInformation* popup = (struct PopupWindowInformation*)malloc(sizeof(struct PopupWindowInformation));
   //ensure the icon has enough room in it's array of instances for the new icon
   if (icon_popup->arr_len == icon_popup->arr_alloc_len || icon_popup->popups == NULL) {
      size_t new_len = (icon_popup->arr_alloc_len == 0 || icon_popup->popups == NULL) 
         ? 10 
         : icon_popup->arr_alloc_len*2;
      
      struct PopupWindowInformation** new_arr = (struct PopupWindowInformation**)malloc(sizeof(struct PopupWindowInformation*) * new_len);
      if (icon_popup->popups != NULL) {
         for (int i = 0; i < icon_popup->arr_len; i++) {
            new_arr[i] = icon_popup->popups[i];
         }
         free(icon_popup->popups);
      } else {
         icon_popup->arr_len = 0;
      }
      icon_popup->popups = new_arr;
      icon_popup->arr_alloc_len = new_len;
   }
   //push the new icon info to the end of the instance array
   icon_popup->popups[icon_popup->arr_len] = popup;
   icon_popup->arr_len++;

   return popup;
}
/// @brief Cleans up the information for a popup instance
/// @param popup The popup instance to clean up/delete
void delete_popup_info(struct PopupWindowInformation* popup) {
   struct IconClickActionPopup* icon_info = popup->icon;
   
   //find the index in the instance array the popup is located in
   int found_index = -1;
   for (int i = 0; i < icon_info->arr_len; i++) {
      if (icon_info->popups[i] == popup) {
         found_index = i;
      }
   }
   //shouldn't be possible to not find it
   // unless something bad happened
   if (found_index == -1) {
      assert(false);
      return;
   }

   //Replaces the index the popup was located at with the last popup in the array
   // Then it shrinks the array size by one
   icon_info->popups[found_index] = icon_info->popups[icon_info->arr_len-1];
   icon_info->popups[icon_info->arr_len-1] = NULL;
   icon_info->arr_len--;

   //free the popup's allocation
   free(popup);
}

/// @brief Opens and initializes a new popup. Calls popup specific initializers as needed.
/// @param icon_popup The icon corresponding to the opening popup
void run_popup_action(struct IconClickActionPopup* icon_popup) {
   
   //create the new popup
   struct PopupWindowInformation* popup = create_new_popup_info(icon_popup);
   /*struct PopupWindowInformation {
      long id;
      struct IconClickActionPopup* icon;
      twr_ioconsole_t* window;
      twr_ioconsole_t* canvas;

      struct dVec2 canvas_size;
      struct IconClickActionPopupFunctionEventIDs event_ids;
      void* extra_info;
   };*/
   //spawn a window for the popup
   twr_ioconsole_t* window = twr_screen_spawn_window(
      global_state.screen,
      icon_popup->title
   );
   //retrieve it's draw canvas
   twr_ioconsole_t* canvas = twr_window_get_draw_canvas(window);
   //register the callbacks for all the event handlers
   struct IconClickActionPopupFunctionEventIDs event_ids = {
      .animation_event = twr_register_callback("popupWindowAnimationEvent"),
      .canvas_resize = twr_register_callback("popupWindowCanvasResizeEvent"),
      .key_down = twr_register_callback("popupWindowKeyDownEvent"),
      .key_up = twr_register_callback("popupWindowKeyUpEvent"),
      .mouse_click = twr_register_callback("popupWindowMouseClickEvent"),
      .mouse_double_click = twr_register_callback("popupWindowMouseDoubleClickEvent"),
      .mouse_move = twr_register_callback("popupWindowMouseMoveEvent"),
   };

   //fill in everything about the popup
   *popup = (struct PopupWindowInformation){
      .id = global_state.objectID++,
      .icon = icon_popup,
      .window = window,
      .canvas = canvas,
      .cursor_type = CURSOR_DEFAULT,
      .canvas_size = (struct dVec2){
         .x = io_get_prop(canvas, "canvasWidth"),
         .y = io_get_prop(canvas, "canvasHeight")
      },

      .event_ids = event_ids,

      .extra_info = NULL,
   };
   //run the popup specific initializer
   // mainly involves allocating something to .extra_info
   if (icon_popup->init_new_popup != NULL)
      icon_popup->init_new_popup(popup);

   //register all of the canvas events with the registered functions setup above
   d2d_register_event_with_con(D2D_CANVAS_RESIZE, event_ids.canvas_resize, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_ANIMATION_FRAME, event_ids.animation_event, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_KEY_DOWN, event_ids.key_down, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_KEY_UP, event_ids.key_up, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_MOUSE_CLICK, event_ids.mouse_click, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_MOUSE_DBLCLICK, event_ids.mouse_double_click, (void*)popup, canvas);
   d2d_register_event_with_con(D2D_MOUSE_MOVE, event_ids.mouse_move, (void*)popup, canvas);

   //setup the close handler
   twr_window_register_close_handler(window, "popupWindowCleanup", (void*)popup);
}
/// @brief Sets the mouse cursor for a popup's window.
/// @param popup The popup to change the cursor for
/// @param cursor The cursor type to change it to
void set_popup_mouse_cursor(struct PopupWindowInformation* popup, enum CursorType cursor) {
   //If the cursor is being set to what it already is, just ignore it
   if (popup->cursor_type != cursor) {
      //update the stored cursor type
      popup->cursor_type = cursor;
      //send the request to change the cursor
      d2d_set_mouse_cursor_with_con(
         CURSOR_NAMES[(size_t)cursor],
         popup->canvas
      );
   }
}
__attribute__((export_name("popupWindowAnimationEvent")))
/// @brief Handles Animation Events for a popup. Propogates down to popup specific handlers.
/// @param event_id ID of the event
/// @param popup The popup the Animation Event is being ran for
/// @param deltaT The time since that last Animation Event
void popup_window_animation_event(int event_id, struct PopupWindowInformation* popup, int deltaT) {
   if (popup->icon->functions.animation_event != NULL)
      popup->icon->functions.animation_event(popup, deltaT);
}

__attribute__((export_name("popupWindowCleanupEvent")))
/// @brief Cleans up a popup when it get's closed. Also calls popup specific cleanup handler.
/// @param event_id ID of the event
/// @param popup The popup being cleaned up
void popup_window_cleanup_event(int event_id, struct PopupWindowInformation* popup) {
   //If the popup has it's own cleanup function, call that first
   if (popup->icon->functions.cleanup != NULL) 
      popup->icon->functions.cleanup(popup);

   //unregister all of the events registered
   struct IconClickActionPopupFunctionEventIDs* events = &popup->event_ids;   
   d2d_unregister_event_with_con(events->animation_event, popup->canvas);
   d2d_unregister_event_with_con(events->canvas_resize, popup->canvas);
   d2d_unregister_event_with_con(events->key_down, popup->canvas);
   d2d_unregister_event_with_con(events->key_up, popup->canvas);
   d2d_unregister_event_with_con(events->mouse_click, popup->canvas);
   d2d_unregister_event_with_con(events->mouse_double_click, popup->canvas);
   d2d_unregister_event_with_con(events->mouse_move, popup->canvas);

   //delete the popup from the list
   delete_popup_info(popup);
}

__attribute__((export_name("popupWindowMouseMoveEvent")))
/// @brief Handles mouse move events for a popup. Propogates down to popup specific handler.
/// @param event_id ID of the event
/// @param popup The popup the mouse move event was called on
/// @param x x position of the mouse
/// @param y y position of the mouse
void popup_window_mouse_move_event(int event_id, struct PopupWindowInformation* popup, int x, int y) {
   if (popup->icon->functions.mouse_move != NULL)
      popup->icon->functions.mouse_move(popup, x, y);
}

__attribute__((export_name("popupWindowMouseClickEvent")))
/// @brief Handles mouse click events for a popup. Propogates update down to popup specific handler.
/// @param event_id ID of the event
/// @param popup The popup the click event was called on
/// @param x x position of the mouse
/// @param y y position of the mouse
/// @param button the button that was clicked
void popup_window_mouse_click_event(int event_id, struct PopupWindowInformation* popup, int x, int y, int button) {
   if (popup->icon->functions.mouse_click != NULL)
      popup->icon->functions.mouse_click(popup, x, y, button);
}

__attribute__((export_name("popupWindowMouseDoubleClickEvent")))
/// @brief Handles mouse double click events for a popup. Propogates update down to popup specific handler.
/// @param event_id ID of the event
/// @param popup The popup the double click event was called on
/// @param x x position of the mouse
/// @param y y position of the mouse
/// @param button the button that was double clicked
void popup_window_mouse_double_click_event(int event_id, struct PopupWindowInformation* popup, int x, int y, int button) {
   if (popup->icon->functions.mouse_double_click != NULL)
      popup->icon->functions.mouse_double_click(popup, x, y, button);
}

__attribute__((export_name("popupWindowKeyDownEvent")))
/// @brief Handles key up events for a popup. Propogates update down to popup specific handlers.
/// @param event_id ID of the event
/// @param popup The popup the key up event was called on
/// @param key The key that was released
void popup_window_mouse_key_down_event(int event_id, struct PopupWindowInformation* popup, int key) {
   if (popup->icon->functions.key_down != NULL)
      popup->icon->functions.key_down(popup, key);
}

__attribute__((export_name("popupWindowKeyUpEvent")))
/// @brief Handles key down events for a popup. Propogates update down to popup specific handlers.
/// @param event_id ID of the event
/// @param popup The popup the key down event was called on
/// @param key The key that was pressed
void popup_window_key_up_event(int event_id, struct PopupWindowInformation* popup, int key) {
   //if the popup has a key_up handler, run it
   if (popup->icon->functions.key_up != NULL)
      popup->icon->functions.key_down(popup, key);
}

__attribute__((export_name("popupWindowCanvasResizeEvent")))
/// @brief Handles canvas resizing events for a popup. Updates canvas size and propogates the update down to popup specific handlers.
/// @param event_id ID of the event
/// @param popup The event the resize event was called on
/// @param width New width of the canvas
/// @param height New height of the canvas
void popup_window_canvas_resize_event(int event_id, struct PopupWindowInformation* popup, int width, int height) {
   popup->canvas_size.x = width;
   popup->canvas_size.y = height;

   //if the popup has a canvas_resize handler, run it
   if (popup->icon->functions.canvas_resize)
      popup->icon->functions.canvas_resize(popup, width, height);
}


struct TextBuffer {
   /// @brief The text stored in the buffer
   char* buffer;
   /// @brief Total length of the current allocation for buffer
   size_t alloc_len;
   /// @brief Length of the string stored in buffer
   size_t str_len;
};

/// @brief Initializes a new TextBuffer with a given size
/// @param buffer TextBuffer to initialize
/// @param alloc_len The initial buffer size to use
void init_text_buffer(struct TextBuffer* buffer, size_t alloc_len) {
   buffer->buffer = (char*)malloc(sizeof(char) * alloc_len);
   buffer->buffer[0] = '\0';
   buffer->alloc_len = alloc_len;
   buffer->str_len = 0;
}

/// @brief Changes the alloc size of a TextBuffer
/// @param buffer TextBuffer to change
/// @param new_alloc_len new buffer size
/// @return number of characters discarded from buffer (if it was shrunk)
int change_alloc_size_of_text_buffer(struct TextBuffer* buffer, size_t new_alloc_len) {
   //Number of chars being discarded (new_alloc_len < current_arr_len)
   size_t discarded_chars = 0;
   if (buffer->str_len > new_alloc_len-1) {
      discarded_chars = buffer->str_len - (new_alloc_len - 1);
      //set the new str_len to the new_alloc_len - 1 (it needs to include the '\0' terminator)
      buffer->str_len = new_alloc_len-1;
      //Add the '\0' terminator to the end of the new string length
      buffer->buffer[new_alloc_len-1] = '\0'; 
   }

   //allocate the new buffer
   char* new_buffer_text = (char*)malloc(sizeof(char) * new_alloc_len);
   //Copy the string in the old buffer to the new one
   // The if statement above should ensure that the new
   // buffer is *always* longer than the string in the old buffer
   strcpy(new_buffer_text, buffer->buffer);
   //free the old buffer
   free(buffer->buffer);

   //set the new buffer properties
   buffer->buffer = new_buffer_text;
   buffer->alloc_len = new_alloc_len;

   //return how many chars were discarded
   return discarded_chars;
}

/// @brief Deinitializes a Text Buffer (frees it's internal memory)
/// @param buffer TextBuffer to deinitialize
void deinit_text_buffer(struct TextBuffer* buffer) {
   free(buffer->buffer);
   buffer->buffer = NULL;
   buffer->str_len = -1;
   buffer->alloc_len = -1;
}
/// @brief Attempts to set the buffer's held text to the provided string
/// @param buffer Text Buffer to set
/// @param str The string to set the Buffer to
/// @return Returns how many characters in the provided string didn't fit in the buffer
int set_text_buffer(struct TextBuffer* buffer, const char* str) {
   //if the provided str is NULL, clear the buffer
   if (str == NULL) {
      buffer->buffer[0] = '\0';
      buffer->str_len = 0;
      return 0;
   }

   //Copy over the string to the buffer
   // Ensures that it can't go passed the buffer size
   size_t i = 0;
   for (; i < buffer->alloc_len-1 && str[i] != '\0'; i++) {
      buffer->buffer[i] = str[i]; 
   }
   //i should be the index of '\0' in the str
   //therefor it should be the string length since arrays are 0 indexed
   buffer->str_len = i;
   buffer->buffer[i] = '\0';

   //counts the number of remaining characters in the provided string
   size_t ret_val = 0;
   for (; str[i + ret_val] != '\0'; ret_val++);

   //returns how many characters didn't fit in the buffer
   return ret_val;
}
char* copy_text_buffer(struct TextBuffer* buffer) {
   //simply creates a copy of the string in the buffer
   return strdup(buffer->buffer);
}
/// @brief Appends a char to the end of the text buffer
/// @param buffer TextBuffer to append to
/// @param ch Char to append
/// @return Returns whether it was successfully appended
bool append_char_to_text_buffer(struct TextBuffer* buffer, char ch) {
   //if the buffer is full, return failure
   if (buffer->alloc_len-1 >= buffer->str_len) return FALSE;

   //otherwise add the char to the end of the string
   buffer->buffer[buffer->str_len] = ch;
   buffer->buffer[buffer->str_len+1] = '\0';
   buffer->str_len++;

   return TRUE;
}
/// @brief Appends a string to the end of the text buffer
/// @param buffer TextBuffer to append to
/// @param str String to append
/// @return The number of characters that couldn't be added to the buffer (0 for successful)
int append_str_to_text_buffer(struct TextBuffer* buffer, const char* str) {
   //if the provided string is NULL return
   if (str == NULL) {
      return 0;
   }
   //Use the previous length to append to the end of the string
   // and to set the new length
   size_t prev_len = buffer->str_len;
   //keep count of how many characters were appended
   size_t i = 0;
   //Keep appending chars from the provided string until it either:
   // 1. Reaches the end of the string
   // 2. Or, it reaches the end of the allocated buffer
   for (; i+prev_len < buffer->alloc_len-1 && str[i] != '\0'; i++) {
      buffer->buffer[prev_len+i] = str[i]; 
   }
   //i should be the index of '\0' in the str
   //therefor it should be the string length since arrays are 0 indexed
   buffer->str_len = prev_len+i;
   buffer->buffer[prev_len+i] = '\0';

   //count how many characters in the string weren't added
   size_t ret_val = 0;
   for (; str[i + ret_val] != '\0'; ret_val++);

   return ret_val;
}
/// @brief Pops the last char from the TextBuffer
/// @param buffer Buffer to pop from
/// @return The char that was popped, returns '\0' if buffer is empty
char pop_char_from_text_buffer(struct TextBuffer* buffer) {
   //if the buffer is empty, return '\0'
   if (buffer->str_len == 0) return '\0';
   //get the char being popped
   char popped = buffer->buffer[buffer->str_len-1];
   //redue string length by one
   buffer->str_len--;
   //then replace that character with a null terminator
   buffer->buffer[buffer->str_len] = '\0';

   return popped;
}
/// @brief Pops a string of a given length from the end of the TextBuffer
/// @param buffer TextBuffer
/// @param str_len Length of the string to be popped
/// @param dest_str The destination of the popped string. If set to NULL, it'll skip this part
/// @return The number of characters that this failed to pop (buffer was too small)
int pop_str_from_text_buffer(struct TextBuffer* buffer, size_t str_len, char* dest_str) {
   //number of characters actually popped = Min(str_len, buffer->str_len)
   size_t used_len = str_len;
   if (used_len > buffer->str_len) {
      used_len = buffer->str_len;
   }

   //if the destination string isn't NULL,
   // then copy the string being popped into the destination string
   if (dest_str != NULL) {
      strcpy(dest_str, &buffer->buffer[buffer->str_len - used_len]);
   }
   //replace the start of the popped string with '\0'
   buffer->buffer[buffer->str_len - used_len] = '\0';
   //shrink array length
   buffer->str_len -= used_len;

   //return number of characters that weren't popped
   return str_len - used_len;
}
/// @brief Clears the given buffer
/// @param buffer Buffer to clear
void clear_text_buffer(struct TextBuffer* buffer) {
   buffer->str_len = 0;
   buffer->buffer[0] = '\0';
}

/// @brief Constants used for App Search Popups
struct AppSearchConstants {
   /// @brief Size of the search text box
   const struct dVec2 text_box_size;
   /// @brief Size of each option box
   const struct dVec2 option_size;

   /// @brief background color of the search text box
   const unsigned long text_box_color;
   /// @brief Text color used
   const unsigned long text_color;
   /// @brief Background color of an option box
   const unsigned long option_color;
   /// @brief Background color of an option box when it's selected
   const unsigned long selected_option_color;
   /// @brief Color of the border around an option box
   const unsigned long option_border_color;

   const char* search_font;
   const char* option_font;
};
const struct AppSearchConstants APP_SEARCH_CONSTANTS = {
   .text_box_size = {
      .x = 200.0,
      .y = 50.0
   },
   .option_size = {
      .x = 200.0,
      .y = 50.0
   },
   .text_box_color = 0xD3D3D3FF,
   .text_color = 0x0000FFFF,
   .option_color = 0x707070FF,
   .selected_option_color = 0x909090FF,
   .option_border_color = 0xFFFFFFFF,

   .search_font = "16px Seriph",
   .option_font = "16px Seriph",
};
/// @brief Information used for the App Search Popup
struct AppSearchPopupInfo {
   /// @brief TextBuffer for the search bar
   struct TextBuffer buffer;
   /// @brief Width of each icon name. Doesn't change after initialization
   double option_text_widths[NUM_ICONS];
   /// @brief Current width of the search buffer
   double search_text_width;

   /// @brief Index (relative to the number of filtered options) that is selected
   size_t selected_index;
   /// @brief ID of the selected option (-1 for nothing) based on it's index in the global_state.icons array
   int selected_id;
   /// @brief Number of options available after filtering
   size_t num_options;
};
//mainly used so I don't make any casting mistakes
//accidentally did (struct AppSearchPopupInfo*)&popup->extra_info
// before and it caused a bunch of errors since I was casting the wrong
// section of memory
//    unfortunately... it didn't segfault, so I didn't immediately know that's what it was doing :/
struct AppSearchPopupInfo* extra_app_search_popup_info(struct PopupWindowInformation* popup) {
   return (struct AppSearchPopupInfo*)popup->extra_info;
}
/// @brief Initializes extra_info for the popup
/// @param popup Popup to initialize
void app_search_popup_init(struct PopupWindowInformation* popup) {
   struct AppSearchPopupInfo* info = (struct AppSearchPopupInfo*)malloc(sizeof(struct AppSearchPopupInfo));

   //initialize the text buffer
   init_text_buffer(&info->buffer, 100);

   //get the text widths for all the icon names
   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, popup->canvas);
   d2d_save(ds);
   d2d_setfont(ds, APP_SEARCH_CONSTANTS.option_font);
   struct d2d_text_metrics metrics;
   for (size_t i = 0; i < NUM_ICONS; i++) {
      d2d_measuretext(ds, global_state.icons[i].item_name, &metrics);
      info->option_text_widths[i] = metrics.width;
      printf("Set width: %ld: %f\n", (long)i, info->option_text_widths[i]);
   }
   d2d_restore(ds);
   d2d_end_draw_sequence(ds);

   //initialize search text width to zero, will be updated as it's typed in
   info->search_text_width = 0.0;

   info->selected_index = 0;
   info->selected_id = 0;
   info->num_options = NUM_ICONS;

   popup->extra_info = (void*)info;
}
/// @brief Frees the allocated memory from extra_info
/// @param popup Popup to cleanup
void app_search_popup_cleanup(struct PopupWindowInformation* popup) {
   struct AppSearchPopupInfo* info = (struct AppSearchPopupInfo*)popup->extra_info;
   
   deinit_text_buffer(&info->buffer);
   free(info);
   popup->extra_info = NULL;
}

/// @brief Returns if/where a string is contained within another
/// @param str String to search
/// @param search_str String to search for
/// @return Returns -1 for not found or the position it was found at
int strfind(const char* str, const char* search_str) {
   for (size_t i = 0; str[i] != '\0'; i++) {
      for (size_t j = 0; ; j++) {
         if (search_str[j] == '\0') return i;

         if (tolower(str[i+j]) != tolower(search_str[j])) break;
      }
   }

   return -1;
}

/// @brief Returns where the search text box for an app search popup starts
struct dVec2 app_search_popup_get_text_start(struct PopupWindowInformation* popup) {
   const struct dVec2* TEXT_BOX_SIZE = &APP_SEARCH_CONSTANTS.text_box_size;
   return (struct dVec2){
      .x = (popup->canvas_size.x - TEXT_BOX_SIZE->x)/2.0,
      .y = TEXT_BOX_SIZE->y/2.0
   };
}

/// @brief Returns where an option box with a given offset starts for an app search popup
struct dVec2 app_search_popup_get_option_box(struct PopupWindowInformation* popup, size_t offset) {
   struct dVec2 text_start = app_search_popup_get_text_start(popup);
   const struct dVec2* TEXT_BOX_SIZE = &APP_SEARCH_CONSTANTS.text_box_size;
   const struct dVec2* OPTION_SIZE = &APP_SEARCH_CONSTANTS.option_size;

   return (struct dVec2){
      .x = text_start.x,
      .y = text_start.y + TEXT_BOX_SIZE->y + OPTION_SIZE->y*offset
   };
}
/// @brief Used to identify the id and position of an option
struct AppSearchSelectedOption {
   int selected_id;
   size_t selected_index;
};

/// @brief Returns the option that contains the given x, y coordinates. Returns -1 in selected_id if none are found
struct AppSearchSelectedOption app_search_popup_get_hovered_option(struct PopupWindowInformation* popup, double x, double y) {
   struct AppSearchSelectedOption selected_option = {
      .selected_id = -1,
      .selected_index = 0
   };

   struct AppSearchPopupInfo* info = extra_app_search_popup_info(popup);
   const struct dVec2* option_size = &APP_SEARCH_CONSTANTS.option_size;
   size_t offset = 0;
   for (size_t i = 0; i < NUM_ICONS; i++) {
      int loc = strfind(global_state.icons[i].item_name, info->buffer.buffer);
      if (loc == -1 && info->buffer.str_len != 0) continue;

      struct dVec2 option_pos = app_search_popup_get_option_box(popup, offset);

      if (
         option_pos.x <= x && x <= option_pos.x + option_size->x
         && option_pos.y <= y && y <= option_pos.y + option_size->y 
      ) {
         selected_option.selected_id = i;
         selected_option.selected_index = offset;
         break;
      }

      offset++;
   }

   return selected_option;
}

/// @brief Opens the selected option then closes the popup
/// @param popup Popup the option comes from
/// @param option_id The id of the option selected
void app_search_popup_run_option(struct PopupWindowInformation* popup, size_t option_id) {
   // printf("selected %d!\n", option_id);
   run_icon_action(&global_state.icons[option_id]);

   twr_screen_close_window(global_state.screen, popup->window);
}

/**
 *  Handles mouse move events for an App Search Popup. Updates the selected app.
 * @param popup Popup information
 * @param x x pos of the mouse
 * @param y y pos of the mouse
*/ 
void app_search_popup_mouse_move_event(struct PopupWindowInformation* popup, int x, int y) {
   struct AppSearchSelectedOption selected_option = app_search_popup_get_hovered_option(popup, x, y);
   if (selected_option.selected_id != -1) {
      struct AppSearchPopupInfo* info = extra_app_search_popup_info(popup);
      info->selected_id = selected_option.selected_id;
      info->selected_index = selected_option.selected_index;
      set_popup_mouse_cursor(popup, CURSOR_POINTER);
   } else {
      set_popup_mouse_cursor(popup, CURSOR_DEFAULT);
   }
}

/**
 *  Handles mouse double click events for an App Search Popup. Used for opening the clicked on icon
 * @param popup Popup information
 * @param x x position of the mouse
 * @param y y position of the mouse
 * @param button the id of the pressed button
*/ 
void app_search_popup_mouse_dblclick_event(struct PopupWindowInformation* popup, int x, int y, int button) {
   struct AppSearchSelectedOption selected_option = app_search_popup_get_hovered_option(popup, x, y);
   if (selected_option.selected_id != -1) {
      struct AppSearchPopupInfo* info = extra_app_search_popup_info(popup);
      info->selected_id = selected_option.selected_id;
      info->selected_index = selected_option.selected_index;

      app_search_popup_run_option(popup, info->selected_id);
   }
}

/**
 *  Handles key events for an App Search Popup. Mainly adds characters to the search bar and helps with selection
 * @param popup Popup information
 * @param ch Key code for the pressed key
*/ 
void app_search_popup_keydown_event(struct PopupWindowInformation* popup, int ch) {
   struct AppSearchPopupInfo* info = extra_app_search_popup_info(popup);

   size_t starting_buffer_len = info->buffer.str_len;
   if (ch <= 0xFF && isprint(ch)) {
      append_char_to_text_buffer(&info->buffer, ch);
   } else {
      printf("pressed char: %d\n", ch);
      switch (ch) {
         case 8: //backspace
         {
            pop_char_from_text_buffer(&info->buffer);
         }
         break;

         case 10: //enter
         {
            app_search_popup_run_option(popup, info->selected_id);
         }
         break;

         case 127: //delete
         {
            clear_text_buffer(&info->buffer);
         }
         break;

         case 8593: //up arrow
         {
            if (info->selected_index != 0) {
               info->selected_index--;
               size_t counter = info->selected_index;
               for (size_t i = 0; i < NUM_ICONS; i++) {
                  int loc = strfind(global_state.icons[i].item_name, info->buffer.buffer);
                  if (loc == -1 && info->buffer.str_len != 0) continue;

                  if (counter == 0) {
                     info->selected_id = i;
                     break;
                  }
                  counter--;
               }
            }
         }
         break;

         case 8595: //down arrow
         {
            if (info->selected_index < info->num_options) {
               info->selected_index++;
               size_t counter = info->selected_index;
               for (size_t i = 0; i < NUM_ICONS; i++) {
                  int loc = strfind(global_state.icons[i].item_name, info->buffer.buffer);
                  if (loc == -1 && info->buffer.str_len != 0) continue;

                  if (counter == 0) {
                     info->selected_id = i;
                     break;
                  }
                  counter--;
               }
            }
         }
         break;
      }
   }

   //if buffer length changed, then the string changed
   //update the search width and the selected item
   if (starting_buffer_len != info->buffer.str_len) {

      int new_selected_id = -1;
      size_t new_selected_pos = 0;
      size_t num_options = 0;
      for (size_t i = 0; i < NUM_ICONS; i++) {
         int loc = strfind(global_state.icons[i].item_name, info->buffer.buffer);
         // printf("%s: %d\n", global_state.icons[i].item_name, loc);
         if (loc == -1 && info->buffer.str_len != 0) continue;

         if (i == info->selected_id) {
            new_selected_pos = num_options;
            new_selected_id = info->selected_id;
         } else if (new_selected_id == -1) {
            new_selected_pos = num_options;
            new_selected_id = i;
         }
         num_options++;
      } 
      info->selected_id = new_selected_id;
      info->selected_index = new_selected_pos;
      info->num_options = num_options;
      

      struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, popup->canvas);
      d2d_save(ds);
      d2d_setfont(ds, APP_SEARCH_CONSTANTS.search_font);
      
      struct d2d_text_metrics metrics;
      d2d_measuretext(ds, info->buffer.buffer, &metrics);
      info->search_text_width = metrics.width;

      d2d_restore(ds);
      d2d_end_draw_sequence(ds);
   }
}

/// @brief Renders the App Search Popup Screen
/// @param popup Popup information
/// @param delta_t Time since last call
void app_search_popup_animation_event(struct PopupWindowInformation* popup, int delta_t) {
   
   struct AppSearchPopupInfo* info = extra_app_search_popup_info(popup);

   struct d2d_draw_seq* ds = d2d_start_draw_sequence_with_con(100, popup->canvas);
   d2d_save(ds);

   d2d_setfillstylergba(ds, 0x3a3a3aFF);
   d2d_fillrect(ds, 0.0, 0.0, popup->canvas_size.x, popup->canvas_size.y);

   d2d_setfillstylergba(ds, APP_SEARCH_CONSTANTS.text_box_color);
   const struct dVec2* TEXT_BOX_SIZE = &APP_SEARCH_CONSTANTS.text_box_size;
   struct dVec2 text_start = app_search_popup_get_text_start(popup);
   d2d_fillrect(ds, text_start.x, text_start.y, TEXT_BOX_SIZE->x, TEXT_BOX_SIZE->y);

   d2d_setcanvaspropstring(ds, "textAlign", "left");
   d2d_setcanvaspropstring(ds, "textBaseline", "middle");
   d2d_setfillstylergba(ds, APP_SEARCH_CONSTANTS.text_color);
   d2d_setfont(ds, APP_SEARCH_CONSTANTS.search_font);
   if (info->buffer.str_len != 0)
      d2d_filltext(
         ds,
         info->buffer.buffer,
         text_start.x + (TEXT_BOX_SIZE->x - info->search_text_width)/2.0,
         text_start.y + TEXT_BOX_SIZE->y/2.0
      );
   
   const struct dVec2* OPTION_SIZE = &APP_SEARCH_CONSTANTS.option_size; 

   d2d_setfont(ds, APP_SEARCH_CONSTANTS.option_font);
   size_t offset = 0;
   for (size_t i = 0; i < NUM_ICONS; i++) {
      int loc = strfind(global_state.icons[i].item_name, info->buffer.buffer);
      // printf("%s: %d\n", global_state.icons[i].item_name, loc);
      if (loc == -1 && info->buffer.str_len != 0) continue;

      struct dVec2 option_box = app_search_popup_get_option_box(popup, offset);
      offset++;

      if (i == info->selected_id) {
         d2d_setfillstylergba(ds, APP_SEARCH_CONSTANTS.selected_option_color);
         d2d_setstrokestylergba(ds, APP_SEARCH_CONSTANTS.option_border_color);
      } else {
         d2d_setfillstylergba(ds, APP_SEARCH_CONSTANTS.option_color);
         d2d_setstrokestylergba(ds, APP_SEARCH_CONSTANTS.option_border_color);
      }
      
      d2d_fillrect(ds, option_box.x, option_box.y, OPTION_SIZE->x, OPTION_SIZE->y);
      d2d_strokerect(ds, option_box.x, option_box.y, OPTION_SIZE->x, OPTION_SIZE->y);

      d2d_setfillstylergba(ds, APP_SEARCH_CONSTANTS.text_color);
      d2d_filltext(
         ds,
         global_state.icons[i].item_name,
         option_box.x + (OPTION_SIZE->x - info->option_text_widths[i])/2.0,
         option_box.y + OPTION_SIZE->y/2.0
      );
   } 
   d2d_restore(ds);
   d2d_end_draw_sequence(ds);
}


