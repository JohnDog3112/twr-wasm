#include <twr-draw2d.h>
#include <stdlib.h>

enum twrWidgetEventType {
   TWR_WIDGET_EVENT_KEYBOARD,
   TWR_WIDGET_EVENT_MOUSE,
   TWR_WIDGET_REQUEST_ANIMATION_FRAME,
   TWR_WIDGET_WHEEL,
};
struct twrWidgetEventBase {
   enum twrWidgetEventType type;
};

enum twrWidgetKeyboardEventType {
   TWR_WIDGET_EVENT_KEY_UP,
   TWR_WIDGET_EVENT_KEY_DOWN
};
struct twrWidgetKeyboardEvent {
   struct twrWidgetEventBase base;
   enum twrWidgetKeyboardEventType type;
   int key;
};

enum twrWidgetMouseEventType {
   TWR_WIDGET_EVENT_MOUSE_MOVE,
   TWR_WIDGET_EVENT_MOUSE_DOWN,
   TWR_WIDGET_EVENT_MOUSE_UP,
   TWR_WIDGET_EVENT_MOUSE_CLICK,
   TWR_WIDGET_EVENT_MOUSE_DBLCLICK,
};
struct twrWidgetMouseEvent {
   struct twrWidgetEventBase base;
   enum twrWidgetMouseEventType type;
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

union twrEventRegistrationUnion {
   enum twrWidgetKeyboardEventType keyboard_type;
   enum twrWidgetMouseEventType mouse_type;
};
struct twrEventRegistration {
   enum twrWidgetEventType base_type;
   union twrEventRegistrationUnion secondary_type;
   void (*event)(struct twrWidgetEventBase, void *);
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



struct twrWidgetButton {
   struct twrWidgetBase base;
   char* text;
   char* text_font;
   char* text_color;
   char* default_color;
   char* hover_color;
   void (*onclick)(void *);
   void* onclick_data;

   int initialized;
   int text_x, text_y;
};


void twr_widget_button_draw(struct d2d_draw_seq* ds, void * self) {
   struct twrWidgetButton* button = (struct twrWidgetButton*)self;

   d2d_save(ds);
   d2d_setfont(ds, button->text_font);
   d2d_setfillstyle(ds, button->text_color);

   if (!button->initialized) {
      button->initialized = 1;
      //find text_x and text_y such that the provided text is centered
   }

   

   d2d_filltext(ds, button->text, button->text_x, button->text_y);
   
   d2d_restore(ds);
}
void twr_widget_button_event(struct twrWidgetEventBase event, void * self) {
   assert(event.type == TWR_WIDGET_EVENT_MOUSE_CLICK);
   struct twrWidgetMouseEvent* mouse_event = (struct twrWidgetMouseEvent*)&event;

   struct twrWidgetButton* button = (struct twrWidgetButton*)self;
   struct twrWidgetBase* base = &button->base;

   if (
      base->x <= mouse_event->relative_x && mouse_event->relative_x <= base->x + base->width
      && base->y <= mouse_event->relative_y && mouse_event->relative_y <= base->y + base->height
   ) {
      button->onclick(button->onclick_data);
   }

}
void twr_widget_button_free(void * self) {
   free(self);
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
         .free = twr_widget_button_free,
         
         .num_events = 1,
         .event_registrations = &(struct twrEventRegistration){
            .base_type = TWR_WIDGET_EVENT_MOUSE,
            .secondary_type = {
               .mouse_type = TWR_WIDGET_EVENT_MOUSE_CLICK
            },
            .event = twr_widget_button_event
         }
      },
      .text = text,
      .text_font = text_font,
      .text_color = text_color,
      .default_color = default_color,
      .hover_color = hover_color,
      .onclick = onclick,
      .onclick_data = onclick_data,

      .initialized = 0,
   };
}
