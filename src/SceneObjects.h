#pragma once

#include "Mesh.h"
#include "Shader.h"
#include <GL/glew.h>
#include <glm/glm.hpp>

// Holds all shared primitive meshes and procedural textures used by the scene.
struct SceneAssets {
    Mesh cube;
    Mesh quad;
    Mesh cylinder;
    Mesh sphere;
    Mesh tabletopMesh;
    Mesh tableEdgeTrimMesh;
    Mesh deskMatMesh;
    Mesh carpetMesh;
    Mesh shelfMesh;
    Mesh chairBackMesh;
    Mesh chairHeadMesh;

    GLuint woodTex = 0;
    GLuint deskMatTex = 0;
    GLuint matteBlackTex = 0;
    GLuint bottleTex = 0;
    GLuint chairMeshTex = 0;
    GLuint wallTex = 0;
    GLuint floorTex = 0;
    GLuint carpetTex = 0;
    GLuint laptopScreenTex = 0;
    GLuint videoTex = 0;

    void init(int videoWidth, int videoHeight);
    void destroy();
};

// Individual, self-contained component renderers for easy explanation:
namespace SceneObjects {

void drawRoom(const Shader& shader, const SceneAssets& assets);
void drawDoor(const Shader& shader, const SceneAssets& assets);
void drawCeilingFan(const Shader& shader, const SceneAssets& assets, float angleDeg);
void drawTubeLight(const Shader& shader, const SceneAssets& assets);
void drawShelfAndPlants(const Shader& shader, const SceneAssets& assets);
void drawDesk(const Shader& shader, const SceneAssets& assets);
void drawDeskMat(const Shader& shader, const SceneAssets& assets);
void drawMonitor(const Shader& shader, const SceneAssets& assets);
void drawLaptop(const Shader& shader, const SceneAssets& assets);
void drawMechanicalKeyboard(const Shader& shader, const SceneAssets& assets);
void drawMouse(const Shader& shader, const SceneAssets& assets);
void drawWaterBottle(const Shader& shader, const SceneAssets& assets);
void drawChair(const Shader& shader, const SceneAssets& assets);
void drawCustomObjects(const Shader& shader, const SceneAssets& assets);

} // namespace SceneObjects
