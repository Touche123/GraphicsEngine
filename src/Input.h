#pragma once

#include <functional>
#include <array>
#include <iostream>

#ifdef _DEBUG
#include <cassert>
#endif

class Input {
	Input()
	{
		std::fill(m_keys.begin(), m_keys.end(), false);
		std::fill(m_prevKeys.begin(), m_prevKeys.end(), false);
	};
	~Input() = default;

public:

	static auto& GetInstance()
	{
		static Input instance;
		return instance;
	}

	Input(const Input&) = delete;
	Input& operator=(const Input&) = delete;

	void Update()
	{
		m_mouseMoved = false;
		m_shouldResize = false;

		std::copy(m_keys.cbegin(), m_keys.cend(), m_prevKeys.begin());
	}

	// Getters
	// Keyboard

	// Was the key only tapped?
	auto IsKeyPressed(const std::size_t key) const noexcept
	{
#ifdef _DEBUG
		assert(key < 1024);
#endif
		return m_keys[key] && !m_prevKeys[key];
	}

	// Is the key being held down?
	auto IsKeyHeld(const std::size_t key) const noexcept
	{
#ifdef _DEBUG
		assert(key < 1024);
#endif
		return m_keys[key];
	}

	// Mouse
	auto MouseMoved() const noexcept { return m_mouseMoved; }
	auto GetMouseX() const noexcept { return m_xPos; }
	auto GetMouseY() const noexcept { return m_yPos; }
	auto GetMouseScrollXOffset() const noexcept { return m_xOffset; }
	auto GetMouseScrollYOffset() const noexcept { return m_yOffset; }

	// Window
	auto ShouldResize() const noexcept { return m_shouldResize; }
	auto GetWidth() const noexcept { return m_width; }
	auto GetHeight() const noexcept { return m_height; }

	// Generic Input Callbacks
	// Mouse moved
	std::function<void(double, double)> mouseMoved = [&](auto xPos, auto yPos) {
		this->m_mouseMoved = true;
		this->m_xPos = xPos;
		this->m_yPos = yPos;
	};

	// Mousebuttons
	std::function<void(int, int, int)> mousePressed = [&](auto button, auto action, auto mods) {
		m_mousePressed = true;
		std::cout << "Mouse button 0\n";
		/*switch (button)
		{
			case 0:
				std::cout << "Mouse button 0\n";
				break;

			case 1:
				std::cout << "Mouse button 1\n";
				break;

			case 2:
				std::cout << "Mouse button 2\n";
				break;
		}*/
	};

	// Mouse scroll
	std::function<void(double, double)> mouseScroll = [&](auto xOffset, auto yOffset) {
		m_xOffset = xOffset;
		m_yOffset = yOffset;
	};

	// SetChar
	std::function<void(unsigned int)> textInput = [&](auto codepoint) {

	};

	// Key Pressed
	std::function<void(int, int, int, int)> keyPressed = [&](auto key, auto scancode, auto action, auto mode) {
		if (key >= 0 && key < 1024)
		{
			switch (action)
			{
				// Pressed
			case 1:
				this->m_keys[key] = true;
				break;
			case 0:
				this->m_keys[key] = false;
				break;
			}
		}
	};

	// Window size changed
	std::function<void(int, int)> windowResized = [&](auto width, auto height) {
		this->m_shouldResize = true;
		this->m_width = width;
		this->m_height = height;
	};

private:
	// Keyboard
	std::array<bool, 1024> m_keys;
	std::array<bool, 1024> m_prevKeys;

	// Mouse
	bool m_mouseMoved = false;
	double m_xPos, m_yPos;
	double m_xOffset, m_yOffset;
	bool m_mousePressed = false;

	// Resize
	bool m_shouldResize = false;
	std::size_t m_width, m_height;
};