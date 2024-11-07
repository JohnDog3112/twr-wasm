#include <twr-draw2d.h>
#include <stdlib.h>
#include <string.h>

enum twrWidgetEventType {
   TWR_WIDGET_EVENT_KEY_UP,
   TWR_WIDGET_EVENT_KEY_DOWN,
   TWR_WIDGET_EVENT_MOUSE_MOVE,
   TWR_WIDGET_EVENT_MOUSE_DOWN,
   TWR_WIDGET_EVENT_MOUSE_UP,
   TWR_WIDGET_EVENT_MOUSE_CLICK,
   TWR_WIDGET_EVENT_MOUSE_DBLCLICK,
   TWR_WIDGET_REQUEST_ANIMATION_FRAME,
   TWR_WIDGET_WHEEL,
};
#define TWR_LAST_WIDGET_EVENT_TYPE TWR_WIDGET_WHEEL
struct twrWidgetEventBase {
   enum twrWidgetEventType type;
};

struct twrWidgetKeyboardEvent {
   struct twrWidgetEventBase base;
   int key;
};

struct twrWidgetMouseEvent {
   struct twrWidgetEventBase base;
   int page_x;
   int page_y;
   int relative_x;
   int relative_y;
};

struct twrWidgetRequestAnimationFrameEvent {
   struct twrWidgetEventBase base;
   int delta;
};

struct twrWidgetWheelEvent {
   struct twrWidgetEventBase base;
   int delta_x;
   int delta_y;
   int delta_z;
   int delta_mode;
};

struct twrEventRegistration {
   enum twrWidgetEventType base_type;
   //returns true if it "captures" the input. In this case, the next widget doesn't get the event
   int (*callback)(struct twrWidgetEventBase*, void *);
};

struct twrWidgetBase {
   char* type;
   int x, y;
   int width, height;
   int visible;
   void (*draw)(struct d2d_draw_seq*, void *);
   void (*free)(void *);

   int num_events;
   struct twrEventRegistration* event_registrations; 
};

struct twrDoublyLinkedList;

struct twrRegisteredEvent {
   enum twrWidgetEventType base_type;
   int (*callback)(struct twrWidgetEventBase*, void*);
   struct twrWidgetBase* widget;
};

struct twrRegisteredWidget {
   struct twrWidgetBase* widget;

   int registered_events_len;
   struct twrDoublyLinkedList** registered_events;
};


struct twrDoublyLinkedList {
   struct twrDoublyLinkedList* next;
   struct twrDoublyLinkedList* prev;
   
   void* val;
};
struct twrDoublyLinkedListRoot {
   struct twrDoublyLinkedList* root;
   struct twrDoublyLinkedList* tail;
};

struct twrDoublyLinkedList* twr_double_linked_list_append(struct twrDoublyLinkedListRoot* list, void* val) {
   struct twrDoublyLinkedList* node = (struct twrDoublyLinkedList*)malloc(sizeof(struct twrDoublyLinkedList));
   node->next = NULL;
   node->prev = list->tail; //previous is tail
   node->val = val;
   
   //list is empty
   if (list->root == NULL) {
      list->root = node;
      list->tail = node;
      return node;
   }

   list->tail->next = node;
   list->tail = node;

   return node;
}

void twr_double_linked_list_remove(struct twrDoublyLinkedListRoot* list, struct twrDoublyLinkedList* node) {
   //only item in list
   if (list->root == node && list->tail == node) {
      list->root = NULL;
      list->tail = NULL;
   } else if (list->root == node) { //first item in list
      list->root = node->next;
      list->root->prev = NULL;
   } else if (list->tail == node) { //last item in list
      list->tail = node->prev;
      list->tail->next = NULL;
   } else { //in the middle of list
      node->prev->next = node->next;
      node->next->prev = node->prev;
   }

   free(node);
}

#define TWR_TOTAL_WIDGET_EVENT_TYPES TWR_LAST_WIDGET_EVENT_TYPE+1
struct twrWidgetManager {
   //stores twrRegisterdWidget
   struct twrDoublyLinkedListRoot widgets;

   //list of registered event handlers for widgets
   //stores Registered Event
   struct twrDoublyLinkedListRoot events[TWR_TOTAL_WIDGET_EVENT_TYPES];
   
};

struct twrRegisteredWidget* twr_widget_manager_register_widget(struct twrWidgetManager* manager, struct twrWidgetBase* widget) {
   struct DoublyLinkedList** events = (struct twrRegisteredEvent**)malloc(sizeof(struct DoublyLinkedList**) * widget->num_events);

   for (int i = 0; i < widget->num_events; i++) {
      struct twrRegisteredEvent* event = (struct twrRegisteredEvent*)malloc(sizeof(struct twrRegisteredEvent));
      event->base_type = widget->event_registrations[i].base_type;
      event->callback = widget->event_registrations[i].callback;
      event->widget = widget;

      struct DoublyLinkedList* event_node = twr_double_linked_list_append(&manager->events[event->base_type], (void*)event);
      events[i] = event_node;
   }


   struct twrRegisteredWidget* r_widget = (struct twrRegisteredWidget*)malloc(sizeof(struct twrRegisteredWidget));
   r_widget->registered_events = events;
   r_widget->registered_events_len = widget->num_events;
   r_widget->widget = widget;

   twr_double_linked_list_append(&manager->widgets, r_widget); 
   
   return r_widget;
}

void twr_widget_manager_send_event(struct twrWidgetManager* manager, enum twrWidgetEventType event_type, struct twrWidgetEventBase* event) {
   for (struct twrDoublyLinkedList* node = manager->events[event_type].root; node; node = node->next) {
      struct twrRegisteredEvent* event_handler = (struct twrRegisteredEvent*)(node->val);
      if (event_handler->callback(event, event_handler->widget)) {
         break; //if callback returns true, stop there
      }
   }
}

