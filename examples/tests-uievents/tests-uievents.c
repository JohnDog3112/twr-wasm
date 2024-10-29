#include "twr-uievents.h"
#include "twr-crt.h"  // twr_register_callback
#include <stdio.h>

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
   MOUSE_DOWN,
   MOUSE_UP,
   MOUSE_CLICK,
   MOUSE_DOUBLE_CLICK,
   
   WHEEL,
};

const char* EVENT_NAMES[10] = {
   "keyup",
   "keydown",

   "mousemove",
   "mousedown",
   "mouseup",
   "click",
   "dblclick",

   "wheel"
};

enum Locality {
   LOCAL,
   GLOBAL
};


#define NUM_EVENTS 1
#define MAX_EVENT_LEN 9
//format:
//General: [TIMEOUT, SUPPOSED_TO_TIMEOUT, Event, Locality, args...]
// ... = TIMEOUT, SUPPOSED_TO_TIMEOUT
// [..., KEY_*, GLOBAL, "key", keycode] or [..., KEY_*, LOCAL, 'key', keycode, "element_id"]
// [..., MOUSE_*, GLOBAL, page_x, page_y] or [..., MOUSE_*, LOCAL, page_x, page_y, "element_id"]
// [..., WHEEL_*, GLOBAL, delta_x, delta_y, delta_z, delta_mode] or [..., WHEEL_*, LOCAL, delta_x, delta_y, delta_z, delta_mode, "element_id"]
long events[NUM_EVENTS][MAX_EVENT_LEN] = {
   {1000, 0, KEY_DOWN, GLOBAL, (long)"a", (long)'a'},
};

//state variables
int current_event = 0;
int KEYBOARD_EVENT_ID = -1;
int MOUSE_EVENT_ID = -1;
int WHEEL_EVENT_ID = -1;

__attribute__((export_name("keyboardEventHandler")))
void keyboard_event_handler(int event_id, long key_code) {
   printf("sent %ld, got %ld!\n", events[current_event][3], key_code);
}

__attribute__((export_name("mouseEventHandler")))
void mouse_event_handler(int event_id, long page_x, long page_y) {

}

__attribute__((export_name("wheelEventHandler")))
void wheel_event_handler(int event_id, long delta_x, long delta_y, long delta_z, long delta_mode) {

}

void run_next_event() {
   if (current_event >= NUM_EVENTS) {
      return;
   }
   stop_all_ui_events();
   KEYBOARD_EVENT_ID = -1;
   MOUSE_EVENT_ID = -1;
   WHEEL_EVENT_ID = -1;

   long* event = events[current_event];
   // char* event_name;
   switch(event[0]) {
      case KEY_DOWN:
      break;
   }
}

// ---------- Press Key and Stop Event Test ----------

int TEST_KEY_PRESS_AND_STOP_STATE = 0;
int TEST_KEY_AND_STOP_EVENT_ID = -1;
int TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;

int TEST_KEY_AND_STOP_EXTRA_ID = -1;

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
      //stop timeout from being called
      TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;

      //run next test
      CURRENT_TEST++;
      TEST_KEY_AND_STOP_EVENT_ID = -1;
      TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;
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
   CURRENT_TEST++;
   TEST_KEY_AND_STOP_EVENT_ID = -1;
   TEST_KEY_AND_STOP_EVENT_TIMEOUT_ID = -1;
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

   }
}

// ---------- Test Starter ----------
__attribute__((export_name("runTests")))
void run_tests() {
   current_event = 0;
   CURRENT_TEST = 0;

   run_next_test();
}