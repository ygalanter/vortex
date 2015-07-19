#include <pebble.h>
#include "main.h"  

static Window *s_main_window;

// for battery & bt
static GBitmap *battery_sprite = NULL, *bluetooth_sprite = NULL;


// for apng animation
#ifdef PBL_COLOR
  static GBitmap *s_bitmap = NULL;
  static BitmapLayer *s_bitmap_layer;
  static GBitmapSequence *s_sequence = NULL;
  int frame_no;    
  #define NO_OF_FRAMES 38 
#endif    
    
// time and date holders
GFont makisupa_big, makisupa_small;
char s_time[] = "     ";

// date and DoW
char buffer_date[] = "      ";
char buffer_dow[] = "   ";

Layer *back_layer; // layer to hold time spheres as well as additional layers  

// flags
bool is_vortex_animating = false;
int flag_show_digital_time = 0, flag_show_bluetooth = 0, flag_show_battery = 0, flag_show_date = 0, flag_show_dow = 0, flag_show_seconds = 0, flag_disable_vortex_animation = 0;

// bluetooth update
void bluetooth_handler(bool connected) {
  
  if (flag_show_bluetooth == 0) return;
  
   if (bluetooth_sprite != NULL) {
     gbitmap_destroy(bluetooth_sprite);
     bluetooth_sprite = NULL;
   }
  
  if (connected) {
    bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_C);
  } else {
    bluetooth_sprite = gbitmap_create_with_resource(RESOURCE_ID_BLUETOOTH_D);
  }
  
  layer_mark_dirty(back_layer);
  
}

// battery update
static void battery_handler(BatteryChargeState state) {
  
  if (flag_show_battery == 0) return;
  
   if (battery_sprite != NULL) {
     gbitmap_destroy(battery_sprite);
     battery_sprite = NULL;
   }
  
   // creating bitmap image based on charge percentage
   if (state.is_charging) {
     battery_sprite = gbitmap_create_with_resource(battery_c_res[state.charge_percent/10]);  
   } else {
     battery_sprite = gbitmap_create_with_resource(battery_d_res[state.charge_percent/10]);
   }
   
  
  layer_mark_dirty(back_layer);

}


// handle configuration change
static void in_recv_handler(DictionaryIterator *iterator, void *context) {
  Tuple *t = dict_read_first(iterator);

  while (t)  {
    
    switch(t->key)    {
     
        case KEY_SHOW_DIGITAL_TIME:
           persist_write_int(KEY_SHOW_DIGITAL_TIME, t->value->uint8);
           flag_show_digital_time = t->value->uint8;
           break;
        case KEY_SHOW_BLUETOOTH:
           persist_write_int(KEY_SHOW_BLUETOOTH, t->value->uint8);
           flag_show_bluetooth = t->value->uint8;
           if (flag_show_bluetooth == 1) bluetooth_handler(bluetooth_connection_service_peek());
      break;
        case KEY_SHOW_DATE:
           persist_write_int(KEY_SHOW_DATE, t->value->uint8);
           flag_show_date = t->value->uint8;
           break;
        case KEY_SHOW_DOW:
           persist_write_int(KEY_SHOW_DOW, t->value->uint8);
           flag_show_dow = t->value->uint8;
           break;
         case KEY_SHOW_BATTERY:
           persist_write_int(KEY_SHOW_BATTERY, t->value->uint8);
           flag_show_battery = t->value->uint8;
           if (flag_show_battery == 1)  battery_handler(battery_state_service_peek());
           break;
         case KEY_SHOW_SECONDS:
           persist_write_int(KEY_SHOW_SECONDS, t->value->uint8);
           flag_show_seconds = t->value->uint8;
           break;
        case KEY_DISABLE_VORTEX_ANIMATION:
           flag_disable_vortex_animation = t->value->uint8;
           persist_write_int(KEY_DISABLE_VORTEX_ANIMATION, t->value->uint8);
           break;
    }    
    
    t = dict_read_next(iterator);
  
  }
  
  layer_mark_dirty(back_layer);
}  


