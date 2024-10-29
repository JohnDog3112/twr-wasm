#ifndef __TWR_LIBUI_H__
#define __TWR_LIBUI_H__

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((import_name("registerGlobalKeyUpEvent"))) 
int register_global_key_up_event(int event_id);
__attribute__((import_name("registerLocalKeyUpEvent"))) 
int register_local_key_up_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalKeyDownEvent"))) 
int register_global_key_down_event(int event_id);
__attribute__((import_name("registerLocalKeyDownEvent"))) 
int register_local_key_down_event(int event_id, const char* element_id);

__attribute__((import_name("registerAnimationLoop")))
int register_animation_loop(int event_id);

__attribute__((import_name("registerGlobalMouseMoveEvent"))) 
int register_global_mouse_move_event(int event_id);
__attribute__((import_name("registerLocalMouseMoveEvent"))) 
int register_local_mouse_move_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalMouseDownEvent"))) 
int register_global_mouse_down_event(int event_id);
__attribute__((import_name("registerLocalMouseDownEvent"))) 
int register_local_mouse_down_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalMouseUpEvent"))) 
int register_global_mouse_up_event(int event_id);
__attribute__((import_name("registerLocalMouseUpEvent"))) 
int register_local_mouse_up_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalMouseClickEvent"))) 
int register_global_mouse_click_event(int event_id);
__attribute__((import_name("registerLocalMouseClickEvent"))) 
int register_local_mouse_click_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalMouseDoubleClickEvent"))) 
int register_global_mouse_double_click_event(int event_id);
__attribute__((import_name("registerLocalMouseDoubleClickEvent"))) 
int register_local_mouse_double_click_event(int event_id, const char* element_id);
__attribute__((import_name("registerGlobalWheelEvent"))) 
int register_global_wheel_event(int event_id);
__attribute__((import_name("registerLocalWheelEvent"))) 
int register_local_wheel_event(int event_id, const char* element_id);

__attribute__((import_name("stopUIEvent")))
void stop_ui_event(int event_handler_id);
__attribute__((import_name("stopAllUIEvents")))
void stop_all_ui_events();


#ifdef __cplusplus
}
#endif

#endif