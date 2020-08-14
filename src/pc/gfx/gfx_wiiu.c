#ifdef TARGET_WII_U

#include <coreinit/foreground.h>
#include <whb/gfx.h>
#include <whb/log.h>
#include <gx2/display.h>
#include <gx2/event.h>
#include <gx2/swap.h>
#include <proc_ui/procui.h>
#include <whb/log_cafe.h>
#include <whb/log_udp.h>
#include <macros.h>
#include <sndcore2/core.h>

#include "gfx_window_manager_api.h"
#include "gfx_whb.h"

#define GFX_API_NAME "Wii U"

static unsigned int window_width;
static unsigned int window_height;
static bool is_running;

void save_callback(void) {
    OSSavesDone_ReadyToRelease();
}

uint32_t exit_callback(UNUSED void* data) {
    WHBLogPrint("Exit callback");
    is_running = false;

    whb_free_vbo();
    whb_free();

    AXQuit();
    WHBGfxShutdown();

    WHBLogCafeDeinit();
    WHBLogUdpDeinit();
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
}

static void gfx_wiiu_init(UNUSED const char *game_name, UNUSED bool start_in_fullscreen) {
    ProcUIInit(save_callback);
    ProcUIRegisterCallback(PROCUI_CALLBACK_EXIT, exit_callback, NULL, 0);

    is_running = true;

    WHBGfxInit();
    GX2SetSwapInterval(2);
    WHBLogCafeInit();
    WHBLogUdpInit();
}

static void gfx_wiiu_set_fullscreen_changed_callback(void (*on_fullscreen_changed)(bool is_now_fullscreen)) {
}

static void gfx_wiiu_set_fullscreen(bool enable) {
    set_fullscreen(enable, true);
}

static void gfx_wiiu_set_keyboard_callbacks(bool (*on_key_down)(int scancode), bool (*on_key_up)(int scancode), void (*on_all_keys_up)(void)) {
}

static void gfx_wiiu_main_loop(void (*run_one_game_iter)(void)) {
    run_one_game_iter();
}

static void gfx_wiiu_onkeydown(UNUSED int scancode) {
}

static void gfx_wiiu_onkeyup(UNUSED int scancode) {
}

static void gfx_wiiu_handle_events(void) {
}

bool gfx_wiiu_is_running(void) {
    if (!is_running) {
        return false;
    }

    ProcUIStatus status = ProcUIProcessMessages(true);

    switch (status) {
        case PROCUI_STATUS_EXITING:
            is_running = false;
            return false;
        case PROCUI_STATUS_RELEASE_FOREGROUND:
            ProcUIDrawDoneRelease();
            break;
        case PROCUI_STATUS_IN_BACKGROUND:
        case PROCUI_STATUS_IN_FOREGROUND:
            break;
    }
    return true;
}

static bool gfx_wiiu_start_frame(void) {
    GX2WaitForFlip();
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
