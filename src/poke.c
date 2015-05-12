#include <pebble.h>

typedef struct {
  int level;
  GBitmap *front;
  GBitmap *back;
  char name[30];
} Monster;

static Window *s_main_window;
static TextLayer *text_layers[2]; // Time, Date, You, You Level, Enemy, Enemy Level
static GFont s_custom_font_9;
static GFont s_custom_font_10;
static GFont s_custom_font_20;
static GBitmap *s_ui[3];
static BitmapLayer *s_ui_layer[3];
static GBitmap *s_backgrounds[4];
static BitmapLayer *s_background_layer;
static int background;
static Monster monsters[2];

static void update_time();
static void main_window_load(Window *window);
static void main_window_unload(Window *window);
static void tick_handler(struct tm *tick_time, TimeUnits units_changed);
static void init();
static void deinit();

static void create_fonts();
static void create_backgrounds();
char *upcase(char *str);
static void create_ui_elements();
static void set_font_layers();
static void set_ui_elements();
static void create_monsters();

int main(void) {
  init();
  app_event_loop();
  deinit();
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);

  srand(time(NULL));
  background = (rand() % 4);

  create_fonts();
  create_backgrounds();
  create_ui_elements();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

static void main_window_load(Window *window) {
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_backgrounds[background]);

  set_font_layers();
  set_ui_elements();
  create_monsters();

  // Add it as a children layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));

  int i = 0;
  while(i < 3) {
    layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_ui_layer[i]));
    i++;
  }

  i = 0;
  while(i < 2) {
    layer_add_child(window_get_root_layer(window), text_layer_get_layer(text_layers[i]));
    i++;
  }

  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  // Destroy FONTS
  fonts_unload_custom_font(s_custom_font_9);
  fonts_unload_custom_font(s_custom_font_10);
  fonts_unload_custom_font(s_custom_font_20);

  // Destroy BACKGROUNDS
  int i = 0;
  while(s_backgrounds[i]) {
    gbitmap_destroy(s_backgrounds[i]);
    i++;
  }
  bitmap_layer_destroy(s_background_layer);

  // Destroy UI ELEMENTS
  i = 0;
  while(s_ui[i]) {
    gbitmap_destroy(s_ui[i]);
    bitmap_layer_destroy(s_ui_layer[i]);
    i++;
  }

  // Destroy Texts
  i = 0;
  while(text_layers[i]) {
    text_layer_destroy(text_layers[i]);
    i++;
  }

  i = 0;
  while(monsters[i].front) {
    gbitmap_destroy(monsters[i].front);
    gbitmap_destroy(monsters[i].back);
    i++;
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char date_buffer[] = "01 JAN";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof(buffer), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof(buffer), "%I:%M", tick_time);
  }

  int tens = buffer[3] - '0';
  int ones = buffer[4] - '0';

  int current_mins = (tens * 10) + ones;

  if(current_mins % 15 == 0) {
    background++;
    if(background > 3) {
      background = 0;
    }
    bitmap_layer_set_bitmap(s_background_layer, s_backgrounds[background]);
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Interval: %d", current_mins);
  }

  strftime(date_buffer, sizeof(date_buffer), "%d %b", tick_time);
  upcase(date_buffer);

  // Display this time on the TextLayer
  text_layer_set_text(text_layers[0], buffer);
  text_layer_set_text(text_layers[1], date_buffer);
}

/*
* END OF MAIN FUNCTION DEFINITIONS
*/

static void create_monsters() {
  strcpy(monsters[0].name, "Pikachu");
  monsters[0].level = 1;
  monsters[0].front = gbitmap_create_with_resource(RESOURCE_ID_MONSTER_FRONT_PIKACHU);
  monsters[0].back = gbitmap_create_with_resource(RESOURCE_ID_MONSTER_FRONT_PIKACHU);
}

char *upcase(char *str) {
    for (int i = 0; str[i] != 0; i++) {
        if (str[i] >= 'a' && str[i] <= 'z') {
            str[i] -= 0x20;
        }
    }

    return str;
}

static void create_fonts() {
  s_custom_font_9 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKE_9));
  s_custom_font_10 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKE_10));
  s_custom_font_20 = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKE_20));
}

static void create_backgrounds() {
  // BACKGROUNDS
  s_backgrounds[0] = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_CAVE);
  s_backgrounds[1] = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_GRASS);
  s_backgrounds[2] = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_SAND);
  s_backgrounds[3] = gbitmap_create_with_resource(RESOURCE_ID_BACKGROUND_WATER);
}

static void create_ui_elements() {
  // UI ELEMENTS
  s_ui[0] = gbitmap_create_with_resource(RESOURCE_ID_STATUS_BOTTOM_BOX);
  s_ui[1] = gbitmap_create_with_resource(RESOURCE_ID_STATUS_MAIN_HEALTH);
  s_ui[2] = gbitmap_create_with_resource(RESOURCE_ID_STATUS_ENEMY_HEALTH);
}

static void set_font_layers() {
  // Create time TextLayer
  text_layers[0] = text_layer_create(GRect(5, 138, 50, 30));
  text_layer_set_background_color(text_layers[0], GColorClear);
  text_layer_set_text_color(text_layers[0], GColorBlack);
  text_layer_set_text(text_layers[0], "22:22");
  text_layer_set_font(text_layers[0], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_layers[0], GTextAlignmentLeft);

  text_layers[1] = text_layer_create(GRect(64, 138, 65, 30));
  text_layer_set_background_color(text_layers[1], GColorClear);
  text_layer_set_text_color(text_layers[1], GColorBlack);
  text_layer_set_text(text_layers[1], "22 JAN");
  text_layer_set_font(text_layers[1], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(text_layers[1], GTextAlignmentRight);
}

static void set_ui_elements() {
  s_ui_layer[0] = bitmap_layer_create(GRect(0, 142, 144, 26));
  bitmap_layer_set_bitmap(s_ui_layer[0], s_ui[0]);
  bitmap_layer_set_compositing_mode(s_ui_layer[0], GCompOpSet);

  s_ui_layer[1] = bitmap_layer_create(GRect(62, 90, 82, 34));
  bitmap_layer_set_bitmap(s_ui_layer[1], s_ui[1]);
  bitmap_layer_set_compositing_mode(s_ui_layer[1], GCompOpSet);

  s_ui_layer[2] = bitmap_layer_create(GRect(0, 12, 82, 26));
  bitmap_layer_set_bitmap(s_ui_layer[2], s_ui[2]);
  bitmap_layer_set_compositing_mode(s_ui_layer[2], GCompOpSet);
}
