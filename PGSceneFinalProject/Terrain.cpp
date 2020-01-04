
#include "Terrain.hpp"
#include "Helpers.hpp"
#include "Noise.hpp"

namespace gps {

  Terrain::Terrain(void) {

  }

  Terrain::Terrain(int verticesCount) {
    this->verticesCount = verticesCount;
    this->wasGenerated = false;
    this->terrainMesh = this->generateTerrain();
  }

  void Terrain::Draw(gps::Shader shader) {
    if (!this->wasGenerated) {
      fprintf(stderr, "Terrain can not be drawn because it was not generated.");
      return;
    }

    this->terrainMesh.Draw(shader);
  }

  Mesh Terrain::generateTerrain() {
    std::vector<gps::Vertex> vertices;
    std::vector<GLuint> indices;
    std::vector<gps::Texture> textures;

    float vx, vy, vz;

		Noise noiseGenerator;

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-.5, +.5);
    auto rng = std::bind(distribution, generator);

    for (int i = 0; i < this->verticesCount; i++) {
      for (int j = 0; j < this->verticesCount; j++) {
        vx = 2 * (float)j / (float)(this->verticesCount - 1) - 1.f; // cover the whole plane: [0, 1] to [-1, 1]
        vz = 2 * (float)i / (float)(this->verticesCount - 1) - 1.f;
				vy = noiseGenerator.getHeightAt(j, i) * .5; //(getHeightAtCoord(j, i) + 1.0f) * .5; // [-1, 1] to [0, 1]

				//printf("Height at (%2d %2d) %5f\n", j, i, vz);
        glm::vec3 vertexPosition(vx, vy, vz);
        glm::vec3 vertexNormal = glm::vec3(0.0f, 1.0f, 0.0f); // default normal, will be overriden
        glm::vec2 vertexTexCoords(0, 0);

        gps::Vertex currentVertex;
        currentVertex.Position = vertexPosition;
        currentVertex.Normal = vertexNormal;
        currentVertex.TexCoords = vertexTexCoords;

        vertices.push_back(currentVertex);
      }
    }

    // Compute normals for each point using its 4 neighbours
    /*
      OBSOLOTE, useless, redundant, unsatisfactory, unpleasing, faulty, buggy

      for (int i = 1; i < this->verticesCount - 1; i++) {
        for (int j = 1; j < this->verticesCount - 1; j++) {
          int index = i * this->verticesCount + j;

          // Get east, west, south and north neighbours
          float left  = vertices.at(index - 1).Position.z;
          float right = vertices.at(index + 1).Position.z;
          float up		= vertices.at((i + 1) * this->verticesCount + j).Position.z;
          float down  = vertices.at((i - 1) * this->verticesCount + j).Position.z;

          float nx = (left + right) / 2.0f;
          float nz = (up + down) / 2.0f;

          vertices.at(index).Normal = glm::vec3(nx, 1.0f, nz);
        }
      }
    */

    // The normal of each vertex position is equal to
    // normalize(sum(normal of triangle(i) adjiacent to our vertex))


    GLuint innerTrianglePairsCount = this->verticesCount - 1;
    for (int i = 0; i < innerTrianglePairsCount; i++) {
      for (int j = 0; j < innerTrianglePairsCount; j++) {

        // Get the 4 points indices enclosing our pair of triangles
        GLuint bottomLt = i * this->verticesCount + j;
        GLuint bottomRt = bottomLt + 1;
        GLuint topLt = (i + 1) * this->verticesCount + j;
        GLuint topRt = topLt + 1;

        // Get each of 4 vertices
        glm::vec3 vecBottomLt = vertices.at(bottomLt).Position;
        glm::vec3 vecBottomRt = vertices.at(bottomRt).Position;
        glm::vec3 vecTopLt = vertices.at(topLt).Position;
        glm::vec3 vecTopRt = vertices.at(topRt).Position;

        // first triangle
        glm::vec3 ab = vecBottomLt - vecTopLt;
        glm::vec3 ac = vecBottomRt - vecTopLt;
        glm::vec3 normal = glm::normalize(glm::cross(ab, ac));

        indices.push_back(topLt);
        indices.push_back(bottomLt);
        indices.push_back(bottomRt);

        vertices.at(topLt).Normal += normal;
        vertices.at(bottomLt).Normal += normal;
        vertices.at(bottomRt).Normal += normal;

        // second triangle
        ab = vecBottomRt - vecTopLt;
        ac = vecTopRt - vecTopLt;
        normal = glm::normalize(glm::cross(ab, ac));

        indices.push_back(topLt);
        indices.push_back(bottomRt);
        indices.push_back(topRt);

        vertices.at(topLt).Normal += normal;
        vertices.at(bottomLt).Normal += normal;
        vertices.at(bottomRt).Normal += normal;
      }
    }

    // Normalize vertices normals
    for (int i = 0; i < vertices.size(); i++) {
      vertices.at(i).Normal = glm::normalize(vertices.at(i).Normal);
    }

    this->wasGenerated = true;
    return Mesh(vertices, indices, textures);
  }

  float Terrain::getHeightAtCoord(int x, int y) {
    float xf = (float)x;
    float yf = (float)y;
    float maxed = (float)(1 << (octaves - 1));
    float result = 0.0f;

		float amplitude = 1;
    float frequency = .5;

    // wave generation by combination noises at different frequencies and amplitudes
    for (int i = 0; i < octaves; i++) {
      result += perlinNoise.getNoise(glm::vec2(xf, yf) * frequency) * amplitude;
      amplitude *= .5;
      frequency *= 2;
    }

    return result;
  }
}