// updates drawable graphics, digital time
static void back_update_proc(Layer *layer, GContext *ctx) {
  
  // precaution: when vortex is animating back layer is hidden and shouldn't update, but this is just in case
  if (is_vortex_animating) return;
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  
  //******************************* Drawing time spheres
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  #ifndef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, center, 53);
    graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, center, 50);
  #endif

  int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  int32_t second_angle = TRIG_MAX_ANGLE * t->tm_sec / 60;
  
  int currY;
  int currX;
  
  // hour
  currY = -53 * cos_lookup(hour_angle) / TRIG_MAX_RATIO + center.y;
  currX = 53 * sin_lookup(hour_angle) / TRIG_MAX_RATIO + center.x;
  
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorPurple); graphics_fill_circle(ctx, GPoint(currX, currY), 11);
  #else
    graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 11);
    graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, GPoint(currX, currY), 4);
  #endif
  
  // minute
  currY = -53 * cos_lookup(minute_angle) / TRIG_MAX_RATIO + center.y;
  currX = 53 * sin_lookup(minute_angle) / TRIG_MAX_RATIO + center.x;
  #ifdef PBL_COLOR
    graphics_context_set_fill_color(ctx, GColorCyan); graphics_fill_circle(ctx, GPoint(currX, currY), 8);
  #else
    graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 8);
    graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, GPoint(currX, currY), 5);
  #endif
  
  // second
  if (flag_show_seconds == 1) {
    currY = -53 * cos_lookup(second_angle) / TRIG_MAX_RATIO + center.y;
    currX = 53 * sin_lookup(second_angle) / TRIG_MAX_RATIO + center.x;
    #ifdef PBL_COLOR
      graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 5);
    #else
      graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 5);
    #endif
  }  
  
  
  // ******************************** displaying digital time
  if (flag_show_digital_time == 1) {
    #ifdef PBL_COLOR
      graphics_context_set_text_color(ctx, GColorMalachite);
    #else
      graphics_context_set_text_color(ctx, GColorWhite);
    #endif
    
     if (!clock_is_24h_style()) {
    
        if(t->tm_hour > 11 ) {   // YG Jun-25-2014: 0..11 - am 12..23 - pm
            if(t->tm_hour > 12 ) t->tm_hour -= 12;
            graphics_draw_text(ctx, "pm", makisupa_big, GRect(33, 91, 85, 40), GTextOverflowModeWordWrap, GTextAlignmentCenter,  NULL);
        } else {
            if(t->tm_hour == 0 ) t->tm_hour = 12;
            graphics_draw_text(ctx, "am", makisupa_big, GRect(33, 91, 85, 40), GTextOverflowModeWordWrap, GTextAlignmentCenter,  NULL);
        }        
    }
   
    // time
    strftime(s_time, sizeof(s_time), "%H:%M", t);
    graphics_draw_text(ctx, s_time, makisupa_big, GRect(30, 67, 85, 40), GTextOverflowModeWordWrap, GTextAlignmentCenter,  NULL);
  }  
  
  
  // ******************************** displaying battery info
  if (flag_show_battery == 1) {
    if (battery_sprite != NULL) graphics_draw_bitmap_in_rect(ctx, battery_sprite, GRect(113, 5, 29, 18));
  }  
  
  // ******************************** displaying bluetooth info
  if (flag_show_bluetooth == 1) {
    if (bluetooth_sprite != NULL) graphics_draw_bitmap_in_rect(ctx, bluetooth_sprite, GRect(3, 3, 18, 24));
  }  
  
  
  // ******************************** displaying digital date & dow
  graphics_context_set_text_color(ctx, GColorWhite);
  
  //date
  if (flag_show_date == 1) {
    strftime(buffer_date, sizeof("SEP 31"), "%b %d", t);
    graphics_draw_text(ctx, buffer_date, makisupa_small, GRect(72, 148, 69, 20), GTextOverflowModeWordWrap, GTextAlignmentRight,  NULL);
  }  
  
  //dow
  if (flag_show_dow == 1) {
    strftime(buffer_dow, sizeof("SAT"), "%a", t);
    graphics_draw_text(ctx, buffer_dow, makisupa_small, GRect(3, 148, 50, 20), GTextOverflowModeWordWrap, GTextAlignmentLeft,  NULL);
  }
}

// function to handle animation timer
#ifdef PBL_COLOR

  static void timer_handler(void *context) {
  uint32_t next_delay;

  // Advance to the next APNG frame (using frame counter, because this APNG is looped)
  if(frame_no <= NO_OF_FRAMES) {
    gbitmap_sequence_update_bitmap_next_frame(s_sequence, s_bitmap, &next_delay);
    bitmap_layer_set_bitmap(s_bitmap_layer, s_bitmap);
    layer_mark_dirty(bitmap_layer_get_layer(s_bitmap_layer));

    // Timer for that delay
    frame_no++;
    app_timer_register(next_delay, timer_handler, NULL);
  } else {
    layer_set_hidden(back_layer, false); // when animation sequence ends - restore visibility of the info layer
    is_vortex_animating = false;
    gbitmap_sequence_restart(s_sequence); // on next run sequence will start from the beginning
  }
}

