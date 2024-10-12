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
void send_local_mouse_event(const char* event_name, double page_x, double page_y, const char* element_id);
__attribute__((import_name("sendGlobalWheelEvent")))
void send_global_wheel_event(const char* event_name, double delta_x, double delta_y, double delta_z, long delta_mode);
__attribute__((import_name("sendLocalWheelEvent")))
void send_local_wheel_event(const char* event_name, double delta_x, double delta_y, double delta_z, long delta_mode, const char* element_id);


int CURRENT_TEST = 0;
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

const char* EVENT_NAMES[10] = {
   "keyup",
   "keydown",

   "mousemove",
   "mouseup",
   "mousedown"
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

struct EventBase {
   long timeout;
   long supposed_to_timeout;
   enum EventType event_type;
   enum Locality locality;
   void* args;
   void* local_args;
};

struct KeyboardEventArgs {
   const char* key;
   long key_code;
};
struct LocalKeyboardEventArgs {
   char* element_id;
};

struct MouseEventArgs {
   double page_x;
   double page_y;
};
struct LocalMouseEventArgs {
   char* element_id;
   int relative;
   double local_x, local_y;
};

struct WheelEventArgs {
   double delta_x;
   double delta_y;
   double delta_z;
   long delta_mode;
};
struct LocalWheelEventArgs {
   char* element_id;
};


#define NUM_EVENTS 3

struct EventBase events[NUM_EVENTS] = {
   {
      .timeout = 1000,
      .supposed_to_timeout = 0,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "a",
         .key_code = 'a'
      }
   },
   {
      .timeout = 1000,
      .supposed_to_timeout = 0,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "g",
         .key_code = 'g'
      }
   },
   {
      .timeout = 1000,
      .supposed_to_timeout = 0,
      .event_type = KEY_DOWN,
      .locality = GLOBAL,
      .args = (void*) &(struct KeyboardEventArgs){
         .key = "Backspace",
         .key_code = 8
      }
   }
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
__attribute__((export_name("keyboardEventHandler")))
void keyboard_event_handler(int event_id, long key_code) {
   if (event_id != EVENT_HANDLER.keyboard_event_id) return;
   //disable timeout
   EVENT_HANDLER.timeout_event_id = -1;

   struct EventBase* event = (struct EventBase*) &events[EVENT_HANDLER.current_event];
   assert(KEY_UP <= event->event_type <= KEY_DOWN);
   struct KeyboardEventArgs* args = (struct KeyboardEventArgs*)event->args;

   const char* event_name = EVENT_NAMES[event->event_type];

   if (event->locality == GLOBAL) {
      printf("Global %s (%s) event", event_name, args->key);
   } else {
      struct LocalKeyboardEventArgs* local_args = (struct LocalKeyboardEventArgs*)event->local_args;
      printf("Local (%s) %s (%s) event", local_args->element_id, event_name, args->key);
   }

   if (event->supposed_to_timeout) {
      printf(" (should fail): didn't fail! Expected a timeout, got ");
      print_keycode(key_code);
      printf("!\n");
      run_next_event();
      return;
   }
   printf(": ");

   if (key_code == args->key_code) {
      printf("was successfull!\n");
   } else {
      printf("failed! Expected ");
      print_keycode(args->key_code);
      printf(" got ");
      print_keycode(key_code);
      printf("!\n");
   }

   run_next_event();
}

__attribute__((export_name("mouseEventHandler")))
void mouse_event_handler(int event_id, long page_x, long page_y) {
   if (event_id != EVENT_HANDLER.mouse_event_id) return;
}

__attribute__((export_name("wheelEventHandler")))
void wheel_event_handler(int event_id, long delta_x, long delta_y, long delta_z, long delta_mode) {
   if (event_id != EVENT_HANDLER.wheel_event_id) return;
}

__attribute__((export_name("timeoutEventHandler")))
void timeout_event_handler(int event_id) {
   if (event_id != EVENT_HANDLER.timeout_event_id) return;
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
      // run_next_test();
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
            struct LocalKeyboardEventArgs* local_args = (struct LocalKeyboardEventArgs*)event->local_args;
            LOCAL_KEYBOARD_REGISTER_FUNCS[event->event_type - KEY_UP](
               EVENT_HANDLER.keyboard_event_id,
               local_args->element_id
            );
            send_local_keyboard_event(event_name, args->key, local_args->element_id);
         }
      }
      break;

      case MOUSE_MOVE:
      case MOUSE_UP:
      case MOUSE_DOWN:
      case MOUSE_CLICK:
      case MOUSE_DOUBLE_CLICK:
      EVENT_HANDLER.mouse_event_id = twr_register_callback("mouseEventHandler");
      break;

      case WHEEL:
      EVENT_HANDLER.mouse_event_id = twr_register_callback("wheelEventHandler");
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
   }

   CURRENT_TEST++; //iterate CURRENT_TEST for next call
}

// ---------- Test Starter ----------
__attribute__((export_name("runTests")))
void run_tests() {
   CURRENT_TEST = 0;

   run_next_test();
}