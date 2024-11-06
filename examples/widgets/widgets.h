#include <twr-draw2d.h>
#include <stdlib.h>

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
   void (*callback)(struct twrWidgetEventBase, void *);
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



struct twrDoublyLinkedList {
   struct twrDoublyLinkedList* next;
   struct twrDoublyLinkedList* prev;
   
   void* val;
};
struct twrDoublyLinkedListRoot {
   struct twrDoublyLinkedList* root;
   struct twrDoublyLinkedList* tail;
};

void twr_doubly_linked_list_append(struct twrDoublyLinkedListRoot* list, void* val) {
   struct twrDoublyLinkedList* node = (struct twrDoublyLinkedList*)malloc(sizeof(struct twrDoublyLinkedList));
   node->next = NULL;
   node->prev = list->tail; //previous is tail
   node->val = val;
   
   //list is empty
   if (list->root == NULL) {
      list->root = node;
      list->tail = node;
      return;
   }

   list->tail->next = node;
   list->tail = node;
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
   struct twrDoublyLinkedList widgets;
   struct twrDoublyLinkedList events[TWR_TOTAL_WIDGET_EVENT_TYPES];
};



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

void twr_widget_button_event(struct twrWidgetEventBase event, void * self) {
   assert(event.type == TWR_WIDGET_EVENT_MOUSE_CLICK || event.type == TWR_WIDGET_EVENT_MOUSE_MOVE);
   struct twrWidgetMouseEvent* mouse_event = (struct twrWidgetMouseEvent*)&event;

   struct twrWidgetButton* button = (struct twrWidgetButton*)self;
   struct twrWidgetBase* base = &button->base;

   if (
      base->x <= mouse_event->relative_x && mouse_event->relative_x <= base->x + base->width
      && base->y <= mouse_event->relative_y && mouse_event->relative_y <= base->y + base->height
   ) {
      button->hovering = 1;
      if (event.type == TWR_WIDGET_EVENT_MOUSE_CLICK)
         button->onclick(button->onclick_data);
   } else {
      button->hovering = 0;
   }

}
struct twrWidgetButton new_button(int x, int y, int width, int height, char* text, char* text_font, char* text_color, char* default_color, char* hover_color, void (*onclick)(void *), void* onclick_data) {
   return twrWidgetButton {
      .base = {
         .type = "twrWidgetButton",
         .x = x,
         .y = y,
         .width = width,
         .height = height,
         .visible = 1,
         .draw = twr_widget_button_draw,
         .free = NULL, //nothing to free
         
         .num_events = 2,
         .event_registrations = (struct twrEventRegistration[2]){
            {
               .base_type = TWR_WIDGET_EVENT_MOUSE_CLICK,
               .callback = twr_widget_button_event
            },
            {
               .base_type = TWR_WIDGET_EVENT_MOUSE_MOVE,
               .callback = twr_widget_button_event
            }
         }
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
}
