#include <runtime/script_camera.h>

#include <beet_core/input.h>
#include <beet_core/window.h>
#include <beet_core/time.h>

#include <beet_math/quat.h>
#include <beet_math/vec3.h>

#include <beet_shared/beet_types.h>
#include "beet_gfx/db_asset.h"

void script_update_camera() {
    const CameraEntity *camEntity = db_get_camera_entity(0);
    Transform *transform = db_get_transform(camEntity->transformIndex);

    if (input_mouse_pressed(MouseButton::Right)) {
        window_set_cursor(CursorState::HiddenLockedLockMousePos);
        window_set_cursor_lock_position(input_mouse_position());
    }

    if (input_mouse_released(MouseButton::Right)) {
        window_set_cursor(CursorState::Normal);
    }

    constexpr const float HALF_PI = 1.5707964;
    constexpr const float INPUT_EPSILON = 0.125f;
    constexpr const float ROTATION_CLAMP = HALF_PI - INPUT_EPSILON;

    if (input_mouse_down(MouseButton::Right)) {
        const vec2f delta = input_mouse_delta();
        const float mouseSpeed = 12.0f;
        transform->rotation.y += (-delta.x * (float) time_delta()) * mouseSpeed;
        transform->rotation.x += (-delta.y * (float) time_delta()) * mouseSpeed;
        transform->rotation.x = clamp(transform->rotation.x, -ROTATION_CLAMP, ROTATION_CLAMP);

        vec3f moveDirection{};
        const vec3f camForward = quat(transform->rotation) * WORLD_FORWARD;
        const vec3f camRight = quat(transform->rotation) * WORLD_RIGHT;

        float moveSpeed = 5.0f;
        const float speedUpScalar = 4.0f;
        const float speedDownScalar = 0.1f;
        if (input_key_down(KeyCode::W)) {
            moveDirection += camForward;
        }
        if (input_key_down(KeyCode::S)) {
            moveDirection += -camForward;
        }
        if (input_key_down(KeyCode::A)) {
            moveDirection += camRight;
        }
        if (input_key_down(KeyCode::D)) {
            moveDirection += -camRight;
        }
        if (input_key_down(KeyCode::R)) {
            moveDirection += WORLD_UP;
        }
        if (input_key_down(KeyCode::F)) {
            moveDirection += -WORLD_UP;
        }
        if (input_key_down(KeyCode::Shift)) {
            moveSpeed *= speedUpScalar;
        }
        if (input_key_down(KeyCode::Control)) {
            moveSpeed *= speedDownScalar;
        }
        transform->position += moveDirection * ((float) time_delta() * moveSpeed);
    }
}