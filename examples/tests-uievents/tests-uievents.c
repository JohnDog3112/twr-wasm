#include "twr-uievents.h"
#include "twr-crt.h"  // twr_register_callback
#include <stdio.h>
#include "ctype.h"

__attribute__((import_name("sendGlobalKeyboardEvent")))
void send_global_keyboard_event(const char* event_name, const char* key);
__attribute__((import_name("sendLocalKeyboardEvent")))
void send_local_keyboard_event(const char* event_name, const char* key, const char* element_id);
__attribute__((import_name("sendGlobalMouseEvent")))
void send_global_mouse_event(const char* event_name, double page_x, double page_y);
__attribute__((import_name("sendLocalMouseEvent")))
void send_local_mouse_event(const char* event_name, double page_x, double page_y, const char* element_id, int relative);
__attribute__((import_name("sendGlobalWheelEvent")))
void send_global_wheel_event(const char* event_name, double delta_x, double delta_y, double delta_z, long delta_mode);
__attribute__((import_name("sendLocalWheelEvent")))
void send_local_wheel_event(const char* event_name, double delta_x, double delta_y, double delta_z, long delta_mode, const char* element_id);


int CURRENT_TEST = 0;
int TOTAL_FAILURES = 0;
void run_next_test();

enum EventType {
   KEY_UP,
   KEY_DOWN,

   MOUSE_MOVE,
   MOUSE_UP,
   MOUSE_DOWN,
   MOUSE_CLICK,
   MOUSE_DOUBLE_CLICK,
   
   WHEEL,
};

const char* EVENT_NAMES[20] = {
   "keyup",
   "keydown",

   "mousemove",
   "mouseup",
   "mousedown",
   "click",
   "dblclick",

   "wheel"
};

typedef int (*GlobalRegisterFunc)(int);
const GlobalRegisterFunc GLOBAL_KEYBOARD_REGISTER_FUNCS[2] = {
   register_global_key_up_event,
   register_global_key_down_event,
};

const GlobalRegisterFunc GLOBAL_MOUSE_REGISTER_FUNCS[5] = {
   register_global_mouse_move_event,
   register_global_mouse_up_event,
   register_global_mouse_down_event,
   register_global_mouse_click_event,
   register_global_mouse_double_click_event,
};

typedef int (*LocalKeyboardRegisterFunc)(int, const char*);
const LocalKeyboardRegisterFunc LOCAL_KEYBOARD_REGISTER_FUNCS[2] = {
   register_local_key_up_event,
   register_local_key_down_event
};

typedef int (*LocalMouseRegisterFunc)(int, const char*, int);
const LocalMouseRegisterFunc LOCAL_MOUSE_REGISTER_FUNCS[5] = {
   register_local_mouse_move_event,
   register_local_mouse_up_event,
   register_local_mouse_down_event,
   register_local_mouse_click_event,
   register_local_mouse_double_click_event
};

enum Locality {
   LOCAL,
   GLOBAL
};

struct LocalArgsBase {
   //only case supposed_to_timeout is used for
   // is when element_id != element2_id to ensure locality
   // so element2_id only needs to be specified when
   // supposed_to_timeout = true
   long supposed_to_timeout;
   char* element_id;
   char* element2_id;
   void* extra;
};

struct EventBase {
   long timeout;
   enum EventType event_type;
   enum Locality locality;
   void* args;
   struct LocalArgsBase local_args;
};

struct KeyboardEventArgs {
   const char* key;
   long key_code;
};

struct MouseEventArgs {
   double x;
   double y;
};
struct MouseEventExtraLocalArgs {
   int relative;
};

struct WheelEventArgs {
   double delta_x;
   double delta_y;
   double delta_z;
   long delta_mode;
};


#define NUM_EVENTS 43

