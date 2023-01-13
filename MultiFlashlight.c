// Based on Flashlight by @xMasterX
// Modified by @GaelicThunder

#include <furi.h>
#include <furi_hal_power.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>
#include <gui/elements.h>
#include "assets_icons.h"
#include "dolphin/dolphin.h"

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} PluginEvent;

typedef struct {
    bool is_on;
} PluginState;

// Lazy way to save the button pressed
int KeyPressed = -1;

int Portrait = 0;

static void render_callback(Canvas* const canvas, void* ctx) {
    const PluginState* plugin_state = acquire_mutex((ValueMutex*)ctx, 25);
    if(plugin_state == NULL) {
        return;
    }

    const char* Str;

    // Green and White are inverted for some reason
    switch(KeyPressed) {
    case 1:
        if(plugin_state->is_on) {
            Str = "RED";
        } else {
            Str = "OFF";
        }
        break;
    case 2:
        if(plugin_state->is_on) {
            Str = "GREEN";
        } else {
            Str = "OFF";
        }
        break;
    case 3:
        if(plugin_state->is_on) {
            Str = "WHITE";
        } else {
            Str = "OFF";
        }
        break;
    case 4:
        if(plugin_state->is_on) {
            Str = "BLUE";
        } else {
            Str = "OFF";
        }
        break;
    case 0:
        if(plugin_state->is_on) {
            Str = "ON";
        } else {
            Str = "OFF";
        }
        break;
    default:
        Str = "SUP!";
        break;
    }

    if(Portrait == 0) {
        canvas_draw_icon(canvas, 9, 8, &I_passport_happy3_46x49);
    } else if(Portrait == 1) {
        canvas_draw_icon(canvas, 9, 8, &I_passport_bad3_46x49);
    } else if(Portrait == 2) {
        canvas_draw_icon(canvas, 9, 8, &I_passport_okay3_46x49);
    }

    canvas_draw_icon(canvas, 0, 8, &I_passport_left_6x46);

    canvas_draw_icon(canvas, 121, 9, &I_passport_left_6x46);

    canvas_draw_icon(canvas, 0, 0, &I_Unplug_bg_top_128x14);

    canvas_draw_icon(canvas, 0, 53, &I_Unplug_bg_bottom_128x10);

    canvas_draw_icon(canvas, 56, 34, &I_Space_65x18);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 76, 46, Str);

    release_mutex((ValueMutex*)ctx, plugin_state);
}

static void input_callback(InputEvent* input_event, FuriMessageQueue* event_queue) {
    furi_assert(event_queue);

    PluginEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(event_queue, &event, FuriWaitForever);
}

//0 ON-OFF 1 Up  2 Down  3 Right 4 Left
//0 ON-OFF 1 Red 2 White 3 Green 4 Blue
//0 B3     1 A4  2 B2    3 ALL   4 C3
//  B3 is the VCC
static void direction(int button, PluginState* const plugin_state) {
    furi_hal_gpio_write(&gpio_ext_pb3, false);
    furi_hal_gpio_write(&gpio_ext_pa4, false);
    furi_hal_gpio_write(&gpio_ext_pb2, false);
    furi_hal_gpio_write(&gpio_ext_pc3, false);

    furi_hal_gpio_init(&gpio_ext_pb3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    KeyPressed = button;
    if(button == 1) {
        furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pa4, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_write(&gpio_ext_pb2, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
    } else if(button == 2) {
        furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pa4, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_write(&gpio_ext_pa4, true);
        furi_hal_gpio_write(&gpio_ext_pc3, true);
    } else if(button == 3) {
        furi_hal_gpio_init(&gpio_ext_pa4, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
    } else if(button == 4) {
        furi_hal_gpio_init(&gpio_ext_pc3, GpioModeOutputOpenDrain, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pa4, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_init(&gpio_ext_pb2, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
        furi_hal_gpio_write(&gpio_ext_pa4, true);
        furi_hal_gpio_write(&gpio_ext_pb2, true);
    }

    if(button == 0) {
        if(plugin_state->is_on) {
            furi_hal_gpio_write(&gpio_ext_pb3, false);
            plugin_state->is_on = false;
        } else {
            furi_hal_gpio_write(&gpio_ext_pb3, true);
            plugin_state->is_on = true;
        }
    } else {
        if(plugin_state->is_on) {
            furi_hal_gpio_write(&gpio_ext_pb3, true);
        } else {
            furi_hal_gpio_write(&gpio_ext_pb3, false);
        }
    }
}

int32_t flashlight_app() {
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(PluginEvent));

    PluginState* plugin_state = malloc(sizeof(PluginState));

    ValueMutex state_mutex;
    if(!init_mutex(&state_mutex, plugin_state, sizeof(PluginState))) {
        FURI_LOG_E("flashlight", "cannot create mutex\r\n");
        furi_message_queue_free(event_queue);
        free(plugin_state);
        return 255;
    }
    Portrait = rand() % 3;
    // Set system callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, render_callback, &state_mutex);
    view_port_input_callback_set(view_port, input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    PluginEvent event;
    for(bool processing = true; processing;) {
        FuriStatus event_status = furi_message_queue_get(event_queue, &event, 100);

        PluginState* plugin_state = (PluginState*)acquire_mutex_block(&state_mutex);

        if(event_status == FuriStatusOk) {
            // press events
            if(event.type == EventTypeKey) {
                if(event.input.type == InputTypePress) {
                    switch(event.input.key) {
                    case InputKeyUp:
                        direction(1, plugin_state);
                        break;
                    case InputKeyDown:
                        direction(2, plugin_state);
                        break;
                    case InputKeyRight:
                        direction(3, plugin_state);
                        break;
                    case InputKeyLeft:
                        direction(4, plugin_state);
                        break;
                    case InputKeyOk:
                        direction(0, plugin_state);
                        break;
                    case InputKeyBack:
                        processing = false;
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        view_port_update(view_port);
        release_mutex(&state_mutex, plugin_state);
    }

    if(plugin_state->is_on) {
        furi_hal_gpio_write(&gpio_ext_pb3, false);
    }
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);
    delete_mutex(&state_mutex);

    return 0;
}
