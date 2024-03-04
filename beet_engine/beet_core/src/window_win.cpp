#include <beet_shared/platform_defines.h>
#if PLATFORM_WINDOWS
#include <cstdint>

#include <beet_shared/assert.h>
#include <beet_core/window.h>
#include <beet_core/input.h>
#include <beet_math/vec2.h>

#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WindowsX.h>

#include "input_types.h"

//===internal structs========
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
} g_windowInfo = {};

//===internal functions======
LRESULT CALLBACK window_procedure_callback(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            g_windowInfo.shouldWindowClose = true;
            break;
        };
        case WM_PAINT: {
            ValidateRect(hwnd, nullptr);
            break;
        };
        case WM_SIZE: {
            g_windowInfo.width = LOWORD(lParam);
            g_windowInfo.height = HIWORD(lParam);
            break;
        }
        case WM_MOVE: {
            g_windowInfo.posX = (int32_t) (short) LOWORD(lParam);
            g_windowInfo.posY = (int32_t) (short) HIWORD(lParam);
            break;
        }
        case WM_MOUSELEAVE: {
            g_windowInfo.cursorOverWindow = false;
            break;
        }
        case WM_MOUSEHOVER: {
            g_windowInfo.cursorOverWindow = true;
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
            g_windowInfo.virtualCursorPosition.x += raw->data.mouse.lLastX;
            g_windowInfo.virtualCursorPosition.y += raw->data.mouse.lLastY;

            input_mouse_move_callback(g_windowInfo.virtualCursorPosition.x,
                                      g_windowInfo.virtualCursorPosition.y
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

void update_cursor_state() {
    switch (g_windowInfo.currentCursorState) {
        case CursorState::HiddenLockedLockMousePos: {
            vec2i &pos = g_windowInfo.lockedCursorPosition;
            SetCursorPos(pos.x, pos.y);
            break;
        }
        default:
            break;
    }
}

void check_cursor_over_window(MSG *msg) {
    POINT point;
    GetCursorPos(&point);
    const RECT rect{
            g_windowInfo.posX,
            g_windowInfo.posY,
            g_windowInfo.posX + g_windowInfo.width,
            g_windowInfo.posY + g_windowInfo.height,
    };
    bool cursorOverWindow = !(point.x <= rect.left || point.x >= rect.right || point.y >= rect.bottom ||
                              point.y <= rect.top);
    if (cursorOverWindow) {
        if (!g_windowInfo.cursorOverWindow) {
            g_windowInfo.cursorOverWindow = true;
            msg->message = WM_MOUSEHOVER;
            msg->hwnd = g_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    } else {
        if (g_windowInfo.cursorOverWindow) {
            g_windowInfo.cursorOverWindow = false;
            msg->message = WM_MOUSELEAVE;
            msg->hwnd = g_windowInfo.handle;
            msg->lParam = 0xFFFFFFFF;
        }
    }
}

void window_poll() {
    MSG msg = {};
    if (PeekMessage(&msg, g_windowInfo.handle, 0, 0, PM_REMOVE)) {
        check_cursor_over_window(&msg);
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

//===api=====================
void window_update() {
    window_poll();
    update_cursor_state();
}

bool window_is_open() {
    return !g_windowInfo.shouldWindowClose;
}

bool window_is_cursor_over_window() {
    return g_windowInfo.cursorOverWindow;
}

void window_set_cursor_lock_position(const vec2i lockPos) {
    g_windowInfo.lockedCursorPosition = vec2i{
            g_windowInfo.posX + lockPos.x,
            g_windowInfo.posY + lockPos.y
    };
}

void window_set_cursor(CursorState state) {
    const RECT rect{
            g_windowInfo.posX,
            g_windowInfo.posY,
            g_windowInfo.posX + g_windowInfo.width,
            g_windowInfo.posY + g_windowInfo.height,
    };
    g_windowInfo.currentCursorState = state;
    switch (g_windowInfo.currentCursorState) {
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
    return g_windowInfo.handle;
}

//===init & shutdown=========
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
//        g_windowInfo = {};
        g_windowInfo.width = windowSize.x;
        g_windowInfo.height = windowSize.y;
        g_windowInfo.posX = screenX;
        g_windowInfo.posY = screenY;
        g_windowInfo.applicationName = L"" "BEET_WINDOW_APPLICATION_NAME";
        g_windowInfo.titleName = wc_windowTitle;
        g_windowInfo.windowClass.lpfnWndProc = window_procedure_callback;
        g_windowInfo.windowClass.hInstance = hInstance;
        g_windowInfo.windowClass.lpszClassName = g_windowInfo.applicationName;
        g_windowInfo.windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    }

    RegisterClass(&g_windowInfo.windowClass);

    g_windowInfo.handle = CreateWindowEx(
            0,
            g_windowInfo.applicationName,
            g_windowInfo.titleName,
            WS_OVERLAPPEDWINDOW,
            g_windowInfo.posX,
            g_windowInfo.posY,
            g_windowInfo.width,
            g_windowInfo.height,
            nullptr,
            nullptr,
            hInstance,
            nullptr
    );

    ASSERT_MSG(g_windowInfo.handle, "Err: Failed to create window.")
    ShowWindow(g_windowInfo.handle, SW_SHOWDEFAULT);
    g_windowInfo.shouldWindowClose = false;
    {
        RAWINPUTDEVICE rawInputDevice[1];
        rawInputDevice[0].usUsagePage = 0x01;          // HID_USAGE_PAGE_GENERIC
        rawInputDevice[0].usUsage = 0x02;              // HID_USAGE_GENERIC_MOUSE
        rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;
        rawInputDevice[0].hwndTarget = g_windowInfo.handle;
        const auto result = RegisterRawInputDevices(rawInputDevice, 1, sizeof(rawInputDevice[0]));
        ASSERT_MSG(result == TRUE, "Failed to register raw input devices");
    }
}

void window_cleanup() {
    g_windowInfo = {};
}
#endif