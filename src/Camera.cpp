#include "Camera.h"
#include "Input.h"

#include <GLFW/glfw3.h>

/***********************************************************************************/
Camera::Camera() noexcept
{
	updateVectors();
}

/***********************************************************************************/
void Camera::SetNear(const float near)
{
	m_near = near;
}

/***********************************************************************************/
void Camera::SetFar(const float far)
{
	m_far = far;
}

/***********************************************************************************/
void Camera::SetSpeed(const float speed)
{
	m_speed = speed;
}

/***********************************************************************************/
void Camera::Update(const double deltaTime)
{

	if (Input::GetInstance().IsKeyPressed(GLFW_KEY_TAB))
	{
		m_dirty = !m_dirty;
	}

	if (m_dirty)
	{
		// Update view from mouse movement
		updateView();

		// Update Keyboard
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_W)) { processKeyboard(Direction::FORWARD, deltaTime); }
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_S)) { processKeyboard(Direction::BACKWARD, deltaTime); }
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_A)) { processKeyboard(Direction::LEFT, deltaTime); }
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_D)) { processKeyboard(Direction::RIGHT, deltaTime); }
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_SPACE)) { processKeyboard(Direction::UP, deltaTime); }
		if (Input::GetInstance().IsKeyHeld(GLFW_KEY_LEFT_CONTROL)) { processKeyboard(Direction::DOWN, deltaTime); }
	}
}

/***********************************************************************************/
void Camera::processKeyboard(const Direction direction, const double deltaTime) noexcept
{
	const float velocity = m_speed * static_cast<float>(deltaTime);

	switch (direction)
	{
	case Direction::FORWARD:
		m_position += m_front * velocity;
		break;
	case Direction::BACKWARD:
		m_position -= m_front * velocity;
		break;
	case Direction::LEFT:
		m_position -= m_right * velocity;
		break;
	case Direction::RIGHT:
		m_position += m_right * velocity;
		break;
	case Direction::UP:
		m_position += m_worldUp * velocity;
		break;
	case Direction::DOWN:
		m_position -= m_worldUp * velocity;
		break;
	}
}

/***********************************************************************************/
void Camera::updateView(const bool constrainPitch /*= true*/)
{

	// If the mouse position has changed, recalculate vectors
	if (Input::GetInstance().MouseMoved())
	{
		const auto xPos = Input::GetInstance().GetMouseX();
		const auto yPos = Input::GetInstance().GetMouseY();

		if (m_firstMouse)
		{
			m_prevX = xPos;
			m_prevY = yPos;
			m_firstMouse = false;
		}

		const auto xOffset = (float)(xPos - m_prevX) * m_sensitivity;
		const auto yOffset = (float)(m_prevY - yPos) * m_sensitivity; // Reversed since y-coordinates go from bottom to top

		m_prevX = xPos;
		m_prevY = yPos;

		m_yaw += xOffset;
		m_pitch += yOffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (m_pitch > 89.0) { m_pitch = 89.0; }
			if (m_pitch < -89.0) { m_pitch = -89.0; }
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateVectors();
	}
}

/***********************************************************************************/
void Camera::updateVectors()
{
	// Calculate the new Front vector
	glm::vec3 front{
			front.x = glm::cos(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch)),
			front.y = glm::sin(glm::radians(m_pitch)),
			front.z = glm::sin(glm::radians(m_yaw)) * glm::cos(glm::radians(m_pitch))
	};

	m_front = glm::normalize(front);
	m_right = glm::normalize(glm::cross(m_front, m_worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	m_up = glm::normalize(glm::cross(m_right, m_front));
}