/* -- Specify TZ settings here -- */

// 0 is your local time

#define TZ_OFFSETS {0, 15, 18, 4, 7}
#define TZ_NAMES {"PT", "Per", "Syd", "Bue", "UTC"}

/* -------------------------------------------------- */

#include <pebble.h>

#define PEBBLE_OFFS_X 32
#define PEBBLE_OFFS_Y 0
  
#define COL_COUNT 3
#define ROW_COUNT 5
  
#define FIELD_COUNT (COL_COUNT * ROW_COUNT)

#define LABEL_MAX 5
  
#define HR_MAX 3

  
static int y_offsets[ROW_COUNT] = TZ_OFFSETS;
static char y_names[ROW_COUNT][LABEL_MAX] = TZ_NAMES; 
  
static int COL_OFFS[COL_COUNT] = {0, 37, 75};

static int COL_SIZES[COL_COUNT] = {37, 38, 37};

static int ROW_OFFS[ROW_COUNT] = {0, 32, 62, 92, 122};

static int ROW_SIZES[ROW_COUNT] = {32, 30, 30, 30, 30};

static int shift = 48;


static int x_offsets[COL_COUNT] = {-1, 0, 1};

static char field_values[FIELD_COUNT][HR_MAX];

static int current_hr;

static int hr;

static TextLayer *text_fields[FIELD_COUNT];

static TextLayer *text_labels[ROW_COUNT];

static Window *s_main_window;  

static void initialize_text_fields() {
  for (int y = 0; y < ROW_COUNT; y++) {
    for (int x = 0; x < COL_COUNT; x++) {
      TextLayer *textfield = text_layer_create(GRect(PEBBLE_OFFS_X + COL_OFFS[x], PEBBLE_OFFS_Y + ROW_OFFS[y], COL_SIZES[x], ROW_SIZES[y]));
      text_layer_set_background_color(textfield, GColorBlack);
      text_layer_set_text_color(textfield, GColorWhite);
      text_layer_set_font(textfield, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
      text_layer_set_text_alignment(textfield, GTextAlignmentCenter);
      layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(textfield));
      text_fields[(y * COL_COUNT) + x] = textfield;
    }
    TextLayer *textlabel = text_layer_create(GRect(0, PEBBLE_OFFS_Y + ROW_OFFS[y], 32, ROW_SIZES[y]));
    text_layer_set_background_color(textlabel, GColorBlack);
    text_layer_set_text_color(textlabel, GColorWhite);
    text_layer_set_font(textlabel, fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(textlabel, GTextAlignmentLeft);
    text_layer_set_text(textlabel, y_names[y]);
    layer_add_child(window_get_root_layer(s_main_window), text_layer_get_layer(textlabel));
    text_labels[y] = textlabel;
  }
}

static void update_text_fields() {
  for (int i = 0; i < FIELD_COUNT; i++) {
    TextLayer *textfield = text_fields[i];
    int x = i % COL_COUNT;
    int y = i / COL_COUNT;
    int t = ((hr + shift + x_offsets[x] + y_offsets[y]) + 48) % 24;
    int ch = ((hr + shift + x_offsets[1] + y_offsets[y]) + 48) % 24;
    snprintf(field_values[i], HR_MAX, "%d", t);
    text_layer_set_text(textfield, field_values[i]);
    if (ch >= 18 || ch <= 6) {
      text_layer_set_background_color(textfield, GColorBlack);
      text_layer_set_text_color(textfield, GColorWhite);
    }
    else {
      text_layer_set_background_color(textfield, GColorWhite);
      text_layer_set_text_color(textfield, GColorBlack);      
    }
  }
}

static void destroy_text_fields() {
  for (int i = 0; i < FIELD_COUNT; i++) {
    text_layer_destroy(text_fields[i]);
  }
  for (int y = 0; y < ROW_COUNT; y++) {
    text_layer_destroy(text_labels[y]);
  }
}

static void sync_hour() {
  hr = current_hr;
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  
  current_hr = tick_time->tm_hour;
}  

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  initialize_text_fields();
}

static void main_window_unload(Window *window) {
  destroy_text_fields();
}

static void down_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  shift = ((shift + 1) % 24) + 48;
  update_text_fields();
}

static void up_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  shift = ((shift - 1) % 24) + 48;
  update_text_fields();
}

static void select_single_click_handler(ClickRecognizerRef recognizer, void *context) {
  shift = 48;
  sync_hour();
  update_text_fields();
}

static void config_provider(Window *window) {
 // single click / repeat-on-hold config:
  window_single_click_subscribe(BUTTON_ID_DOWN, down_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_SELECT, select_single_click_handler);
}

static void init() {
  // Register with TickTimerService
  tick_timer_service_subscribe(HOUR_UNIT, tick_handler);
  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  window_set_click_config_provider(s_main_window, (ClickConfigProvider) config_provider);  
  
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
  sync_hour();
  update_text_fields();
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

