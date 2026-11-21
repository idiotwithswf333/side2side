// Side-by-side TV output: TV shows [TV image | GamePad image], GamePad unchanged.
#include <wups.h>
#include <gx2/display.h>
#include <gx2/surface.h>
#include <gx2/state.h>
#include <gx2/enum.h>
#include <cstdint>

WUPS_PLUGIN_NAME("TV Side By Side");
WUPS_PLUGIN_DESCRIPTION("Shows the TV and GamePad screens side by side on the TV.");
WUPS_PLUGIN_VERSION("v0.1");
WUPS_PLUGIN_AUTHOR("you");
WUPS_PLUGIN_LICENSE("GPL");

static uint32_t sTvW = 1280, sTvH = 720;

static void setHalfViewport(bool right) {
    // Each half is sTvW/2 wide; keep 16:9 and letterbox vertically.
    float halfW = sTvW / 2.0f;
    float h     = halfW * 9.0f / 16.0f;
    float y     = (sTvH - h) / 2.0f;
    float x     = right ? halfW : 0.0f;
    GX2SetViewport(x, y, halfW, h, 0.0f, 1.0f);
    GX2SetScissor((uint32_t) x, (uint32_t) y, (uint32_t) halfW, (uint32_t) h);
}

static void setFullViewport(uint32_t w, uint32_t h) {
    GX2SetViewport(0, 0, (float) w, (float) h, 0.0f, 1.0f);
    GX2SetScissor(0, 0, w, h);
}

DECL_FUNCTION(void, GX2SetTVBuffer, void *buffer, uint32_t size, GX2TVRenderMode mode,
              GX2SurfaceFormat format, GX2BufferingMode bufferingMode) {
    switch (mode) {
        case GX2_TV_RENDER_MODE_STANDARD_480P: sTvW = 640;  sTvH = 480;  break;
        case GX2_TV_RENDER_MODE_WIDE_480P:     sTvW = 854;  sTvH = 480;  break;
        case GX2_TV_RENDER_MODE_WIDE_1080P:    sTvW = 1920; sTvH = 1080; break;
        default:                               sTvW = 1280; sTvH = 720;  break;
    }
    real_GX2SetTVBuffer(buffer, size, mode, format, bufferingMode);
}

DECL_FUNCTION(void, GX2CopyColorBufferToScanBuffer, const GX2ColorBuffer *cb, GX2ScanTarget target) {
    if (target == GX2_SCAN_TARGET_TV) {
        setHalfViewport(false);                       // TV image -> left half
        real_GX2CopyColorBufferToScanBuffer(cb, target);
    } else if (target == GX2_SCAN_TARGET_DRC) {
        setFullViewport(854, 480);                    // GamePad: normal
        real_GX2CopyColorBufferToScanBuffer(cb, target);
        setHalfViewport(true);                        // same image -> TV right half
        real_GX2CopyColorBufferToScanBuffer(cb, GX2_SCAN_TARGET_TV);
    } else {
        real_GX2CopyColorBufferToScanBuffer(cb, target);
    }
}

WUPS_MUST_REPLACE(GX2SetTVBuffer,                 WUPS_LOADER_LIBRARY_GX2, GX2SetTVBuffer);
WUPS_MUST_REPLACE(GX2CopyColorBufferToScanBuffer, WUPS_LOADER_LIBRARY_GX2, GX2CopyColorBufferToScanBuffer);
