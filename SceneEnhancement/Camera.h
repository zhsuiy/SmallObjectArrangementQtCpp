#ifndef CAMERA_H
#define CAMERA_H
#include <QtGui/qmatrix4x4.h>
#include <QtMath>
#include <iostream>

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 3.0f;
const float SENSITIVTY = 0.25f;
const float ZOOM = 45.0f;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

class Camera
{
public:

	// camera attributes
	QVector3D Position;
	QVector3D Front;
	QVector3D Up;
	QVector3D Right;
	QVector3D WorldUp;

	// Eular angles
	float Yaw;
	float Pitch;

	// camera options
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

private:
	QVector3D orgPosition;
	QVector3D orgUp;
	float orgYaw;
	float orgPitch;
	
public:

	// Constructor with vectors
	Camera(QVector3D position = QVector3D(0.0f, 0.0f, 0.0f), QVector3D up = QVector3D(0.0f, 1.0f, 0.0f),
		float yaw = YAW, float pitch = PITCH, float zoom = ZOOM) : Front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(zoom)
	{
		orgPosition = position;
		orgUp = up;
		orgYaw = yaw;
		orgPitch = pitch;
		this->Position = position;
		this->WorldUp = up;
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(QVector3D(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVTY), Zoom(ZOOM)
	{
		this->Position = QVector3D(posX, posY, posZ);
		this->WorldUp = QVector3D(upX, upY, upZ);
		this->Yaw = yaw;
		this->Pitch = pitch;
		this->updateCameraVectors();
	}

	// Returns the view matrix calculated using Eular Angles and the LookAt Matrix
	QMatrix4x4 GetViewMatrix()
	{
		QMatrix4x4 view;
		view.lookAt(this->Position, this->Position + this->Front, this->Up);
		return view;
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime = 0.1)
	{
		float velocity = this->MovementSpeed * deltaTime;
		if (direction == FORWARD)
			this->Position += this->Front * velocity;
		if (direction == BACKWARD)
			this->Position -= this->Front * velocity;
		if (direction == LEFT)
			this->Position -= this->Right * velocity;
		if (direction == RIGHT)
			this->Position += this->Right * velocity;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
	{
		xoffset *= this->MouseSensitivity;
		yoffset *= this->MouseSensitivity;

		this->Yaw += xoffset;
		this->Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (this->Pitch > 89.0f)
				this->Pitch = 89.0f;
			if (this->Pitch < -89.0f)
				this->Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		this->updateCameraVectors();
		//std::cout << "Yaw: " << Yaw << ", Pitch:  " << Pitch << std::endl;
	}

	void ProcessRightMouseMovement(float xoffset,float yoffset, float deltaTime = 0.1)
	{
		xoffset = xoffset * this->MouseSensitivity * 0.1;
		yoffset = yoffset * this->MouseSensitivity * 0.1;

		this->Position += this->Right * xoffset;
		//updateCameraVectors();
		this->Position += this->Up * yoffset;
		//std::cout << "Position: " << Position.x()  << ","<< Position.y() <<","<< Position.z() << std::endl;

	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		this->Position += this->Front * yoffset * MouseSensitivity;
		/*if (this->Zoom >= 1.0f && this->Zoom <= 45.0f)
			this->Zoom -= yoffset;
		if (this->Zoom <= 1.0f)
			this->Zoom = 1.0f;
		if (this->Zoom >= 45.0f)
			this->Zoom = 45.0f;*/
	}

	void Reset()
	{
		this->Position = orgPosition;
		this->WorldUp = orgUp;
		this->Front = QVector3D(0.0f, 0.0f, -1.0f);
		this->Yaw = orgYaw;
		this->Pitch = orgPitch;
		this->updateCameraVectors();
		Zoom = ZOOM;
		MouseSensitivity = SENSITIVTY;
		MovementSpeed = SPEED;
	}

private:
	// Calculates the front vector from the Camera's (updated) Eular Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		QVector3D front;
		front.setX(cos(qDegreesToRadians(this->Yaw)) * cos(qDegreesToRadians(this->Pitch)));
		front.setY(sin(qDegreesToRadians(this->Pitch)));
		front.setZ(sin(qDegreesToRadians(this->Yaw)) * cos(qDegreesToRadians(this->Pitch)));
		this->Front = front.normalized();
		// Also re-calculate the Right and Up vector
		
		//this->Right = glm::normalize(glm::cross(this->Front, this->WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->Right = QVector3D::crossProduct(this->Front, this->WorldUp).normalized();
		//this->Up = glm::normalize(glm::cross(this->Right, this->Front));
		this->Up = QVector3D::crossProduct(this->Right, this->Front).normalized();
	}
};


#endif

