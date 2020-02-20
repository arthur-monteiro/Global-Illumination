#include "Camera.h"

void Camera::initialize(glm::vec3 position, glm::vec3 target, glm::vec3 verticalAxis, float sensibility, float speed, float aspect)
{
	m_position = position;
	m_target = target;
	m_verticalAxis = verticalAxis;
	m_sensibility = sensibility;
	m_speed = speed;
	m_aspect = aspect;

	setTarget(m_target);
}

void Camera::update(GLFWwindow* window)
{
	if (m_oldMousePosX < 0 || m_fixed)
	{
		glfwGetCursorPos(window, &m_oldMousePosX, &m_oldMousePosY);
		return;
	}

	auto currentTime = std::chrono::high_resolution_clock::now();
	long long microsecondOffset = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - m_startTime).count();
	float secondOffset = microsecondOffset / 1'000'000.0;
	m_startTime = currentTime;

	double currentMousePosX, currentMousePosY;
	glfwGetCursorPos(window, &currentMousePosX, &currentMousePosY);

	updateOrientation(currentMousePosX - m_oldMousePosX, currentMousePosY - m_oldMousePosY);
	m_oldMousePosX = currentMousePosX;
	m_oldMousePosY = currentMousePosY;

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		m_position = m_position + m_orientation * (secondOffset * m_speed);
		m_target = m_position + m_orientation;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		m_position = m_position - m_orientation * (secondOffset * m_speed);
		m_target = m_position + m_orientation;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		m_position = m_position + m_lateralDirection * (secondOffset * m_speed);
		m_target = m_position + m_orientation;
	}

	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		m_position = m_position - m_lateralDirection * (secondOffset * m_speed);
		m_target = m_position + m_orientation;
	}
}

glm::mat4 Camera::getViewMatrix(glm::vec3 forceOrientation)
{
	if (forceOrientation.x == -1.0f)
		return glm::lookAt(m_position, m_target, m_verticalAxis);
		
	return glm::lookAt(m_position, glm::normalize(m_position + forceOrientation), m_verticalAxis);
}

glm::vec3 Camera::getPosition()
{
	return m_position;
}

glm::mat4 Camera::getProjection()
{
	glm::mat4 r = glm::perspective(m_radFOV, m_aspect, m_near, m_far);
	r[1][1] *= -1;

	return r;
}

void Camera::setPosition(glm::vec3 position)
{
}

void Camera::setTarget(glm::vec3 target)
{
	m_orientation = m_target - m_position;
	m_orientation = glm::normalize(m_orientation);

	if (m_verticalAxis.x == 1.0)
	{
		m_phi = glm::asin(m_orientation.x);
		m_theta = glm::acos(m_orientation.y / glm::cos(m_phi));

		if (m_orientation.y < 0)
			m_theta *= -1;
	}

	else if (m_verticalAxis.y == 1.0)
	{
		m_phi = glm::asin(m_orientation.y);
		m_theta = glm::acos(m_orientation.z / glm::cos(m_phi));

		if (m_orientation.z < 0)
			m_theta *= -1;
	}

	else
	{
		m_phi = glm::asin(m_orientation.x);
		m_theta = glm::acos(m_orientation.z / glm::cos(m_phi));

		if (m_orientation.z < 0)
			m_theta *= -1;
	}
}

void Camera::updateOrientation(int xOffset, int yOffset)
{
	m_phi -= yOffset * m_sensibility;
	m_theta -= xOffset * m_sensibility;

	if (m_phi > glm::half_pi<float>() - OFFSET_ANGLES)
		m_phi = glm::half_pi<float>() - OFFSET_ANGLES;
	else if (m_phi < -glm::half_pi<float>() + OFFSET_ANGLES)
		m_phi = -glm::half_pi<float>() + OFFSET_ANGLES;

	if (m_verticalAxis.x == 1.0)
	{
		m_orientation.x = glm::sin(m_phi);
		m_orientation.y = glm::cos(m_phi) * glm::cos(m_theta);
		m_orientation.z = glm::cos(m_phi) * glm::sin(m_theta);
	}

	else if (m_verticalAxis.y == 1.0)
	{
		m_orientation.x = glm::cos(m_phi) * glm::sin(m_theta);
		m_orientation.y = glm::sin(m_phi);
		m_orientation.z = glm::cos(m_phi) * glm::cos(m_theta);
	}

	else
	{
		m_orientation.x = glm::cos(m_phi) * glm::cos(m_theta);
		m_orientation.y = glm::cos(m_phi) * glm::sin(m_theta);
		m_orientation.z = glm::sin(m_phi);
	}

	m_lateralDirection = glm::normalize(glm::cross(m_verticalAxis, m_orientation));
	m_target = m_position + m_orientation;
}