#include <beet_shared/feature_defines.h>
#if CHECK_FEATURE(FEATURE_PLATFORM_LINUX)
#include <cstdio>
#include <cstdlib>

#include <beet_shared/assert.h>

#include <X11/XKBlib.h>

#include <beet_core/window.h>
#include <beet_core/input_types.h>

#include <beet_core/input.h>
#include <cstddef>
#include <cstdint>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xfixes.h>
#include <X11/keysym.h>

//===INTERNAL_STRUCTS===================================================================================================
#define KAH_MAX_WINDOW_SIZE_X 16384
#define KAH_MAX_WINDOW_SIZE_Y 16384
#define KAH_MIN_WINDOW_SIZE_X 256
#define KAH_MIN_WINDOW_SIZE_Y 256

typedef struct XLibHandles // used in kah_gfx/vulkan/gfx_vulkan_surface_linux.c
{
    Display* display;
    Window window;
} XLibHandles;

static struct WindowInfo
{
    const char* applicationName;
    const char* titleName;

    int32_t width;
    int32_t height;
    int32_t posX;
    int32_t posY;
    bool shouldWindowClose;

    CursorState currentCursorState;

    vec2i lockedCursorPosition;
    vec2i virtualCursorPosition;
    vec2i lastPosition;
    bool cursorOverWindow;
    XLibHandles xLibHandles;
    int screen;

    XEvent event;
} s_windowInfo = {};

//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static void cursor_hide(Display* display, Window window)
{
    XFixesHideCursor(display, window);
}

static void cursor_show(Display* display, Window window)
{
    XFixesShowCursor(display, window);
}

static void cursor_lock(Display* display, Window window)
{
    XGrabPointer(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window,
                 True,ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, s_windowInfo.xLibHandles.window, None, CurrentTime
    );
}

beet_KeyCode x11_to_beet_keycode(KeySym keysym)
{
    switch (keysym)
    {
    case XK_space: return beet_KeyCode::Space;
    case XK_Return: return beet_KeyCode::Enter;
    case XK_Escape: return beet_KeyCode::Escape;
    case XK_BackSpace: return beet_KeyCode::Backspace;
    case XK_Tab: return beet_KeyCode::Tab;
    case XK_Shift_L: return beet_KeyCode::LeftShift;
    case XK_Shift_R: return beet_KeyCode::RightShift;
    case XK_Control_L: return beet_KeyCode::LeftControl;
    case XK_Control_R: return beet_KeyCode::RightControl;
    case XK_Alt_L: return beet_KeyCode::Alt;
    case XK_Alt_R: return beet_KeyCode::Menu;
    case XK_Meta_L: return beet_KeyCode::LeftSuper;
    case XK_Meta_R: return beet_KeyCode::RightSuper;

    case XK_0: return beet_KeyCode::N0;
    case XK_1: return beet_KeyCode::N1;
    case XK_2: return beet_KeyCode::N2;
    case XK_3: return beet_KeyCode::N3;
    case XK_4: return beet_KeyCode::N4;
    case XK_5: return beet_KeyCode::N5;
    case XK_6: return beet_KeyCode::N6;
    case XK_7: return beet_KeyCode::N7;
    case XK_8: return beet_KeyCode::N8;
    case XK_9: return beet_KeyCode::N9;

    case XK_A:
    case XK_a: return beet_KeyCode::A;
    case XK_B:
    case XK_b: return beet_KeyCode::B;
    case XK_C:
    case XK_c: return beet_KeyCode::C;
    case XK_D:
    case XK_d: return beet_KeyCode::D;
    case XK_E:
    case XK_e: return beet_KeyCode::E;
    case XK_F:
    case XK_f: return beet_KeyCode::F;
    case XK_G:
    case XK_g: return beet_KeyCode::G;
    case XK_H:
    case XK_h: return beet_KeyCode::H;
    case XK_I:
    case XK_i: return beet_KeyCode::I;
    case XK_J:
    case XK_j: return beet_KeyCode::J;
    case XK_K:
    case XK_k: return beet_KeyCode::K;
    case XK_L:
    case XK_l: return beet_KeyCode::L;
    case XK_M:
    case XK_m: return beet_KeyCode::M;
    case XK_N:
    case XK_n: return beet_KeyCode::N;
    case XK_O:
    case XK_o: return beet_KeyCode::O;
    case XK_P:
    case XK_p: return beet_KeyCode::P;
    case XK_Q:
    case XK_q: return beet_KeyCode::Q;
    case XK_R:
    case XK_r: return beet_KeyCode::R;
    case XK_S:
    case XK_s: return beet_KeyCode::S;
    case XK_T:
    case XK_t: return beet_KeyCode::T;
    case XK_U:
    case XK_u: return beet_KeyCode::U;
    case XK_V:
    case XK_v: return beet_KeyCode::V;
    case XK_W:
    case XK_w: return beet_KeyCode::W;
    case XK_X:
    case XK_x: return beet_KeyCode::X;
    case XK_Y:
    case XK_y: return beet_KeyCode::Y;
    case XK_Z:
    case XK_z: return beet_KeyCode::Z;

    case XK_Up: return beet_KeyCode::Up;
    case XK_Down: return beet_KeyCode::Down;
    case XK_Left: return beet_KeyCode::Left;
    case XK_Right: return beet_KeyCode::Right;

    case XK_F1: return beet_KeyCode::F1;
    case XK_F2: return beet_KeyCode::F2;
    case XK_F3: return beet_KeyCode::F3;
    case XK_F4: return beet_KeyCode::F4;
    case XK_F5: return beet_KeyCode::F5;
    case XK_F6: return beet_KeyCode::F6;
    case XK_F7: return beet_KeyCode::F7;
    case XK_F8: return beet_KeyCode::F8;
    case XK_F9: return beet_KeyCode::F9;
    case XK_F10: return beet_KeyCode::F10;
    case XK_F11: return beet_KeyCode::F11;
    case XK_F12: return beet_KeyCode::F12;

    default: return beet_KeyCode::Unknown;
    }
}

