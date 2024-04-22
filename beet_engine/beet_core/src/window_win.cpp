#include <beet_shared/platform_defines.h>

#if PLATFORM_WINDOWS

#include <cstdint>

#include <beet_shared/assert.h>
#include <beet_core/window.h>
#include <beet_core/input.h>
#include <beet_core/input_types.h>
#include <beet_math/vec2.h>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WindowsX.h>

//===INTERNAL_STRUCTS===================================================================================================
static struct WindowInfo {
    WNDCLASS windowClass;
    HWND handle;
    const wchar_t *applicationName;
    const wchar_t *titleName;

    int32_t width;
    int32_t height;
    int32_t posX;
    int32_t posY;
    bool shouldWindowClose;

    CursorState currentCursorState;
    vec2i lockedCursorPosition;
    vec2i virtualCursorPosition;
    bool cursorOverWindow;
} s_windowInfo = {};

LRESULT (*g_windowProcCallback_Func)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = nullptr;
//======================================================================================================================

//===INTERNAL_FUNCTIONS=================================================================================================
static LRESULT CALLBACK window_procedure_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (g_windowProcCallback_Func != nullptr) {
        g_windowProcCallback_Func(hwnd, uMsg, wParam, lParam);
    }
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            s_windowInfo.shouldWindowClose = true;
            break;
        };
        case WM_PAINT: {
            ValidateRect(hwnd, nullptr);
            break;
        };
        case WM_SIZE: {
            s_windowInfo.width = LOWORD(lParam);
            s_windowInfo.height = HIWORD(lParam);
            break;
        }
        case WM_MOVE: {
            s_windowInfo.posX = (int32_t) (short) LOWORD(lParam);
            s_windowInfo.posY = (int32_t) (short) HIWORD(lParam);
            break;
        }
        case WM_MOUSELEAVE: {
            s_windowInfo.cursorOverWindow = false;
            break;
        }
        case WM_MOUSEHOVER: {
            s_windowInfo.cursorOverWindow = true;
            break;
        }
            //input
        case WM_KEYDOWN: {
            input_key_down_callback((char) wParam);
            break;
        };
        case WM_KEYUP: {
            input_key_up_callback((char) wParam);
            break;
        };
        case WM_INPUT: {
            unsigned size = sizeof(RAWINPUT);
            static RAWINPUT raw[sizeof(RAWINPUT)];

            GetRawInputData((HRAWINPUT) lParam, RID_INPUT, raw, &size, sizeof(RAWINPUTHEADER));
            s_windowInfo.virtualCursorPosition.x += raw->data.mouse.lLastX;
            s_windowInfo.virtualCursorPosition.y += raw->data.mouse.lLastY;

            input_mouse_move_callback(s_windowInfo.virtualCursorPosition.x,
                                      s_windowInfo.virtualCursorPosition.y
            );

            //TODO:CORE consider moving other input callbacks to WM_INPUT i.e. WHEEL_DELTA and JOY PADS.
            //          wheel `raw->data.mouse.usButtonData`
            break;
        }
        case WM_MOUSEMOVE: {
            const int32_t x = GET_X_LPARAM(lParam);
            const int32_t y = GET_Y_LPARAM(lParam);
            input_mouse_windowed_position_callback(x, y);
            break;
        }
        case WM_RBUTTONDOWN: {
            input_mouse_down_callback((int32_t) MouseButton::Right);
            break;
        }
        case WM_RBUTTONUP: {
            input_mouse_up_callback((int32_t) MouseButton::Right);
            break;
        }
        case WM_LBUTTONDOWN: {
            input_mouse_down_callback((int32_t) MouseButton::Left);
            break;
        }
        case WM_LBUTTONUP: {
            input_mouse_up_callback((int32_t) MouseButton::Left);
            break;
        }
        case WM_MBUTTONDOWN: {
            input_mouse_down_callback((int32_t) MouseButton::Middle);
            break;
        }
        case WM_MBUTTONUP: {
            input_mouse_up_callback((int32_t) MouseButton::Middle);
            break;
        }
        case WM_MOUSEWHEEL: {
            const int32_t scrollDelta = GET_WHEEL_DELTA_WPARAM(wParam);
            input_mouse_scroll_callback(scrollDelta);
            break;
        }
        case WM_XBUTTONDOWN: {
            switch (GET_XBUTTON_WPARAM(wParam)) {
                case XBUTTON1: {
                    input_mouse_down_callback((int32_t) MouseButton::Back);
                    break;
                }
                case XBUTTON2: {
                    input_mouse_down_callback((int32_t) MouseButton::Forward);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case WM_XBUTTONUP: {
            switch (GET_XBUTTON_WPARAM(wParam)) {
                case XBUTTON1: {
                    input_mouse_up_callback((int32_t) MouseButton::Back);
                    break;
                }
                case XBUTTON2: {
                    input_mouse_up_callback((int32_t) MouseButton::Forward);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

static void update_cursor_state() {
    switch (s_windowInfo.currentCursorState) {
        case CursorState::HiddenLockedLockMousePos: {
            vec2i &pos = s_windowInfo.lockedCursorPosition;
            SetCursorPos(pos.x, pos.y);
            break;
        }
        default:
            break;
    }
}

static void check_cursor_over_window(MSG *msg) {
    POINT point;
    GetCursorPos(&point);
    const RECT rect{
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.posX + s_windowInfo.width,
            s_windowInfo.posY + s_windowInfo.height,
    };
    bool cursorOverWindow = !(point.x <= rect.left || point.x >= rect.right || point.y >= rect.bottom ||
                              point.y <= rect.top);
    if (cursorOverWindow) {
        if (!s_windowInfo.cursorOverWindow) {
            s_windowInfo.cursorOverWindow = true;
            msg->message = WM_MOUSEHOVER;
            msg->hwnd = s_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    } else {
        if (s_windowInfo.cursorOverWindow) {
            s_windowInfo.cursorOverWindow = false;
            msg->message = WM_MOUSELEAVE;
            msg->hwnd = s_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    }
}

static void window_poll() {
    MSG msg = {};
    if (PeekMessage(&msg, s_windowInfo.handle, 0, 0, PM_REMOVE)) {
        check_cursor_over_window(&msg);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}
//======================================================================================================================

//===API================================================================================================================
void window_update() {
    window_poll();
    update_cursor_state();
}

bool window_is_open() {
    return !s_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window() {
    return s_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos) {
    s_windowInfo.lockedCursorPosition = vec2i{
            s_windowInfo.posX + lockPos.x,
            s_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(CursorState state) {
    const RECT rect{
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.posX + s_windowInfo.width,
            s_windowInfo.posY + s_windowInfo.height,
    };
    s_windowInfo.currentCursorState = state;
    switch (s_windowInfo.currentCursorState) {
        case CursorState::Normal: {
            ShowCursor(true);
            ClipCursor(nullptr);
            break;
        }
        case CursorState::Hidden: {
            ShowCursor(false);
            ClipCursor(nullptr);
            break;
        }
        case CursorState::Locked: {
            ShowCursor(true);
            ClipCursor(&rect);
            break;
        }
        case CursorState::HiddenLockedLockMousePos:
        case CursorState::HiddenLocked: {
            ShowCursor(false);
            ClipCursor(&rect);
            break;
        }
        default:
            break;
    }
}

void *window_get_handle() {
    return s_windowInfo.handle;
}

void window_set_procedure_callback_func(void *procCallback) {
    //Cursed casting but makes the API very convenient
    g_windowProcCallback_Func = (LRESULT (*)(HWND, UINT, WPARAM, LPARAM)) (procCallback);
}
//======================================================================================================================

//===INIT_&_SHUTDOWN====================================================================================================
void window_create(const char windowTitle[MAX_WINDOW_TITLE_SIZE], const vec2i &windowSize, const vec2i &windowPosition) {
    int32_t screenX = windowPosition.x;
    int32_t screenY = windowPosition.y;
    if (windowPosition.x == UINT32_MAX || windowPosition.y == UINT32_MAX) {
        screenX = GetSystemMetrics(SM_CXSCREEN) / 2 - (windowSize.x / 2);
        screenY = GetSystemMetrics(SM_CYSCREEN) / 2 - (windowSize.y / 2);
    }

    wchar_t wc_windowTitle[MAX_WINDOW_TITLE_SIZE];
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, windowTitle, -1, wc_windowTitle, sizeof(wchar_t) * MAX_WINDOW_TITLE_SIZE);

    HINSTANCE hInstance = GetModuleHandle(nullptr);
    {
//        s_windowInfo = {};
        s_windowInfo.width = windowSize.x;
        s_windowInfo.height = windowSize.y;
        s_windowInfo.posX = screenX;
        s_windowInfo.posY = screenY;
        s_windowInfo.applicationName = L"" "BEET_WINDOW_APPLICATION_NAME";
        s_windowInfo.titleName = wc_windowTitle;
        s_windowInfo.windowClass.lpfnWndProc = window_procedure_callback;
        s_windowInfo.windowClass.hInstance = hInstance;
        s_windowInfo.windowClass.lpszClassName = s_windowInfo.applicationName;
        s_windowInfo.windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    }

    RegisterClass(&s_windowInfo.windowClass);

    s_windowInfo.handle = CreateWindowEx(
            0,
            s_windowInfo.applicationName,
            s_windowInfo.titleName,
            WS_OVERLAPPEDWINDOW,
            s_windowInfo.posX,
            s_windowInfo.posY,
            s_windowInfo.width,
            s_windowInfo.height,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    ASSERT_MSG(s_windowInfo.handle, "Err: Failed to create window.")
    ShowWindow(s_windowInfo.handle, SW_SHOWDEFAULT);
    s_windowInfo.shouldWindowClose = false;
    {
        RAWINPUTDEVICE rawInputDevice[1];
        rawInputDevice[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        rawInputDevice[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
        rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;
        rawInputDevice[0].hwndTarget = s_windowInfo.handle;
        const auto result = RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
        ASSERT_MSG(result == TRUE, "Failed to register raw input devices");
    }
}

void window_cleanup() {
    s_windowInfo = {};
}
//======================================================================================================================
#endif