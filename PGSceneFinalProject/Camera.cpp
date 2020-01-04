//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"
#include "glm/gtx/string_cast.hpp"

namespace gps {

	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget) {
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;
		this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
		this->up = glm::vec3(0.0f, 1.0f, 0.0f);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, this->up));
	}

	glm::mat4 Camera::getViewMatrix() {
		return glm::lookAt(cameraPosition, cameraPosition + cameraDirection, up);
	}

	glm::vec3 Camera::getCameraDirection() {
		return this->cameraDirection;
	}

	glm::vec3 Camera::getCameraPosition() {
		return this->cameraPosition;
	}

	void Camera::setTerrainCollision(TerrainCollision terrainCollision) {
		this->terrainCollision = terrainCollision;
	}

	void Camera::setCameraDirection(glm::vec3 cameraDirection) {
		this->cameraDirection = glm::normalize(cameraDirection);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, this->up));
	}

	void Camera::setCameraPosition(glm::vec3 cameraPosition) {
		this->cameraPosition = cameraPosition;
		this->cameraTarget = this->cameraDirection + cameraPosition;

		printf("New camera position: %s\n", glm::to_string(this->cameraPosition).c_str());
	}

	void Camera::setModels(std::vector<Model3D*> models) {
		this->models = models;
	}

	void Camera::move(MOVE_DIRECTION direction, float speed) {
		glm::vec3 newPosition;

		switch (direction) {
		case MOVE_FORWARD:
			newPosition = cameraPosition + cameraDirection * speed;
			break;

		case MOVE_BACKWARD:
			newPosition = cameraPosition - cameraDirection * speed;
			break;

		case MOVE_RIGHT:
			newPosition = cameraPosition + cameraRightDirection * speed;
			break;

		case MOVE_LEFT:
			newPosition = cameraPosition - cameraRightDirection * speed;
			break;
		}

		bool isAbove = terrainCollision.isAbove(newPosition);
		if (!isAbove) {
			newPosition = cameraPosition;
		}

		for (auto const& model : models) {
			if (model->checkIfPointInside(newPosition)) {
				newPosition = cameraPosition;
			}
		}

		this->setCameraPosition(newPosition);
		std::cout << isAbove << std::endl;
		printf("Camera position: %5f %5f %5f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
	}

	void Camera::rotate(float pitch, float yaw) {
		pitch = glm::radians(pitch);
		yaw = glm::radians(yaw);

		glm::vec3 direction = glm::vec3(
			cos(yaw) * cos(pitch),
			sin(pitch),
			sin(yaw) * cos(pitch)
		);

		setCameraDirection(direction);
		printf("Camera direction: %5f %5f %5f\n", cameraDirection.x, cameraDirection.y, cameraDirection.z);
		return;
	}
}