typedef void (*EventCallback)(XEvent* event);
static EventCallback s_xEventCallback = nullptr;

void window_set_procedure_callback_func(void* procCallback)
{
    s_xEventCallback = (EventCallback)procCallback;
}

static void x_event_callback(XEvent* event)
{
    if (s_xEventCallback)
    {
        s_xEventCallback(event);
    }
}

static void window_poll()
{
    while (XPending(s_windowInfo.xLibHandles.display) > 0)
    {
        XNextEvent(s_windowInfo.xLibHandles.display, &s_windowInfo.event);
        x_event_callback(&s_windowInfo.event);
        switch (s_windowInfo.event.type)
        {
        case Expose:
            break;

        case KeyPress:
            {
                int key = XkbKeycodeToKeysym(s_windowInfo.xLibHandles.display, s_windowInfo.event.xkey.keycode, 0, 0);
                beet_KeyCode keyCode = x11_to_beet_keycode(key);
                input_key_down_callback((int32_t)keyCode);

                if (key == XK_Q)
                {
                    if (s_windowInfo.currentCursorState != CursorState::Locked)
                    {
                        window_set_cursor(CursorState::Locked);
                        window_set_cursor_lock_position(s_windowInfo.virtualCursorPosition);
                    }
                    else
                    {
                        window_set_cursor(CursorState::Normal);
                    }
                }
                break;
            }

        case KeyRelease:
            {
                int key = XkbKeycodeToKeysym(s_windowInfo.xLibHandles.display, s_windowInfo.event.xkey.keycode, 0, 0);
                beet_KeyCode keyCode = x11_to_beet_keycode(key);
                input_key_up_callback((int32_t)keyCode);
                break;
            }

        case ConfigureNotify:
            s_windowInfo.width = s_windowInfo.event.xconfigure.width;
            s_windowInfo.height = s_windowInfo.event.xconfigure.height;
            break;

        case MotionNotify:
            {
                vec2i delta;
                delta.x = s_windowInfo.event.xmotion.x - s_windowInfo.lastPosition.x;
                delta.y = s_windowInfo.event.xmotion.y - s_windowInfo.lastPosition.y;
                s_windowInfo.virtualCursorPosition.x += delta.x;
                s_windowInfo.virtualCursorPosition.y += delta.y;

                input_mouse_move_callback(s_windowInfo.virtualCursorPosition.x, s_windowInfo.virtualCursorPosition.y);
                input_mouse_windowed_position_callback(s_windowInfo.event.xmotion.x, s_windowInfo.event.xmotion.y);

                if (s_windowInfo.currentCursorState == CursorState::HiddenLockedLockMousePos)
                {
                    s_windowInfo.lastPosition.x = s_windowInfo.lockedCursorPosition.x;
                    s_windowInfo.lastPosition.y = s_windowInfo.lockedCursorPosition.y;
                    XWarpPointer(s_windowInfo.xLibHandles.display, None, s_windowInfo.xLibHandles.window, 0, 0, 0, 0, s_windowInfo.lastPosition.x, s_windowInfo.lastPosition.y);
                }
                else
                {
                    s_windowInfo.lastPosition.x = s_windowInfo.event.xmotion.x;
                    s_windowInfo.lastPosition.y = s_windowInfo.event.xmotion.y;
                }
                break;
            }

        case FocusOut:
            window_set_cursor(CursorState::Normal);
            break;

        case ButtonPress:
            {
                int button = s_windowInfo.event.xbutton.button;
                switch (button)
                {
                case Button1: // Left mouse button
                    input_mouse_down_callback((int32_t)MouseButton::Left);
                    break;
                case Button2: // Middle mouse button
                    input_mouse_down_callback((int32_t)MouseButton::Middle);
                    break;
                case Button3: // Right mouse button
                    input_mouse_down_callback((int32_t)MouseButton::Right);
                    break;
                case Button4: // Scroll up
                    input_mouse_scroll_callback(1);
                    break;
                case Button5: // Scroll down
                    input_mouse_scroll_callback(-1);
                    break;
                default:
                    break;
                }
                break;
            }

        case ButtonRelease:
            {
                int button = s_windowInfo.event.xbutton.button;
                switch (button)
                {
                case Button1:
                    input_mouse_up_callback((int32_t)MouseButton::Left);
                    break;
                case Button2:
                    input_mouse_up_callback((int32_t)MouseButton::Middle);
                    break;
                case Button3:
                    input_mouse_up_callback((int32_t)MouseButton::Right);
                    break;
                default:
                    break;
                }
                break;
            }

        case LeaveNotify:
            s_windowInfo.cursorOverWindow = false;
            break;

        case EnterNotify:

            s_windowInfo.cursorOverWindow = true;
            break;

        default:
            break;
        }
    }
}