// initiating APNG animation sequence
static void load_sequence() {
   
  // Begin animation
  layer_set_hidden(back_layer, true); // hiding additional info for duration of animation
  is_vortex_animating = true;
  frame_no = 1;
  app_timer_register(1, timer_handler, NULL);
}

#endif


// on timer tick initiate animation sequence
static void time_timer_tick(struct tm *tick_time, TimeUnits units_changed) {
  
  if (units_changed & MINUTE_UNIT) {  // if minute changed - display animation
    #ifdef PBL_PLATFORM_BASALT
    if (flag_disable_vortex_animation == 0) { // if enabled - load animation
      load_sequence();  
    } else {
      layer_mark_dirty(back_layer); // otherwise update time layer
    }
    #else
      layer_mark_dirty(back_layer); // on aplite just update time layer
    #endif
  }
  else if ((units_changed & SECOND_UNIT)) { // if second changed - update layer to display new second position
    if (flag_show_seconds == 1) {
      layer_mark_dirty(back_layer);
    }  
  } 
  
  
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  #ifdef PBL_COLOR
    s_bitmap_layer = bitmap_layer_create(GRect(11, 23, 123, 123));
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  #endif
  
  // init hands
  back_layer = layer_create(GRect(0, 0, 144, 168));
  #ifdef PBL_COLOR
    layer_set_hidden(back_layer, true); // initialy hide it - it will appear after initial animation
  #endif
  layer_set_update_proc(back_layer, back_update_proc);
  layer_add_child(window_layer, back_layer);
  
  //loading fonts
  makisupa_big = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MAKISUPA_30));
  makisupa_small  = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MAKISUPA_18));
  
  // prepping battery & bluetootj info
  battery_handler(battery_state_service_peek());
  bluetooth_handler(bluetooth_connection_service_peek());
  
  #ifdef PBL_COLOR
    // preparing vortex animation resources
    s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);
    s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);
    
    load_sequence();  //showing initial animation
  #endif 

}


static void main_window_unload(Window *window) {
  
  // destroying vortex animation resources
  #ifdef PBL_COLOR
    if(s_sequence) {
      gbitmap_sequence_destroy(s_sequence);
      s_sequence = NULL;
    }
    if(s_bitmap) {
      gbitmap_destroy(s_bitmap);
      s_bitmap = NULL;
    }
  
    bitmap_layer_destroy(s_bitmap_layer);
  #endif
  
  // destroying battery & bluetooth
  if (battery_sprite) {
     gbitmap_destroy(battery_sprite);
     battery_sprite = NULL;
   }
   if (bluetooth_sprite) {
     gbitmap_destroy(bluetooth_sprite);
     battery_sprite = NULL;
   }
  
  layer_destroy(back_layer);
  fonts_unload_custom_font(makisupa_big);
  fonts_unload_custom_font(makisupa_small);
  
}

static void init() {
  
  char *sys_locale = setlocale(LC_ALL, "");
  // we're not supporting chinese yet
  if (strcmp("zh_CN", sys_locale) || strcmp("zh_TW", sys_locale) == 0) {
    setlocale(LC_ALL, "en_US");
  }
  
  //reading flag settings
  flag_show_digital_time = persist_read_int(KEY_SHOW_DIGITAL_TIME)? persist_read_int(KEY_SHOW_DIGITAL_TIME) : 0;
  flag_show_bluetooth = persist_read_int(KEY_SHOW_BLUETOOTH)? persist_read_int(KEY_SHOW_BLUETOOTH) : 0;
  flag_show_battery = persist_read_int(KEY_SHOW_BATTERY)? persist_read_int(KEY_SHOW_BATTERY) : 0;
  flag_show_date = persist_read_int(KEY_SHOW_DATE)? persist_read_int(KEY_SHOW_DATE) : 0;
  flag_show_dow = persist_read_int(KEY_SHOW_DOW)? persist_read_int(KEY_SHOW_DOW) : 0;
  flag_show_seconds = persist_read_int(KEY_SHOW_SECONDS)? persist_read_int(KEY_SHOW_SECONDS) : 0;
  flag_disable_vortex_animation = persist_read_int(KEY_DISABLE_VORTEX_ANIMATION)? persist_read_int(KEY_DISABLE_VORTEX_ANIMATION) : 0;
  
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(SECOND_UNIT, time_timer_tick);
  battery_state_service_subscribe(battery_handler);
  bluetooth_connection_service_subscribe(bluetooth_handler);
  
  // subscribing to JS messages
  app_message_register_inbox_received(in_recv_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
}

static void deinit() {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  app_message_deregister_callbacks();
}

int main() {
  init();
  app_event_loop();
  deinit();
}