struct EventBase events[NUM_EVENTS] = {
   //Global KEY_DOWN
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "a",
         .key_code = 'a'
      }
   },
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "g",
         .key_code = 'g'
      }
   },
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "Backspace",
         .key_code = 8
      }
   },
   //local KEY_DOWN success
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "a",
         .key_code = 'a'
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "g",
         .key_code = 'g'
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   {
      .timeout = 1000,
      .event_type = KEY_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "Backspace",
         .key_code = 8
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   //keydown fail
   {
      .timeout = 100,
      .event_type = KEY_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "a",
         .key_code = 'a'
      },
      .local_args =  {
         .supposed_to_timeout = 1,
         .element_id = "test_div_1",
         .element2_id = "test_div_2",
      }
   },

   //keyup global
   {
      .timeout = 1000,
      .event_type = KEY_UP,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs) {
         .key = "a",
         .key_code = 'a'
      },
   },
   //keyup local
   {
      .timeout = 1000,
      .event_type = KEY_UP,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs) {
         .key = "a",
         .key_code = 'a'
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   //keyup fail
   {
      .timeout = 100,
      .event_type = KEY_UP,
      .locality = LOCAL,
      .args = (void*) &(struct KeyboardEventArgs) {
         .key = "a",
         .key_code = 'a'
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_1",
         .element2_id = "test_div_2"
      }
   },


   //------- MOUSE EVENTS -------
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 54.0,
         .y = 753.0,
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 6756.0,
         .y = 5334.0,
      }
   },
   //mouse_move non-relative local
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 54.0,
         .y = 753.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 6756.0,
         .y = 5334.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   //mouse_move relative local
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 54.0,
         .y = 753.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {
      .timeout = 1000,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 6756.0,
         .y = 5334.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   //mouse_move locality test (should fail)
   {
      .timeout = 100,
      .event_type = MOUSE_MOVE,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 1233.0,
         .y = 23454.0,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_2",
         .element2_id = "test_div_1",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1,
         }
      }
   },

   //mouse_down
   {  //global
      .timeout = 1000,
      .event_type = MOUSE_UP,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      }
   },
   {  //non-relative local
      .timeout = 1000,
      .event_type = MOUSE_UP,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {  //relative local
      .timeout = 1000,
      .event_type = MOUSE_UP,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {  //locality test (should fail)
      .timeout = 100,
      .event_type = MOUSE_UP,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 1233.0,
         .y = 23454.0,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_2",
         .element2_id = "test_div_1",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1,
         }
      }
   },

   //mouse_down
   {  //global
      .timeout = 1000,
      .event_type = MOUSE_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      }
   },
   {  //non-relative local
      .timeout = 1000,
      .event_type = MOUSE_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {  //relative local
      .timeout = 1000,
      .event_type = MOUSE_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {  //locality test (should fail)
      .timeout = 100,
      .event_type = MOUSE_DOWN,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 1233.0,
         .y = 23454.0,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_2",
         .element2_id = "test_div_1",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1,
         }
      }
   },

   //mouse_click
   {  //global
      .timeout = 1000,
      .event_type = MOUSE_CLICK,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      }
   },
   {  //non-relative local
      .timeout = 1000,
      .event_type = MOUSE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {  //relative local
      .timeout = 1000,
      .event_type = MOUSE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {  //locality test (should fail)
      .timeout = 100,
      .event_type = MOUSE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 1233.0,
         .y = 23454.0,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_2",
         .element2_id = "test_div_1",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1,
         }
      }
   },

   //mouse_double_click
   {  //global
      .timeout = 1000,
      .event_type = MOUSE_DOUBLE_CLICK,
      .locality = GLOBAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      }
   },
   {  //non-relative local
      .timeout = 1000,
      .event_type = MOUSE_DOUBLE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 0
         }
      }
   },
   {  //relative local
      .timeout = 1000,
      .event_type = MOUSE_DOUBLE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 500.0,
         .y = 1040.0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_2",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1
         }
      }
   },
   {  //locality test (should fail)
      .timeout = 100,
      .event_type = MOUSE_DOUBLE_CLICK,
      .locality = LOCAL,
      .args = (void*) &(struct MouseEventArgs) {
         .x = 1233.0,
         .y = 23454.0,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_2",
         .element2_id = "test_div_1",
         .extra = (void*) &(struct MouseEventExtraLocalArgs) {
            .relative = 1,
         }
      }
   },


   //------WHEEL EVENTS------
   
   //Global
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = GLOBAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 100.0,
         .delta_y = 150.0,
         .delta_z = 503.0,
         .delta_mode = 0,
      }
   },
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = GLOBAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 434.0,
         .delta_y = 5682.0,
         .delta_z = 953.0,
         .delta_mode = 1,
      }
   },
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = GLOBAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 19234.0,
         .delta_y = 241.0,
         .delta_z = 54234.0,
         .delta_mode = 2,
      }
   },

   //Local
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = LOCAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 100.0,
         .delta_y = 150.0,
         .delta_z = 503.0,
         .delta_mode = 0,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = LOCAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 434.0,
         .delta_y = 5682.0,
         .delta_z = 953.0,
         .delta_mode = 1,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },
   {
      .timeout = 1000,
      .event_type = WHEEL,
      .locality = LOCAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 19234.0,
         .delta_y = 241.0,
         .delta_z = 54234.0,
         .delta_mode = 2,
      },
      .local_args = {
         .supposed_to_timeout = 0,
         .element_id = "test_div_1"
      }
   },

   //Locality test (should fail)
   {
      .timeout = 100,
      .event_type = WHEEL,
      .locality = LOCAL,
      .args = (void*) &(struct WheelEventArgs) {
         .delta_x = 19234.0,
         .delta_y = 241.0,
         .delta_z = 54234.0,
         .delta_mode = 2,
      },
      .local_args = {
         .supposed_to_timeout = 1,
         .element_id = "test_div_1",
         .element2_id = "test_div_2"
      }
   },
};
//state variables
struct EventHandlerStateVariables {
   int current_event;
   int keyboard_event_id;
   int mouse_event_id;
   int wheel_event_id;
   int timeout_event_id;
};
struct EventHandlerStateVariables EVENT_HANDLER = {
   .current_event = -1,
   .keyboard_event_id = -1,
   .mouse_event_id = -1,
   .wheel_event_id = -1,
   .timeout_event_id = -1
};
void reset_event_handler() {
   EVENT_HANDLER.current_event = -1;
   EVENT_HANDLER.keyboard_event_id = -1;
   EVENT_HANDLER.mouse_event_id = -1;
   EVENT_HANDLER.wheel_event_id = -1;
   EVENT_HANDLER.timeout_event_id = -1;
}


