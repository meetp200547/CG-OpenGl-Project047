#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

namespace Texture {

// Creates a 2D OpenGL texture from an RGB byte buffer (3 bytes per pixel).
GLuint createFromRGB(int width, int height, const std::vector<unsigned char>& rgb, bool repeat = true);

// Procedural static textures matching the real desktop & chair setup
GLuint createWoodTexture(int size = 1024);
GLuint createDeskMatTexture(int size = 512);
GLuint createMatteBlackTexture(int size = 256);
GLuint createBottleTexture(int size = 256);
GLuint createChairMeshTexture(int size = 256);
GLuint createWallTexture(int size = 1024);
GLuint createFloorTexture(int size = 512);
GLuint createCarpetTexture(int size = 1024);
GLuint createLaptopScreenTexture(int width = 512, int height = 320);

// Dynamic video texture for the center monitor (updated every frame via glTexSubImage2D)
GLuint createVideoTexture(int width, int height);
glm::vec3 updateVideoTexture(GLuint texId, int width, int height, float timeSec,
                             bool paused, std::vector<unsigned char>& pixelBuffer);

} // namespace Texture
