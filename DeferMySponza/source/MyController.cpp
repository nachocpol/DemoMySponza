#include "MyController.hpp"
#include "MyView.hpp"

#include <scene/scene.hpp>
#include <tygra/Window.hpp>

#include <iostream>

MyController::MyController() : camera_turn_mode_(false)
{
    camera_move_speed_[0] = 0;
    camera_move_speed_[1] = 0;
    camera_move_speed_[2] = 0;
    camera_move_speed_[3] = 0;
    camera_rotate_speed_[0] = 0;
    camera_rotate_speed_[1] = 0;
    scene_ = new scene::Context();
    view_ = new MyView();
    view_->setScene(scene_);
}

MyController::~MyController()
{
    delete view_;
    delete scene_;
}

void MyController::windowControlWillStart(tygra::Window * window)
{
    window->setView(view_);
    window->setTitle("Real-Time Graphics :: DeferMySponza");
    std::cout << "Real-Time Graphics :: DeferMySponza" << std::endl;
    std::cout << "  Press F2 to toggle an animated camera" << std::endl;
}

void MyController::windowControlDidStop(tygra::Window * window)
{
    window->setView(nullptr);
}

void MyController::windowControlViewWillRender(tygra::Window * window)
{
    scene_->update();
    if (camera_turn_mode_) {
        scene_->getCamera().setRotationalVelocity(scene::Vector2(0, 0));
    }
}

void MyController::windowControlMouseMoved(tygra::Window * window,
                                           int x,
                                           int y)
{
    static int prev_x = x;
    static int prev_y = y;
    if (camera_turn_mode_) {
        int dx = x - prev_x;
        int dy = y - prev_y;
        const float mouse_speed = 0.6f;
        scene_->getCamera().setRotationalVelocity(
            scene::Vector2(-dx * mouse_speed, -dy * mouse_speed));
    }
    prev_x = x;
    prev_y = y;
}

void MyController::windowControlMouseButtonChanged(tygra::Window * window,
                                                   int button_index,
                                                   bool down)
{
    if (button_index == tygra::kWindowMouseButtonLeft) {
        camera_turn_mode_ = down;
    }
}


void MyController::windowControlMouseWheelMoved(tygra::Window * window,
                                                int position)
{
}

void MyController::windowControlKeyboardChanged(tygra::Window * window,
                                                int key_index,
                                                bool down)
{
    switch (key_index) {
    case tygra::kWindowKeyLeft:
    case 'A':
        camera_move_speed_[0] = down ? 1.f : 0.f;
        break;
    case tygra::kWindowKeyRight:
    case 'D':
        camera_move_speed_[1] = down ? 1.f : 0.f;
        break;
    case tygra::kWindowKeyUp:
    case 'W':
        camera_move_speed_[2] = down ? 1.f : 0.f;
        break;
    case tygra::kWindowKeyDown:
    case 'S':
        camera_move_speed_[3] = down ? 1.f : 0.f;
        break;
    }

    updateCameraTranslation();

    if (!down)
        return;

    switch (key_index)
    {
    case tygra::kWindowKeyF2:
        scene_->toggleCameraAnimation();
        break;
    }
}

void MyController::windowControlGamepadAxisMoved(tygra::Window * window,
                                                 int gamepad_index,
                                                 int axis_index,
                                                 float pos)
{
    const float deadzone = 0.2f;
    const float rotate_speed = 3.f;
    switch (axis_index) {
    case tygra::kWindowGamepadAxisLeftThumbX:
        if (pos < -deadzone) {
            camera_move_speed_[0] = -pos;
            camera_move_speed_[1] = 0.f;
        }
        else if (pos > deadzone) {
            camera_move_speed_[0] = 0.f;
            camera_move_speed_[1] = pos;
        }
        else {
            camera_move_speed_[0] = 0.f;
            camera_move_speed_[1] = 0.f;
        }
        break;
    case tygra::kWindowGamepadAxisLeftThumbY:
        if (pos < -deadzone) {
            camera_move_speed_[3] = -pos;
            camera_move_speed_[2] = 0.f;
        }
        else if (pos > deadzone) {
            camera_move_speed_[3] = 0.f;
            camera_move_speed_[2] = pos;
        }
        else {
            camera_move_speed_[3] = 0.f;
            camera_move_speed_[2] = 0.f;
        }
        break;
    case tygra::kWindowGamepadAxisRightThumbX:
        if (pos < -deadzone || pos > deadzone) {
            camera_rotate_speed_[0] = -pos;
        }
        else {
            camera_rotate_speed_[0] = 0.f;
        }
        scene_->getCamera().setRotationalVelocity(
            scene::Vector2(camera_rotate_speed_[0] * rotate_speed,
                           camera_rotate_speed_[1] * rotate_speed));
        break;
    case tygra::kWindowGamepadAxisRightThumbY:
        if (pos < -deadzone || pos > deadzone) {
            camera_rotate_speed_[1] = pos;
        }
        else {
            camera_rotate_speed_[1] = 0.f;
        }
        scene_->getCamera().setRotationalVelocity(
            scene::Vector2(camera_rotate_speed_[0] * rotate_speed,
                           camera_rotate_speed_[1] * rotate_speed));
        break;
    }

    updateCameraTranslation();
}

void MyController::windowControlGamepadButtonChanged(tygra::Window * window,
                                                     int gamepad_index,
                                                     int button_index,
                                                     bool down)
{
}

void MyController::updateCameraTranslation()
{
    const float key_speed = 100.f;
    const float sideward_speed = -key_speed * camera_move_speed_[0]
        + key_speed * camera_move_speed_[1];
    const float forward_speed = key_speed * camera_move_speed_[2]
        - key_speed * camera_move_speed_[3];
    scene_->getCamera().setLinearVelocity(
        scene::Vector3(sideward_speed, 0, forward_speed));
}