void run_next_event();

void print_keycode(long key_code) {
   if (key_code <= 0xFF && (isprint((char)key_code) || isspace((char)key_code))) {
      printf("%c", (char)key_code);
   } else {
      printf("%ld", key_code);
   }
}

void print_event_message_start(struct EventBase* event) {
   const char* event_name = EVENT_NAMES[event->event_type];
   
   if (event->locality == GLOBAL) {
      printf("Global %s ", event_name);
   } else if (event->local_args.supposed_to_timeout) {
      printf("Local (%s, %s) %s ", event->local_args.element_id, event->local_args.element2_id, event_name);
   } else {
      printf("Local (%s) %s ", event->local_args.element_id, event_name);
   }

   switch (event->event_type) {
      case KEY_DOWN:
      case KEY_UP:
      {
         struct KeyboardEventArgs* args = (struct KeyboardEventArgs*)event->args;
         printf("(%s) event", args->key);
      }
      break;

      case MOUSE_MOVE:
      case MOUSE_UP:
      case MOUSE_DOWN:
      case MOUSE_CLICK:
      case MOUSE_DOUBLE_CLICK:
      {
         struct MouseEventArgs* args = (struct MouseEventArgs*)event->args;
         if (event->locality == LOCAL) {
            struct MouseEventExtraLocalArgs* local_mouse = (struct MouseEventExtraLocalArgs*)event->local_args.extra;
            if (local_mouse->relative) {
               printf("(Relative) ");
            }
         }
         printf("(%f, %f) event", args->x, args->y);
      }
      break;

      case WHEEL:
      {
         struct WheelEventArgs* args = (struct WheelEventArgs*)event->args;
         printf("(%f, %f, %f, %ld) event", args->delta_x, args->delta_y, args->delta_z, args->delta_mode);
         
      }
      break;
   }
   
   if (event->locality == LOCAL && event->local_args.supposed_to_timeout) {
      printf(" (should fail): ");
   } else {
      printf(": ");
   }

}
__attribute__((export_name("keyboardEventHandler")))
void keyboard_event_handler(int event_id, long key_code) {
   if (event_id != EVENT_HANDLER.keyboard_event_id) return;
   //disable timeout
   EVENT_HANDLER.timeout_event_id = -1;

   struct EventBase* event = &events[EVENT_HANDLER.current_event];
   assert(KEY_UP <= event->event_type && event->event_type <= KEY_DOWN);
   struct KeyboardEventArgs* args = (struct KeyboardEventArgs*)event->args;

   print_event_message_start(event);

   if (event->locality == LOCAL && event->local_args.supposed_to_timeout) {
      TOTAL_FAILURES++;
      printf("didn't fail! Expected a timeout, got ");
      print_keycode(key_code);
      printf("!\n");
      run_next_event();
      return;
   }

   if (key_code == args->key_code) {
      printf("was successfull!\n");
   } else {
      TOTAL_FAILURES++;
      printf("failed! Expected ");
      print_keycode(args->key_code);
      printf(" got ");
      print_keycode(key_code);
      printf("!\n");
   }

   run_next_event();
}

