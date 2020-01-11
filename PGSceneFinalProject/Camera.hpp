//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "TerrainCollision.hpp"
#include "Model3D.hpp"

namespace gps {

	enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

	class Camera
	{

	public:
		Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget);
		glm::mat4 getViewMatrix();
		void move(MOVE_DIRECTION direction, float speed);
		void rotate(float pitch, float yaw);
		glm::vec3 getCameraDirection();
		glm::vec3 getCameraPosition();
		glm::vec3 getCameaTarget();
		void setTerrainCollision(TerrainCollision terrainCollision);
		void setCameraDirection(glm::vec3 cameraDirection);
		void setCameraPosition(glm::vec3 cameraPosition);
		void setModels(std::vector<Model3D*> models);
		void setDetectCollision();

	private:
		TerrainCollision terrainCollision;
		glm::vec3 cameraPosition;
		glm::vec3 cameraTarget;
		glm::vec3 cameraDirection;
		glm::vec3 cameraRightDirection;
		glm::vec3 up;
		std::vector<Model3D*> models;
		bool detectCollision;

	};

}

#endif /* Camera_hpp */
