#ifndef __TWR_LIBUI_H__
#define __TWR_LIBUI_H__

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((import_name("twrRegisterGlobalKeyEvent"))) 
int twr_register_global_key_event(const char* event_name, int event_id);
__attribute__((import_name("twrRegisterLocalKeyEvent"))) 
int twr_register_local_key_event(const char* event_name, int event_id, const char* element_id);

__attribute__((import_name("registerAnimationLoop")))
int register_animation_loop(int event_id);

__attribute__((import_name("twrRegisterGlobalMouseEvent"))) 
int twr_register_global_mouse_event(const char* event_name, int event_id);
__attribute__((import_name("twrRegisterLocalMouseEvent"))) 
int twr_register_local_mouse_event(const char* event_name, int event_id, const char* element_id);

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