__attribute__((export_name("mouseEventHandler")))
void mouse_event_handler(int event_id, double x, double y) {
   if (event_id != EVENT_HANDLER.mouse_event_id) return;

   //disable timeout
   EVENT_HANDLER.timeout_event_id = -1;

   struct EventBase* event = &events[EVENT_HANDLER.current_event];
   assert(MOUSE_MOVE <= event->event_type && event->event_type <= MOUSE_DOUBLE_CLICK);

   struct MouseEventArgs* args = (struct MouseEventArgs*)event->args;

   print_event_message_start(event);

   if (event->locality == LOCAL && event->local_args.supposed_to_timeout) {
      TOTAL_FAILURES++;
      printf("failed to timeout!\n");
      run_next_event();
      return;
   }


   if (args->x == x && args->y == y) {
      printf("was successful!\n");
   } else {
      TOTAL_FAILURES++;
      printf("failed! Expected (%f, %f) got (%f, %f)\n", args->x, args->y, x, y);
   }
   run_next_event();
}

__attribute__((export_name("wheelEventHandler")))
void wheel_event_handler(int event_id, double delta_x, double delta_y, double delta_z, long delta_mode) {
   if (event_id != EVENT_HANDLER.wheel_event_id) return;

   //disable timeout
   EVENT_HANDLER.timeout_event_id = -1;

   struct EventBase* event = &events[EVENT_HANDLER.current_event];
   assert(WHEEL == event->event_type);

   struct WheelEventArgs* args = (struct WheelEventArgs*)event->args;
   
   print_event_message_start(event);

   if (event->locality == LOCAL && event->local_args.supposed_to_timeout) {
      printf("failed to timeout!\n");
      TOTAL_FAILURES++;
      run_next_event();
      return;
   }

   if (
      delta_x == args->delta_x && delta_y == args->delta_y
      && delta_z == args->delta_z && delta_mode == args->delta_mode
   ) {
      printf("was successful!\n");
   } else {
      TOTAL_FAILURES++;
      printf(
         "failed! Expected (%f, %f, %f, %ld) got (%f, %f, %f, %ld)\n",
         args->delta_x, args->delta_y, args->delta_z, args->delta_mode,
         delta_x, delta_y, delta_z, delta_mode   
      );
   }
   run_next_event();
}

