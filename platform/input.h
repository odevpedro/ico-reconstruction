#pragma once

#include <cstdint>
#include <array>

namespace platform {

enum class Key : uint32_t {
    None = 0,
    
    // Letters
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    
    // Numbers
    Num0, Num1, Num2, Num3, Num4, Num5,
    Num6, Num7, Num8, Num9,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    // Arrow keys
    Up, Down, Left, Right,
    
    // Modifiers
    LeftShift, RightShift,
    LeftCtrl, RightCtrl,
    LeftAlt, RightAlt,
    
    // Special keys
    Space, Enter, Escape, Backspace, Tab,
    CapsLock, NumLock, ScrollLock,
    
    // Punctuation
    Minus, Equals, LeftBracket, RightBracket,
    Backslash, Semicolon, Quote, Comma, Period, Slash,
    Backtick,
    
    // Navigation
    Home, End, PageUp, PageDown,
    Insert, Delete,
    
    // Media
    PrintScreen, Pause, Menu,
    
    // Numpad
    Numpad0, Numpad1, Numpad2, Numpad3, Numpad4,
    Numpad5, Numpad6, Numpad7, Numpad8, Numpad9,
    NumpadEnter, NumpadPlus, NumpadMinus, NumpadMultiply,
    NumpadDivide, NumpadPeriod,
    
    Count
};

enum class GamepadButton : uint32_t {
    None = 0,
    
    // Face buttons
    A, B, X, Y,
    
    // Shoulder buttons
    LeftBumper, RightBumper,
    LeftTrigger, RightTrigger,
    
    // Thumbstick buttons
    LeftThumb, RightThumb,
    
    // D-pad
    DpadUp, DpadDown, DpadLeft, DpadRight,
    
    // Menu buttons
    Start, Back, Guide,
    
    Count
};

enum class GamepadAxis : uint32_t {
    None = 0,
    
    LeftX, LeftY,
    RightX, RightY,
    
    LeftTrigger,
    RightTrigger,
    
    Count
};

class Input {
public:
    virtual ~Input() = default;
    
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    
    virtual void update() = 0;
    
    // Keyboard
    virtual bool isKeyDown(Key key) = 0;
    virtual bool isKeyUp(Key key) = 0;
    virtual bool isKeyPressed(Key key) = 0;
    virtual bool isKeyReleased(Key key) = 0;
    
    // Gamepad
    virtual bool isGamepadConnected(int index = 0) = 0;
    virtual bool isGamepadButtonDown(GamepadButton button, int index = 0) = 0;
    virtual bool isGamepadButtonPressed(GamepadButton button, int index = 0) = 0;
    virtual bool isGamepadButtonReleased(GamepadButton button, int index = 0) = 0;
    virtual float getGamepadAxisValue(GamepadAxis axis, int index = 0) = 0;
    virtual const char* getGamepadName(int index = 0) = 0;
    
    // Mouse
    virtual bool isMouseButtonDown(int button) = 0;
    virtual bool isMouseButtonPressed(int button) = 0;
    virtual bool isMouseButtonReleased(int button) = 0;
    virtual void getMousePosition(int& x, int& y) = 0;
    virtual void getMouseDelta(int& dx, int& dy) = 0;
    virtual float getMouseWheel() = 0;
};

std::unique_ptr<Input> createInput();

} // namespace platform
