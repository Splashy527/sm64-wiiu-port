#ifdef TARGET_WII_U

#include <coreinit/exit.h>
#include <coreinit/foreground.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <gx2/display.h>
#include <gx2/event.h>
#include <proc_ui/procui.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <stdlib.h>

#include "gfx_window_manager_api.h"
#include "gfx_screen_config.h"
#include "gfx_whb.h"

#define GFX_API_NAME "Wii U"

static unsigned int window_width;
static unsigned int window_height;
bool gfx_wiiu_running;
static bool fullscreen_state = false;
static void (*on_fullscreen_changed_callback)(bool is_now_fullscreen);
static bool (*on_key_down_callback)(int scancode);
static bool (*on_key_up_callback)(int scancode);
static void (*on_all_keys_up_callback)(void);

void save_callback(void) {
    OSSavesDone_ReadyToRelease();
}

uint32_t exit_callback(void* data) {
    (void) data;
    WHBLogPrint("Exit callback");

    whb_free_vbo();
    whb_free();

    WHBLogCafeDeinit();
    WHBLogUdpDeinit();

    WHBGfxShutdown();
    ProcUIShutdown();

    gfx_wiiu_running = false;
}

static void gfx_wiiu_get_dimensions(uint32_t *width, uint32_t *height) {
    switch (GX2GetSystemTVScanMode()) {
        case GX2_TV_SCAN_MODE_480I:
        case GX2_TV_SCAN_MODE_480P:
            *width = 854;
            *height = 480;
            break;
        case GX2_TV_SCAN_MODE_1080I:
        case GX2_TV_SCAN_MODE_1080P:
            *width = 1920;
            *height = 1080;
            break;
        case GX2_TV_SCAN_MODE_720P:
        default:
            *width = 1280;
            *height = 720;
            break;
    }
}

static void set_fullscreen(bool on, bool call_callback) {
    // We don't support not running in full screen
    if (on_fullscreen_changed_callback != NULL && call_callback) {
        on_fullscreen_changed_callback(on);
    }
}

static void gfx_wiiu_init(const char *game_name, bool start_in_fullscreen) {
    (void) game_name;
    (void) start_in_fullscreen;

    ProcUIInit(save_callback);
    ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, exit_callback, NULL, 0);

    gfx_wiiu_running = true;
    WHBGfxInit();
    WHBLogCafeInit();
    WHBLogUdpInit();
}

static void gfx_wiiu_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
    on_fullscreen_changed_callback = on_fullscreen_changed;
}

static void gfx_wiiu_set_fullscreen(bool enable) {
    set_fullscreen(enable, true);
}

static void gfx_wiiu_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {
    on_key_down_callback = on_key_down;
    on_key_up_callback = on_key_up;
    on_all_keys_up_callback = on_all_keys_up;
}

static void gfx_wiiu_main_loop(void (*run_one_game_iter)(void)) {
    // Ensure we run at 30FPS
    // Fool-proof unless the Wii U is able to
    // execute `run_one_game_iter()` so fast
    // that it doesn't even stall for enough time for
    // the second `GX2WaitForVsync()` to register
    if (!gfx_wiiu_running) {
        return;
    }

    GX2WaitForVsync();
    run_one_game_iter();
    GX2WaitForVsync();
}

static void gfx_wiiu_onkeydown(int scancode) {
    (void) scancode;
}

static void gfx_wiiu_onkeyup(int scancode) {
    (void) scancode;
}

static void gfx_wiiu_handle_events(void) {
    ProcUIStatus status = ProcUIProcessMessages(true);

    switch (status) {
        case PROCUI_STATUS_EXITING:
            WHBLogPrint("Going to exit");
            gfx_wiiu_running = false;
            break;
        case PROCUI_STATUS_RELEASE_FOREGROUND:
            ProcUIDrawDoneRelease();
            break;
    }
}

static bool gfx_wiiu_start_frame(void) {
    WHBGfxBeginRender();
    return true;
}

static void gfx_wiiu_swap_buffers_begin(void) {
    WHBGfxFinishRender();
    whb_free_vbo();
}

static void gfx_wiiu_swap_buffers_end(void) {
}

static double gfx_wiiu_get_time(void) {
    return 0.0;
}

struct GfxWindowManagerAPI gfx_wiiu = {
    gfx_wiiu_init,
    gfx_wiiu_set_keyboard_callbacks,
    gfx_wiiu_set_fullscreen_changed_callback,
    gfx_wiiu_set_fullscreen,
    gfx_wiiu_main_loop,
    gfx_wiiu_get_dimensions,
    gfx_wiiu_handle_events,
    gfx_wiiu_start_frame,
    gfx_wiiu_swap_buffers_begin,
    gfx_wiiu_swap_buffers_end,
    gfx_wiiu_get_time
};

#endif