__attribute__((export_name("timeoutEventHandler")))
void timeout_event_handler(int event_id) {
   if (event_id != EVENT_HANDLER.timeout_event_id) return;

   //stop all other events
   EVENT_HANDLER.keyboard_event_id = -1;
   EVENT_HANDLER.mouse_event_id = -1;
   EVENT_HANDLER.wheel_event_id = -1;
   EVENT_HANDLER.timeout_event_id = -1;

   struct EventBase* event = (struct EventBase*) &events[EVENT_HANDLER.current_event];

   print_event_message_start(event);

   if (event->locality == LOCAL && event->local_args.supposed_to_timeout) {
      printf("successfully failed!\n");
      run_next_event();
      return;
   }

   TOTAL_FAILURES++;
   printf("failed (timed out)!\n");
   run_next_event();
   return;
}
void run_next_event() {
   //bring current_event up, that way everytime run_next_event is called
   // it automatically iterates
   EVENT_HANDLER.current_event++;

   stop_all_ui_events();
   EVENT_HANDLER.keyboard_event_id = -1;
   EVENT_HANDLER.mouse_event_id = -1;
   EVENT_HANDLER.wheel_event_id = -1;
   EVENT_HANDLER.timeout_event_id = -1; 

   if (EVENT_HANDLER.current_event >= NUM_EVENTS) {
      reset_event_handler();
      run_next_test();
      return;
   }

   struct EventBase* event = &events[EVENT_HANDLER.current_event];
   const char* event_name = EVENT_NAMES[event->event_type];

   EVENT_HANDLER.timeout_event_id = twr_register_callback("timeoutEventHandler");
   twr_timer_single_shot(event->timeout, EVENT_HANDLER.timeout_event_id);

   switch (event->event_type) {
      case KEY_UP:
      case KEY_DOWN:
      {
         struct KeyboardEventArgs* args = (struct KeyboardEventArgs*)event->args; 
         EVENT_HANDLER.keyboard_event_id = twr_register_callback("keyboardEventHandler");
         if (event->locality == GLOBAL) {
            GLOBAL_KEYBOARD_REGISTER_FUNCS[event->event_type - KEY_UP](
               EVENT_HANDLER.keyboard_event_id
            );
            send_global_keyboard_event(event_name, args->key);
         } else {
            LOCAL_KEYBOARD_REGISTER_FUNCS[event->event_type - KEY_UP](
               EVENT_HANDLER.keyboard_event_id,
               //if it's supposed_to_timeout, watch the event on element2 so it *should* fail if it's properly local
               event->local_args.supposed_to_timeout ? event->local_args.element2_id : event->local_args.element_id
            );
            send_local_keyboard_event(event_name, args->key, event->local_args.element_id);
         }
      }
      break;

      case MOUSE_MOVE:
      case MOUSE_UP:
      case MOUSE_DOWN:
      case MOUSE_CLICK:
      case MOUSE_DOUBLE_CLICK:
      {
         struct MouseEventArgs* args = (struct MouseEventArgs*)event->args;
         EVENT_HANDLER.mouse_event_id = twr_register_callback("mouseEventHandler");
         if (event->locality == GLOBAL) {
            GLOBAL_MOUSE_REGISTER_FUNCS[event->event_type - MOUSE_MOVE](
               EVENT_HANDLER.mouse_event_id
            );
            send_global_mouse_event(event_name, args->x, args->y);
         } else {
            struct MouseEventExtraLocalArgs* local_mouse = (struct MouseEventExtraLocalArgs*)event->local_args.extra;
            LOCAL_MOUSE_REGISTER_FUNCS[event->event_type - MOUSE_MOVE](
               EVENT_HANDLER.mouse_event_id,
               //if it's supposed_to_timeout, watch the event on element2 so it *should* fail if it's properly local
               event->local_args.supposed_to_timeout ? event->local_args.element2_id : event->local_args.element_id,
               local_mouse->relative
            );
            send_local_mouse_event(
               event_name,
               args->x, args->y,
               event->local_args.element_id, local_mouse->relative
            );
         }
      }
      break;

      case WHEEL:
      {
         struct WheelEventArgs* args = (struct WheelEventArgs*)event->args;
         EVENT_HANDLER.wheel_event_id = twr_register_callback("wheelEventHandler");

         if (event->locality == GLOBAL) {
            register_global_wheel_event(
               EVENT_HANDLER.wheel_event_id
            );
            send_global_wheel_event(
               event_name,
               args->delta_x, args->delta_y,
               args->delta_z, args->delta_mode
            );
         } else {
            register_local_wheel_event(
               EVENT_HANDLER.wheel_event_id,
               event->local_args.supposed_to_timeout ? event->local_args.element2_id : event->local_args.element_id
            );
            send_local_wheel_event(
               event_name,
               args->delta_x, args->delta_y,
               args->delta_z, args->delta_mode,
               event->local_args.element_id
            );
         }
      }
      break;

      default:
      assert(0);
      break;
   }
}