//should likely have a different method for sending events
//that way you can focus on a single text widget and 
void twr_widget_manager_keyboard_event(struct twrWidgetManager* manager, enum twrWidgetEventType event_type, int key) {
   struct twrWidgetKeyboardEvent event = {
      .base = {
         .type = event_type
      },
      .key = key,
   };
   twr_widget_manager_send_event(manager, event_type, &event.base);
}

void twr_widget_manager_mouse_event(struct twrWidgetManager* manager, enum twrWidgetEventType event_type, int page_x, int page_y, int relative_x, int relative_y) {
   struct twrWidgetMouseEvent event = {
      .base = {
         .type = event_type,
      },
      .page_x = page_x,
      .page_y = page_y,
      .relative_x = relative_x,
      .relative_y = relative_y
   };
   twr_widget_manager_send_event(manager, event_type, &event.base);
}

void twr_widget_manager_request_animation_frame_event(struct twrWidgetManager* manager, enum twrWidgetEventType event_type, int delta) {
   struct twrWidgetRequestAnimationFrameEvent event = {
      .base = {
         .type = event_type
      },
      .delta = delta,
   };
   twr_widget_manager_send_event(manager, event_type, &event.base);
}

void twr_widget_manager_wheel_event(struct twrWidgetManager* manager, enum twrWidgetEventType event_type, int delta_x, int delta_y, int delta_z, int delta_mode) {
   struct twrWidgetWheelEvent event = {
      .base = {
         .type = event_type,
      },
      .delta_x = delta_x,
      .delta_y = delta_y,
      .delta_z = delta_z,
      .delta_mode = delta_mode
   };
   twr_widget_manager_send_event(manager, event_type, &event.base);
}



struct twrWidgetButton {
   struct twrWidgetBase base;
   char* text;
   char* text_font;
   char* text_color;
   char* default_color;
   char* hover_color;
   void (*onclick)(void *);
   void* onclick_data;

   int hovering;

   int initialized;
   int text_x, text_y;
};


void twr_widget_button_draw(struct d2d_draw_seq* ds, void * self) {
   struct twrWidgetButton* button = (struct twrWidgetButton*)self;
   struct twrWidgetBase* base = &button->base;

   d2d_save(ds);
   d2d_setfont(ds, button->text_font);

   if (!button->initialized) {
      button->initialized = 1;
      //find text_x and text_y such that the provided text is centered
   }

   d2d_setfillstyle(ds, button->hovering ? button->hover_color : button->default_color);
   d2d_fillrect(ds, base->x, base->y, base->x + base->width, base->y + base->height);

   d2d_setfillstyle(ds, button->text_color);
   d2d_filltext(ds, button->text, button->text_x, button->text_y);
   
   d2d_restore(ds);
}

int twr_widget_button_event(struct twrWidgetEventBase* event, void * self) {
   assert(event->type == TWR_WIDGET_EVENT_MOUSE_CLICK || event->type == TWR_WIDGET_EVENT_MOUSE_MOVE);
   struct twrWidgetMouseEvent* mouse_event = (struct twrWidgetMouseEvent*)event;

   struct twrWidgetButton* button = (struct twrWidgetButton*)self;
   struct twrWidgetBase* base = &button->base;

   if (
      base->x <= mouse_event->relative_x && mouse_event->relative_x <= base->x + base->width
      && base->y <= mouse_event->relative_y && mouse_event->relative_y <= base->y + base->height
   ) {
      button->hovering = 1;
      if (event->type == TWR_WIDGET_EVENT_MOUSE_CLICK)
         button->onclick(button->onclick_data);
      return true;
   } else {
      button->hovering = 0;
      return false;
   }

}

struct twrEventRegistration* twr_create_dynamic_event_list(struct twrEventRegistration* events, int event_len) {
   size_t event_heap_size = sizeof(struct twrEventRegistration) * event_len;
   struct twrEventRegistration* event_heap = (struct twrEventRegistration*)malloc(event_heap_size);
   memcpy(event_heap, &events, event_len);

   return event_heap;
}
struct twrWidgetButton* new_button(int x, int y, int width, int height, char* text, char* text_font, char* text_color, char* default_color, char* hover_color, void (*onclick)(void *), void* onclick_data) {
   const int EVENT_LEN = 2;
   
   struct twrWidgetButton widget =  {
      .base = {
         .type = "twrWidgetButton",
         .x = x,
         .y = y,
         .width = width,
         .height = height,
         .visible = 1,
         .draw = twr_widget_button_draw,
         .free = NULL, //nothing to free
         
         .num_events = EVENT_LEN,
         .event_registrations = twr_create_dynamic_event_list(
            (struct twrEventRegistration[2]){
               {
                  .base_type = TWR_WIDGET_EVENT_MOUSE_CLICK,
                  .callback = twr_widget_button_event
               },
               {
                  .base_type = TWR_WIDGET_EVENT_MOUSE_MOVE,
                  .callback = twr_widget_button_event
               }
            },
            EVENT_LEN
         ),
      },
      .text = text,
      .text_font = text_font,
      .text_color = text_color,
      .default_color = default_color,
      .hover_color = hover_color,
      .onclick = onclick,
      .onclick_data = onclick_data,

      .hovering = 0,

      .initialized = 0,
   };

   struct twrWidgetButton* widget_heap = (struct twrWidgetButton*)malloc(sizeof(struct twrWidgetButton));
   memcpy(widget_heap, &widget, sizeof(struct twrWidgetButton));

   return widget_heap;
}