//======================================================================================================================

//===API================================================================================================================
void window_update()
{
    window_poll();
}

bool window_is_open()
{
    return !s_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window()
{
    return s_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos)
{
    s_windowInfo.lockedCursorPosition = (vec2i){
        s_windowInfo.posX + lockPos.x,
        s_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(CursorState state)
{
    switch (state)
    {
    case CursorState::Normal:
        if (s_windowInfo.currentCursorState == CursorState::Hidden ||
            s_windowInfo.currentCursorState == CursorState::HiddenLocked ||
            s_windowInfo.currentCursorState == CursorState::HiddenLockedLockMousePos)
        {
            cursor_show(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        }

        if (s_windowInfo.currentCursorState == CursorState::HiddenLockedLockMousePos)
        {
            XWarpPointer(s_windowInfo.xLibHandles.display, None, s_windowInfo.xLibHandles.window, 0, 0, 0, 0, s_windowInfo.lockedCursorPosition.x,
                         s_windowInfo.lockedCursorPosition.y);
        }

        if (s_windowInfo.currentCursorState == CursorState::Locked ||
            s_windowInfo.currentCursorState == CursorState::HiddenLocked ||
            s_windowInfo.currentCursorState == CursorState::HiddenLockedLockMousePos)
        {
            XUngrabPointer(s_windowInfo.xLibHandles.display, CurrentTime);
        }

        break;
    case CursorState::Hidden:
        cursor_hide(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        XUngrabPointer(s_windowInfo.xLibHandles.display, CurrentTime);
        break;
    case CursorState::Locked:
        cursor_lock(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        break;
    case CursorState::HiddenLocked:
    case CursorState::HiddenLockedLockMousePos:
        cursor_hide(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
        cursor_lock(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);

        break;
    default:
        break;
    }

    s_windowInfo.currentCursorState = state;
}

void* window_get_handle()
{
    return &s_windowInfo.xLibHandles;
}

//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void window_create(const char windowTitle[MAX_WINDOW_TITLE_SIZE], const vec2i& windowSize, const vec2i& windowPosition)
{
    s_windowInfo.applicationName = "kah engine (linux)";
    s_windowInfo.titleName = windowTitle;
    s_windowInfo.width = windowSize.x;
    s_windowInfo.height = windowSize.y;
    s_windowInfo.posX = windowPosition.x; // Corrected: use x for position
    s_windowInfo.posY = windowPosition.y;

    s_windowInfo.xLibHandles.display = XOpenDisplay(nullptr);
    if (s_windowInfo.xLibHandles.display == nullptr)
    {
        // TODO: ADD ASSERT
        return;
    }
    s_windowInfo.screen = DefaultScreen(s_windowInfo.xLibHandles.display);
    s_windowInfo.xLibHandles.window = XCreateSimpleWindow(
        s_windowInfo.xLibHandles.display,
        RootWindow(s_windowInfo.xLibHandles.display, s_windowInfo.screen),
        s_windowInfo.posX, s_windowInfo.posY,
        s_windowInfo.width, s_windowInfo.height,
        1,
        BlackPixel(s_windowInfo.xLibHandles.display, s_windowInfo.screen),
        WhitePixel(s_windowInfo.xLibHandles.display, s_windowInfo.screen)
    );

    XSizeHints hints = {
        .flags = PMinSize | PMaxSize,
        .min_width = KAH_MIN_WINDOW_SIZE_X,
        .min_height = KAH_MIN_WINDOW_SIZE_Y,
        .max_width = KAH_MAX_WINDOW_SIZE_X,
        .max_height = KAH_MAX_WINDOW_SIZE_Y,
    };
    XSetNormalHints(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window, &hints);
    XStoreName(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window, s_windowInfo.titleName);

    XSelectInput(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window,
                 ExposureMask | KeyPressMask | KeyReleaseMask |
                 StructureNotifyMask | PointerMotionMask |
                 ButtonPressMask | ButtonReleaseMask |
                 EnterWindowMask | LeaveWindowMask);

    XMapWindow(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
}

void window_cleanup()
{
    XDestroyWindow(s_windowInfo.xLibHandles.display, s_windowInfo.xLibHandles.window);
    XCloseDisplay(s_windowInfo.xLibHandles.display);
    s_windowInfo = (struct WindowInfo){};
}

//======================================================================================================================
#endif