// ---------- Press Key and Stop Event Test ----------

int TEST_KEY_PRESS_AND_STOP_STATE = 0;
int TEST_KEY_AND_STOP_EVENT_ID = -1;
int TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;

int TEST_KEY_AND_STOP_EXTRA_ID = -1;

void reset_test_key_and_stop() {
   TEST_KEY_PRESS_AND_STOP_STATE = 0;
   TEST_KEY_AND_STOP_EVENT_ID = -1;
   TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;
   TEST_KEY_AND_STOP_EXTRA_ID = -1;
}
__attribute__((export_name("testKeyAndStopKeyPress")))
void test_key_and_stop_key_press(int event_id, long key_code) {
   if (TEST_KEY_AND_STOP_EVENT_ID != event_id) return;

   if (TEST_KEY_PRESS_AND_STOP_STATE == 0) {
      if (key_code == (long)'a') {
         printf("Successfully captured key press!");
      } else {
         printf("Received the wrong key! Expected 'a', got '%c'", (char)key_code);
      }
      //test stopping event
      TEST_KEY_PRESS_AND_STOP_STATE = 1; //testing for timeout from key press
      stop_ui_event(TEST_KEY_AND_STOP_EXTRA_ID); //stop key press event
      //get new event id for next timer so previous one is ignored
      TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = twr_register_callback("testKeyAndStopTimeout");
      twr_timer_single_shot(1000, TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID); //call next timer, should fail
      //send another keyboard event, should be ignored
      send_global_keyboard_event("keydown", "a");
   } else {
      printf("Failed to stop the event!\n");
      
      //run next test
      reset_test_key_and_stop();
      run_next_test();

   }
}
__attribute__((export_name("testKeyAndStopTimeout")))
void test_key_and_stop_timeout(int event_id) {
   if (TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID != event_id) return;

   if (TEST_KEY_PRESS_AND_STOP_STATE == 0) {
      printf("Timed out before key press was registered!!\n");
   } else {
      printf(" Succesfully Stopped event!\n");
   }
   //run next test
   reset_test_key_and_stop();
   run_next_test();
}

void test_key_press_and_stop_event() {
   TEST_KEY_AND_STOP_EVENT_ID = twr_register_callback("testKeyAndStopKeyPress");
   TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = twr_register_callback("testKeyAndStopTimeout");

   TEST_KEY_PRESS_AND_STOP_STATE = 0;

   

   TEST_KEY_AND_STOP_EXTRA_ID = register_global_key_down_event(TEST_KEY_AND_STOP_EVENT_ID);

   //make first print for total message:
   printf("TestKeyPressAndStopEvent: ");
   twr_timer_single_shot(1000, TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID);

   send_global_keyboard_event("keydown", "a");

}


// ---------- Test Runner ----------
void run_next_test() {
   stop_all_ui_events(); //stop all ui events
   switch (CURRENT_TEST) {
      case 0:
      {
         test_key_press_and_stop_event();
      }
      break;
      case 1:
      {
         reset_event_handler();
         run_next_event();
      }
      break;

      default:
      {
         printf("Finished with %d failures!\n", TOTAL_FAILURES);
      }
      break;
   }

   CURRENT_TEST++; //iterate CURRENT_TEST for next call
}

// ---------- Test Starter ----------
__attribute__((export_name("runTests")))
void run_tests() {
   CURRENT_TEST = 0;

   run_next_test();
}