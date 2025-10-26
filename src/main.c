/**
 * LVGL Watch Application
 * Features: Loading screen with "Welcome" text, followed by time display
 * Target: 1.47" display (172x320) - Currently running on SDL2 emulator
 */

#include "lvgl/lvgl.h"
#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>

// Display settings for 1.47" screen
#define SCREEN_WIDTH  172
#define SCREEN_HEIGHT 320

// SDL window and renderer
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// UI objects
static lv_obj_t *loading_screen = NULL;
static lv_obj_t *time_screen = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *date_label = NULL;
static lv_timer_t *loading_timer = NULL;
static lv_timer_t *clock_timer = NULL;

// Function declarations
static void create_loading_screen(void);
static void create_time_screen(void);
static void loading_timer_cb(lv_timer_t *timer);
static void clock_update_cb(lv_timer_t *timer);
static void update_time_display(void);
static void sdl_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
static void sdl_mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

/**
 * Create the loading screen with "Welcome" text
 */
static void create_loading_screen(void)
{
    // Create loading screen container
    loading_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(loading_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(loading_screen, lv_color_black(), 0);
    lv_obj_set_style_border_width(loading_screen, 0, 0);
    lv_obj_clear_flag(loading_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(loading_screen, LV_ALIGN_CENTER, 0, 0);

    // Create "Welcome" label
    lv_obj_t *welcome_label = lv_label_create(loading_screen);
    lv_label_set_text(welcome_label, "Welcome");
    lv_obj_set_style_text_color(welcome_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(welcome_label, &lv_font_montserrat_24, 0);
    lv_obj_align(welcome_label, LV_ALIGN_CENTER, 0, 0);

    // Create timer to switch to time screen after 4 seconds
    loading_timer = lv_timer_create(loading_timer_cb, 4000, NULL);
    lv_timer_set_repeat_count(loading_timer, 1);
}

/**
 * Create the time display screen
 */
static void create_time_screen(void)
{
    // Create time screen container
    time_screen = lv_obj_create(lv_scr_act());
    lv_obj_set_size(time_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(time_screen, lv_color_black(), 0);
    lv_obj_set_style_border_width(time_screen, 0, 0);
    lv_obj_clear_flag(time_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(time_screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_flag(time_screen, LV_OBJ_FLAG_HIDDEN); // Hide initially

    // Create time label (HH:MM:SS)
    time_label = lv_label_create(time_screen);
    lv_label_set_text(time_label, "00:00:00");
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_28, 0);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -20);

    // Create date label
    date_label = lv_label_create(time_screen);
    lv_label_set_text(date_label, "");
    lv_obj_set_style_text_color(date_label, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 30);

    // Update time immediately
    update_time_display();

    // Create timer to update clock every second
    clock_timer = lv_timer_create(clock_update_cb, 1000, NULL);
}

/**
 * Timer callback to switch from loading to time screen
 */
static void loading_timer_cb(lv_timer_t *timer)
{
    // Hide loading screen
    lv_obj_add_flag(loading_screen, LV_OBJ_FLAG_HIDDEN);
    
    // Show time screen
    lv_obj_clear_flag(time_screen, LV_OBJ_FLAG_HIDDEN);
    
    printf("Switched to time display\n");
}

/**
 * Timer callback to update clock display
 */
static void clock_update_cb(lv_timer_t *timer)
{
    update_time_display();
}

/**
 * Update the time display with current system time
 */
static void update_time_display(void)
{
    time_t now;
    struct tm *timeinfo;
    char time_str[32];
    char date_str[64];

    // Get current time
    time(&now);
    timeinfo = localtime(&now);

    // Format time (HH:MM:SS)
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    lv_label_set_text(time_label, time_str);

    // Format date (Day, Mon DD YYYY)
    strftime(date_str, sizeof(date_str), "%a, %b %d %Y", timeinfo);
    lv_label_set_text(date_label, date_str);
}

/**
 * SDL display flush callback
 */
static void sdl_display_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    // Update pixels in the specified area
    for(int y = area->y1; y <= area->y2; y++) {
        for(int x = area->x1; x <= area->x2; x++) {
            int idx = (y - area->y1) * (area->x2 - area->x1 + 1) + (x - area->x1);
            lv_color_t c = color_p[idx];
            
            SDL_SetRenderDrawColor(renderer, 
                LV_COLOR_GET_R(c) << 3,  // 5-bit to 8-bit
                LV_COLOR_GET_G(c) << 2,  // 6-bit to 8-bit
                LV_COLOR_GET_B(c) << 3,  // 5-bit to 8-bit
                0xFF);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }

    lv_disp_flush_ready(disp_drv);
}

/**
 * SDL mouse read callback
 */
static void sdl_mouse_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    int x, y;
    uint32_t mouse_state = SDL_GetMouseState(&x, &y);
    
    // Scale mouse coordinates back to logical size
    SDL_RenderGetLogicalSize(renderer, NULL, NULL);
    
    if(mouse_state & SDL_BUTTON_LMASK) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    
    data->point.x = x;
    data->point.y = y;
}

/**
 * Main function
 */
int main(int argc, char **argv)
{
    printf("Starting LVGL Watch Application...\n");
    printf("Target display: 1.47\" (%dx%d)\n", SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize SDL2
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create SDL window
    window = SDL_CreateWindow("LVGL Watch Simulator",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH * 2,  // Scale 2x for easier viewing
                              SCREEN_HEIGHT * 2,
                              SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Enable logical size to match display resolution
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Initialize LVGL
    lv_init();

    // Create display buffer
    static lv_color_t buf1[SCREEN_WIDTH * 10];
    static lv_color_t buf2[SCREEN_WIDTH * 10];
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, SCREEN_WIDTH * 10);

    // Register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = sdl_display_flush;
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    lv_disp_drv_register(&disp_drv);

    // Register input device (mouse for emulator)
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = sdl_mouse_read;
    lv_indev_drv_register(&indev_drv);

    // Create loading screen first
    create_loading_screen();
    
    // Create time screen (hidden initially)
    create_time_screen();

    printf("Loading screen displayed. Will switch to time in 4 seconds...\n");

    // Main loop
    SDL_Event event;
    bool quit = false;
    
    while (!quit) {
        // Handle SDL events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Handle LVGL tasks
        lv_timer_handler();

        // Present frame
        SDL_RenderPresent(renderer);

        // Small delay
        SDL_Delay(5);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    printf("Application closed.\n");

    return 0;
}