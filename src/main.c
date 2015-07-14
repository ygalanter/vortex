#include <pebble.h>

#define NO_OF_FRAMES 38 
int frame_no;  

static Window *s_main_window;

static GBitmap *s_bitmap = NULL;
static BitmapLayer *s_bitmap_layer;
static GBitmapSequence *s_sequence = NULL;
Layer *back_layer;

static void back_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);

  int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  
  // minute
  int currY = -46 * cos_lookup(minute_angle) / TRIG_MAX_RATIO + center.y - 7;
  int currX = 46 * sin_lookup(minute_angle) / TRIG_MAX_RATIO + center.x;
  graphics_context_set_fill_color(ctx, GColorCyan); graphics_fill_circle(ctx, GPoint(currX, currY), 8);
 
  // hour
  currY = -54 * cos_lookup(hour_angle) / TRIG_MAX_RATIO + center.y -7;
  currX = 54 * sin_lookup(hour_angle) / TRIG_MAX_RATIO + center.x;
  graphics_context_set_fill_color(ctx, GColorPurple); graphics_fill_circle(ctx, GPoint(currX, currY), 11);
  
}


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
    layer_set_hidden(back_layer, false);
  }
}

static void load_sequence() {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }

  // Create sequence
  s_sequence = gbitmap_sequence_create_with_resource(RESOURCE_ID_ANIMATION);

  // Create GBitmap
  s_bitmap = gbitmap_create_blank(gbitmap_sequence_get_bitmap_size(s_sequence), GBitmapFormat8Bit);

   
  
  // Begin animation
  layer_set_hidden(back_layer, true);
  frame_no = 1;
  app_timer_register(1, timer_handler, NULL);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  load_sequence();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  
  s_bitmap_layer = bitmap_layer_create(GRect(11, 23, 123, 123));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bitmap_layer));
  
  // init hands
  back_layer = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(back_layer, back_update_proc);
  layer_add_child(window_layer, back_layer);

}


static void main_window_unload(Window *window) {
  // Free old data
  if(s_sequence) {
    gbitmap_sequence_destroy(s_sequence);
    s_sequence = NULL;
  }
  if(s_bitmap) {
    gbitmap_destroy(s_bitmap);
    s_bitmap = NULL;
  }
  
  bitmap_layer_destroy(s_bitmap_layer);
  layer_destroy(back_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit() {
  window_destroy(s_main_window);
  tick_timer_service_unsubscribe();
}

int main() {
  init();
  app_event_loop();
  deinit();
}