#include "base.h"

#include "io/io.h"
#include "gfx/gfx.h"
#include "memory/memory.h"
#include "platform/platform.h"
#include "util/util.h"

int main(void) {
    gfx_init(GFX_BACKEND_GL);
    io_init();

    monitors_detect();

    io_ctx.window = window_new(io_ctx.active_monitor, 1600, 900, "hello there");

    while(!window_closing(game_window)) {
        input_start_frame(&io_ctx);

        // do ALL the things
        LOG("%.2f, %.2f\n", v2f_expand(input_mouse_move()));
        LOG("%d\n", input_mouse_down(GLFW_MOUSE_BUTTON_LEFT) ? 1 : 0);

        glfwSwapBuffers(game_window->glfw_window);
        input_end_frame(&io_ctx);
    }

    io_terminate();
    return 0;
}
