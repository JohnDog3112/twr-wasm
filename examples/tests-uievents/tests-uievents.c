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