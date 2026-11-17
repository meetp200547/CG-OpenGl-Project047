#include "SceneObjects.h"
#include "SceneConfig.h"
#include "Texture.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace {

constexpr int SURFACE_DEFAULT = 0;
constexpr int SURFACE_WOOD    = 1;
constexpr int SURFACE_WALL    = 2;
constexpr int SURFACE_CARPET  = 3;
constexpr int SURFACE_FLOOR   = 4;

void drawPart(const Shader& shader,
              const Mesh& mesh,
              const glm::mat4& model,
              const glm::vec3& baseColor,
              float shininess,
              float specularStrength,
              GLuint textureId = 0,
              const glm::vec2& texScale = glm::vec2(1.0f),
              float emissiveStrength = 0.0f,
              int surfaceType = SURFACE_DEFAULT) {
    shader.setMat4("uModel", model);
    shader.setVec3("uMaterial.baseColor", baseColor);
    shader.setFloat("uMaterial.shininess", shininess);
    shader.setFloat("uMaterial.specularStrength", specularStrength);
    shader.setFloat("uMaterial.emissiveStrength", emissiveStrength);
    shader.setInt("uMaterial.surfaceType", surfaceType);
    shader.setVec2("uTexScale", texScale);

    if (textureId != 0) {
        shader.setBool("uMaterial.useTexture", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
    } else {
        shader.setBool("uMaterial.useTexture", false);
    }

    mesh.draw();
}

} // namespace

void SceneAssets::init(int videoWidth, int videoHeight) {
    cube = Mesh::createCube();
    quad = Mesh::createQuad();
    cylinder = Mesh::createCylinder(36);
    sphere = Mesh::createSphere(20, 32);

    // Beveled & rounded meshes for realistic furniture edges and textiles
    tabletopMesh      = Mesh::createRoundedBox(2.50f, 0.048f, 1.16f, 0.036f, 0.010f, 12, 6);
    tableEdgeTrimMesh = Mesh::createRoundedRim(2.50f, 0.048f, 1.16f, 0.036f, 0.010f, 12, 6);
    deskMatMesh       = Mesh::createRoundedBox(1.08f, 0.007f, 0.46f, 0.020f, 0.003f, 8, 4);
    carpetMesh        = Mesh::createRoundedBox(2.90f, 0.012f, 2.10f, 0.048f, 0.0055f, 10, 5);
    shelfMesh         = Mesh::createRoundedBox(1.56f, 0.038f, 0.25f, 0.012f, 0.005f, 8, 4);

    chairBackMesh = Mesh::createCurvedPanel(28, 36, 0.085f, 0.065f, 0.030f, 0.14f);
    chairHeadMesh = Mesh::createCurvedPanel(20, 16, 0.030f, 0.085f, 0.045f, 0.05f);

    woodTex = Texture::createWoodTexture(1024);
    deskMatTex = Texture::createDeskMatTexture(512);
    matteBlackTex = Texture::createMatteBlackTexture(256);
    bottleTex = Texture::createBottleTexture(256);
    chairMeshTex = Texture::createChairMeshTexture(256);
    wallTex = Texture::createWallTexture(1024);
    floorTex = Texture::createFloorTexture(512);
    carpetTex = Texture::createCarpetTexture(1024);
    laptopScreenTex = Texture::createLaptopScreenTexture(512, 320);
    videoTex = Texture::createVideoTexture(videoWidth, videoHeight);
}

void SceneAssets::destroy() {
    GLuint textures[] = {
        woodTex, deskMatTex, matteBlackTex, bottleTex,
        chairMeshTex, wallTex, floorTex, carpetTex, laptopScreenTex, videoTex
    };
    glDeleteTextures(10, textures);
    woodTex = deskMatTex = matteBlackTex = bottleTex = 0;
    chairMeshTex = wallTex = floorTex = carpetTex = laptopScreenTex = videoTex = 0;
}

namespace SceneObjects {

// 0. ROOM BACKDROP: Enclosed 4-wall plaster room + baseboards + ceiling + slate floor + tufted area carpet
void drawRoom(const Shader& shader, const SceneAssets& assets) {
    // Enable backface culling for the room quads so orbiting outside the room still shows the interior against black
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Room spans X: [-2.10, +2.10] (4.2m), Y: [-0.78, +1.82] (2.6m), Z: [-0.72, +3.60] (4.32m)
    const float roomDepth = 4.32f;
    const float roomCenterZ = 1.44f;

    // Base dark slate tile floor
    glm::mat4 floorMat(1.0f);
    floorMat = glm::translate(floorMat, glm::vec3(0.0f, -0.78f, roomCenterZ));
    floorMat = glm::rotate(floorMat, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    floorMat = glm::scale(floorMat, glm::vec3(4.2f, roomDepth, 1.0f));
    drawPart(shader, assets.quad, floorMat, SceneConfig::floorTint, 18.0f, 0.12f,
             assets.floorTex, glm::vec2(3.5f, 3.6f), 0.0f, SURFACE_FLOOR);

    // Grey plaster wall behind the desk (z = -0.72, facing +Z)
    glm::mat4 frontWall(1.0f);
    frontWall = glm::translate(frontWall, glm::vec3(0.0f, 0.52f, -0.72f));
    frontWall = glm::scale(frontWall, glm::vec3(4.2f, 2.6f, 1.0f));
    drawPart(shader, assets.quad, frontWall, SceneConfig::wallTint, 6.0f, 0.025f,
             assets.wallTex, glm::vec2(2.4f, 1.6f), 0.0f, SURFACE_WALL);

    // Grey back wall behind the chair/room (z = +3.60, facing -Z)
    glm::mat4 backWall(1.0f);
    backWall = glm::translate(backWall, glm::vec3(0.0f, 0.52f, 3.60f));
    backWall = glm::rotate(backWall, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    backWall = glm::scale(backWall, glm::vec3(4.2f, 2.6f, 1.0f));
    drawPart(shader, assets.quad, backWall, SceneConfig::wallTint, 6.0f, 0.025f,
             assets.wallTex, glm::vec2(2.4f, 1.6f), 0.0f, SURFACE_WALL);

    // Grey left wall (x = -2.10, facing +X)
    glm::mat4 leftWall(1.0f);
    leftWall = glm::translate(leftWall, glm::vec3(-2.10f, 0.52f, roomCenterZ));
    leftWall = glm::rotate(leftWall, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    leftWall = glm::scale(leftWall, glm::vec3(roomDepth, 2.6f, 1.0f));
    drawPart(shader, assets.quad, leftWall, SceneConfig::wallTint, 6.0f, 0.025f,
             assets.wallTex, glm::vec2(2.5f, 1.6f), 0.0f, SURFACE_WALL);

    // Grey right wall (x = +2.10, facing -X)
    glm::mat4 rightWall(1.0f);
    rightWall = glm::translate(rightWall, glm::vec3(2.10f, 0.52f, roomCenterZ));
    rightWall = glm::rotate(rightWall, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    rightWall = glm::scale(rightWall, glm::vec3(roomDepth, 2.6f, 1.0f));
    drawPart(shader, assets.quad, rightWall, SceneConfig::wallTint, 6.0f, 0.025f,
             assets.wallTex, glm::vec2(2.5f, 1.6f), 0.0f, SURFACE_WALL);

    // Matte ceiling (y = +1.82, facing -Y)
    glm::mat4 ceiling(1.0f);
    ceiling = glm::translate(ceiling, glm::vec3(0.0f, 1.82f, roomCenterZ));
    ceiling = glm::rotate(ceiling, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    ceiling = glm::scale(ceiling, glm::vec3(4.2f, roomDepth, 1.0f));
    drawPart(shader, assets.quad, ceiling, SceneConfig::ceilingTint, 5.0f, 0.02f,
             assets.wallTex, glm::vec2(2.4f, 2.5f), 0.0f, SURFACE_WALL);

    glDisable(GL_CULL_FACE);

    // Architectural skirting baseboards along the bottom of the walls (grounding walls to the floor)
    const glm::vec3 baseboardColor = SceneConfig::baseboardColor;
    const float bbH = 0.072f;
    const float bbY = -0.78f + bbH * 0.5f;

    // Front wall baseboard
    glm::mat4 bbFront(1.0f);
    bbFront = glm::translate(bbFront, glm::vec3(0.0f, bbY, -0.712f));
    bbFront = glm::scale(bbFront, glm::vec3(4.20f, bbH, 0.016f));
    drawPart(shader, assets.cube, bbFront, baseboardColor, 18.0f, 0.16f);

    // Left wall baseboard
    glm::mat4 bbLeft(1.0f);
    bbLeft = glm::translate(bbLeft, glm::vec3(-2.092f, bbY, roomCenterZ));
    bbLeft = glm::scale(bbLeft, glm::vec3(0.016f, bbH, roomDepth));
    drawPart(shader, assets.cube, bbLeft, baseboardColor, 18.0f, 0.16f);

    // Back wall baseboard
    glm::mat4 bbBack(1.0f);
    bbBack = glm::translate(bbBack, glm::vec3(0.0f, bbY, 3.592f));
    bbBack = glm::scale(bbBack, glm::vec3(4.20f, bbH, 0.016f));
    drawPart(shader, assets.cube, bbBack, baseboardColor, 18.0f, 0.16f);

    // Right wall baseboard (split cleanly on either side of the wooden door frame at z in [1.73, 2.67])
    glm::mat4 bbRight1(1.0f);
    bbRight1 = glm::translate(bbRight1, glm::vec3(2.092f, bbY, 0.505f));
    bbRight1 = glm::scale(bbRight1, glm::vec3(0.016f, bbH, 2.45f));
    drawPart(shader, assets.cube, bbRight1, baseboardColor, 18.0f, 0.16f);

    glm::mat4 bbRight2(1.0f);
    bbRight2 = glm::translate(bbRight2, glm::vec3(2.092f, bbY, 3.135f));
    bbRight2 = glm::scale(bbRight2, glm::vec3(0.016f, bbH, 0.93f));
    drawPart(shader, assets.cube, bbRight2, baseboardColor, 18.0f, 0.16f);

    // Beveled & tufted heather-grey woven area carpet with serged border trim
    glm::mat4 carpet(1.0f);
    carpet = glm::translate(carpet, glm::vec3(0.0f, -0.774f, 0.45f));
    drawPart(shader, assets.carpetMesh, carpet, SceneConfig::carpetTint, 5.0f, 0.03f,
             assets.carpetTex, glm::vec2(1.0f, 1.0f), 0.0f, SURFACE_CARPET);
}

// 0a-door. WOODEN DOOR / GATE: Placed on the right wall (x = +2.10) toward the back-right of the room
void drawDoor(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 doorWood  = SceneConfig::doorWoodColor;
    const glm::vec3 frameWood = SceneConfig::doorFrameColor;
    const glm::vec3 knobMetal = SceneConfig::doorKnobColor;

    const float wallX = 2.10f;
    const float doorZ = 2.20f;
    const float floorY = -0.78f;
    const float doorH = 2.02f;
    const float doorW = 0.88f;

    // Outer wooden door frame (left jamb, right jamb, top header)
    glm::mat4 leftJamb(1.0f);
    leftJamb = glm::translate(leftJamb, glm::vec3(wallX - 0.025f, floorY + doorH * 0.5f, doorZ - doorW * 0.5f - 0.03f));
    leftJamb = glm::scale(leftJamb, glm::vec3(0.05f, doorH + 0.06f, 0.06f));
    drawPart(shader, assets.cube, leftJamb, frameWood, 24.0f, 0.28f, assets.woodTex, glm::vec2(1.0f), 0.0f, SURFACE_WOOD);

    glm::mat4 rightJamb(1.0f);
    rightJamb = glm::translate(rightJamb, glm::vec3(wallX - 0.025f, floorY + doorH * 0.5f, doorZ + doorW * 0.5f + 0.03f));
    rightJamb = glm::scale(rightJamb, glm::vec3(0.05f, doorH + 0.06f, 0.06f));
    drawPart(shader, assets.cube, rightJamb, frameWood, 24.0f, 0.28f, assets.woodTex, glm::vec2(1.0f), 0.0f, SURFACE_WOOD);

    glm::mat4 topHeader(1.0f);
    topHeader = glm::translate(topHeader, glm::vec3(wallX - 0.025f, floorY + doorH + 0.03f, doorZ));
    topHeader = glm::scale(topHeader, glm::vec3(0.05f, 0.06f, doorW + 0.12f));
    drawPart(shader, assets.cube, topHeader, frameWood, 24.0f, 0.28f, assets.woodTex, glm::vec2(1.0f), 0.0f, SURFACE_WOOD);

    // Main wooden door leaf
    glm::mat4 leaf(1.0f);
    leaf = glm::translate(leaf, glm::vec3(wallX - 0.015f, floorY + doorH * 0.5f, doorZ));
    leaf = glm::scale(leaf, glm::vec3(0.030f, doorH, doorW));
    drawPart(shader, assets.cube, leaf, doorWood, 28.0f, 0.30f, assets.woodTex, glm::vec2(1.0f, 2.0f), 0.0f, SURFACE_WOOD);

    // Upper & lower raised wooden molding panels
    const float panelHeights[2] = {floorY + 1.44f, floorY + 0.56f};
    const float panelSizesY[2] = {0.72f, 0.72f};
    for (int i = 0; i < 2; ++i) {
        glm::mat4 panel(1.0f);
        panel = glm::translate(panel, glm::vec3(wallX - 0.033f, panelHeights[i], doorZ));
        panel = glm::scale(panel, glm::vec3(0.010f, panelSizesY[i], doorW - 0.22f));
        drawPart(shader, assets.cube, panel, doorWood * 0.90f, 28.0f, 0.30f, assets.woodTex, glm::vec2(1.0f), 0.0f, SURFACE_WOOD);
    }

    // Metallic doorknob + rosette on the left side of the door
    const float knobZ = doorZ - doorW * 0.5f + 0.09f;
    const float knobY = floorY + 0.98f;

    glm::mat4 rosette(1.0f);
    rosette = glm::translate(rosette, glm::vec3(wallX - 0.036f, knobY, knobZ));
    rosette = glm::rotate(rosette, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    rosette = glm::scale(rosette, glm::vec3(0.060f, 0.012f, 0.060f));
    drawPart(shader, assets.cylinder, rosette, knobMetal, 80.0f, 0.85f);

    glm::mat4 knobStem(1.0f);
    knobStem = glm::translate(knobStem, glm::vec3(wallX - 0.055f, knobY, knobZ));
    knobStem = glm::rotate(knobStem, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    knobStem = glm::scale(knobStem, glm::vec3(0.022f, 0.035f, 0.022f));
    drawPart(shader, assets.cylinder, knobStem, knobMetal, 80.0f, 0.85f);

    glm::mat4 knobBall(1.0f);
    knobBall = glm::translate(knobBall, glm::vec3(wallX - 0.080f, knobY, knobZ));
    knobBall = glm::scale(knobBall, glm::vec3(0.050f, 0.050f, 0.050f));
    drawPart(shader, assets.sphere, knobBall, knobMetal, 96.0f, 0.90f);
}

// 0a. CEILING FAN: Mounted at center of ceiling with rotating angled blades
void drawCeilingFan(const Shader& shader, const SceneAssets& assets, float angleDeg) {
    const glm::vec3 fanBody    = SceneConfig::fanBodyColor;
    const glm::vec3 bladeColor = SceneConfig::fanBladeColor;
    const glm::vec3 trimSilver = SceneConfig::fanTrimColor;

    glm::mat4 root(1.0f);
    root = glm::translate(root, glm::vec3(0.0f, 1.82f, 0.48f));

    glm::mat4 canopy = glm::translate(root, glm::vec3(0.0f, -0.025f, 0.0f));
    canopy = glm::scale(canopy, glm::vec3(0.14f, 0.05f, 0.14f));
    drawPart(shader, assets.cylinder, canopy, fanBody, 40.0f, 0.45f, assets.matteBlackTex);

    glm::mat4 rod = glm::translate(root, glm::vec3(0.0f, -0.14f, 0.0f));
    rod = glm::scale(rod, glm::vec3(0.032f, 0.22f, 0.032f));
    drawPart(shader, assets.cylinder, rod, fanBody, 48.0f, 0.55f);

    glm::mat4 spin = glm::translate(root, glm::vec3(0.0f, -0.25f, 0.0f));
    spin = glm::rotate(spin, glm::radians(angleDeg), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 motor = glm::scale(spin, glm::vec3(0.24f, 0.075f, 0.24f));
    drawPart(shader, assets.cylinder, motor, fanBody, 48.0f, 0.55f, assets.matteBlackTex);

    glm::mat4 cap = glm::translate(spin, glm::vec3(0.0f, -0.042f, 0.0f));
    cap = glm::scale(cap, glm::vec3(0.12f, 0.015f, 0.12f));
    drawPart(shader, assets.cylinder, cap, trimSilver, 72.0f, 0.75f);

    for (int i = 0; i < 3; ++i) {
        glm::mat4 bladePivot = glm::rotate(spin, glm::radians(i * 120.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 bracket = glm::translate(bladePivot, glm::vec3(0.15f, 0.005f, 0.0f));
        bracket = glm::scale(bracket, glm::vec3(0.10f, 0.014f, 0.045f));
        drawPart(shader, assets.cube, bracket, trimSilver, 56.0f, 0.60f);

        glm::mat4 blade = glm::translate(bladePivot, glm::vec3(0.46f, 0.005f, 0.0f));
        blade = glm::rotate(blade, glm::radians(12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        blade = glm::scale(blade, glm::vec3(0.56f, 0.010f, 0.135f));
        drawPart(shader, assets.cube, blade, bladeColor, 32.0f, 0.38f, assets.matteBlackTex);
    }
}

// 0b. TUBELIGHT: Wall-mounted LED tubelight above the shelf that illuminates the room
void drawTubeLight(const Shader& shader, const SceneAssets& assets) {
    glm::mat4 rail(1.0f);
    rail = glm::translate(rail, glm::vec3(0.0f, 1.46f, -0.70f));
    rail = glm::scale(rail, glm::vec3(1.36f, 0.032f, 0.035f));
    drawPart(shader, assets.cube, rail, glm::vec3(0.86f, 0.87f, 0.89f), 48.0f, 0.50f);

    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 cap(1.0f);
        cap = glm::translate(cap, glm::vec3(side * 0.67f, 1.45f, -0.665f));
        cap = glm::scale(cap, glm::vec3(0.025f, 0.042f, 0.055f));
        drawPart(shader, assets.cube, cap, glm::vec3(0.82f, 0.83f, 0.85f), 32.0f, 0.40f);
    }

    // Glowing LED tube cylinder
    glm::mat4 tube(1.0f);
    tube = glm::translate(tube, glm::vec3(0.0f, 1.45f, -0.665f));
    tube = glm::rotate(tube, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    tube = glm::scale(tube, glm::vec3(0.034f, 1.32f, 0.034f));
    drawPart(shader, assets.cylinder, tube, glm::vec3(0.98f, 0.99f, 1.0f), 16.0f, 0.1f, 0, glm::vec2(1.0f), 1.35f);
}

// 0c. SHELF & PLANTS: Beveled floating wood shelf above the monitor with books, potted plants & blossom vase
void drawShelfAndPlants(const Shader& shader, const SceneAssets& assets) {
    const float shelfY = 0.94f;
    const float shelfZ = -0.59f;

    // Beveled floating warm-wood shelf
    glm::mat4 shelf(1.0f);
    shelf = glm::translate(shelf, glm::vec3(0.0f, shelfY, shelfZ));
    drawPart(shader, assets.shelfMesh, shelf, SceneConfig::shelfWoodTint, 28.0f, 0.28f,
             assets.woodTex, glm::vec2(1.5f, 0.5f), 0.0f, SURFACE_WOOD);

    const float topY = shelfY + 0.019f;

    // --- LEFT SIDE: Upright books + small pot + ribbed white pot with trailing green plant ---
    glm::mat4 bookBig(1.0f);
    bookBig = glm::translate(bookBig, glm::vec3(-0.62f, topY + 0.12f, shelfZ - 0.03f));
    bookBig = glm::scale(bookBig, glm::vec3(0.035f, 0.24f, 0.17f));
    drawPart(shader, assets.cube, bookBig, glm::vec3(0.76f, 0.72f, 0.66f), 12.0f, 0.15f);

    glm::mat4 bookFront(1.0f);
    bookFront = glm::translate(bookFront, glm::vec3(-0.56f, topY + 0.085f, shelfZ + 0.02f));
    bookFront = glm::rotate(bookFront, glm::radians(-12.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    bookFront = glm::scale(bookFront, glm::vec3(0.12f, 0.17f, 0.028f));
    drawPart(shader, assets.cube, bookFront, glm::vec3(0.22f, 0.28f, 0.24f), 16.0f, 0.20f);

    // Small white ceramic pot
    glm::mat4 smallPot(1.0f);
    smallPot = glm::translate(smallPot, glm::vec3(-0.40f, topY + 0.042f, shelfZ + 0.01f));
    smallPot = glm::scale(smallPot, glm::vec3(0.075f, 0.084f, 0.075f));
    drawPart(shader, assets.cylinder, smallPot, glm::vec3(0.90f, 0.89f, 0.86f), 32.0f, 0.35f);

    for (int i = 0; i < 7; ++i) {
        float a = glm::radians(i * 51.4f);
        glm::mat4 leaf(1.0f);
        leaf = glm::translate(leaf, glm::vec3(-0.40f + 0.035f * std::cos(a), topY + 0.098f + (i % 2) * 0.015f, shelfZ + 0.01f + 0.035f * std::sin(a)));
        leaf = glm::rotate(leaf, a, glm::vec3(0.0f, 1.0f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(28.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        leaf = glm::scale(leaf, glm::vec3(0.055f, 0.014f, 0.030f));
        drawPart(shader, assets.sphere, leaf, glm::vec3(0.22f, 0.42f, 0.18f), 20.0f, 0.22f);
    }

    // Main ribbed white planter pot (pedestal + bowl)
    glm::mat4 potBase(1.0f);
    potBase = glm::translate(potBase, glm::vec3(-0.22f, topY + 0.025f, shelfZ + 0.01f));
    potBase = glm::scale(potBase, glm::vec3(0.085f, 0.050f, 0.085f));
    drawPart(shader, assets.cylinder, potBase, SceneConfig::potColor, 32.0f, 0.35f);

    glm::mat4 potBowl(1.0f);
    potBowl = glm::translate(potBowl, glm::vec3(-0.22f, topY + 0.085f, shelfZ + 0.01f));
    potBowl = glm::scale(potBowl, glm::vec3(0.135f, 0.095f, 0.135f));
    drawPart(shader, assets.cylinder, potBowl, SceneConfig::potColor, 36.0f, 0.38f);

    // Lush green foliage cluster + trailing vine leaves over front edge
    const glm::vec3 leafGreen = SceneConfig::plantLeafColor;
    const glm::vec3 leafLight = SceneConfig::plantLeafLight;
    for (int i = 0; i < 16; ++i) {
        float a = glm::radians(i * 22.5f);
        float rad = 0.045f + (i % 3) * 0.032f;
        float h = 0.145f + (i % 4) * 0.025f;
        glm::mat4 leaf(1.0f);
        leaf = glm::translate(leaf, glm::vec3(-0.22f + rad * std::cos(a), topY + h, shelfZ + 0.01f + rad * std::sin(a)));
        leaf = glm::rotate(leaf, -a, glm::vec3(0.0f, 1.0f, 0.0f));
        leaf = glm::rotate(leaf, glm::radians(35.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        leaf = glm::scale(leaf, glm::vec3(0.075f, 0.016f, 0.044f));
        drawPart(shader, assets.sphere, leaf, (i % 2 == 0) ? leafGreen : leafLight, 24.0f, 0.25f);
    }
    for (int v = 0; v < 5; ++v) {
        glm::mat4 vine(1.0f);
        vine = glm::translate(vine, glm::vec3(-0.24f + v * 0.012f, topY + 0.04f - v * 0.032f, shelfZ + 0.11f + v * 0.008f));
        vine = glm::rotate(vine, glm::radians(55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        vine = glm::scale(vine, glm::vec3(0.038f, 0.012f, 0.048f));
        drawPart(shader, assets.sphere, vine, leafGreen, 24.0f, 0.25f);
    }

    // --- RIGHT SIDE: Stacked books + candle jar + upright book + cream vase with pink blossoms ---
    glm::mat4 hBook1(1.0f);
    hBook1 = glm::translate(hBook1, glm::vec3(0.28f, topY + 0.014f, shelfZ + 0.01f));
    hBook1 = glm::scale(hBook1, glm::vec3(0.23f, 0.028f, 0.16f));
    drawPart(shader, assets.cube, hBook1, glm::vec3(0.88f, 0.88f, 0.87f), 14.0f, 0.15f);

    glm::mat4 hBook2(1.0f);
    hBook2 = glm::translate(hBook2, glm::vec3(0.285f, topY + 0.039f, shelfZ + 0.01f));
    hBook2 = glm::scale(hBook2, glm::vec3(0.21f, 0.022f, 0.15f));
    drawPart(shader, assets.cube, hBook2, glm::vec3(0.82f, 0.82f, 0.80f), 14.0f, 0.15f);

    glm::mat4 candle(1.0f);
    candle = glm::translate(candle, glm::vec3(0.32f, topY + 0.075f, shelfZ + 0.01f));
    candle = glm::scale(candle, glm::vec3(0.068f, 0.052f, 0.068f));
    drawPart(shader, assets.cylinder, candle, glm::vec3(0.76f, 0.74f, 0.70f), 48.0f, 0.50f);

    glm::mat4 vBook(1.0f);
    vBook = glm::translate(vBook, glm::vec3(0.42f, topY + 0.105f, shelfZ + 0.01f));
    vBook = glm::scale(vBook, glm::vec3(0.028f, 0.21f, 0.15f));
    drawPart(shader, assets.cube, vBook, glm::vec3(0.84f, 0.72f, 0.76f), 14.0f, 0.15f);

    glm::mat4 vaseBody(1.0f);
    vaseBody = glm::translate(vaseBody, glm::vec3(0.54f, topY + 0.085f, shelfZ + 0.01f));
    vaseBody = glm::scale(vaseBody, glm::vec3(0.11f, 0.17f, 0.11f));
    drawPart(shader, assets.cylinder, vaseBody, SceneConfig::vaseColor, 42.0f, 0.42f);

    glm::mat4 vaseNeck(1.0f);
    vaseNeck = glm::translate(vaseNeck, glm::vec3(0.54f, topY + 0.178f, shelfZ + 0.01f));
    vaseNeck = glm::scale(vaseNeck, glm::vec3(0.058f, 0.032f, 0.058f));
    drawPart(shader, assets.cylinder, vaseNeck, SceneConfig::vaseColor, 42.0f, 0.42f);

    for (int b = 0; b < 4; ++b) {
        float ang = glm::radians(-24.0f + b * 16.0f);
        glm::mat4 stem(1.0f);
        stem = glm::translate(stem, glm::vec3(0.54f + (b - 1.5f) * 0.025f, topY + 0.26f, shelfZ + 0.01f));
        stem = glm::rotate(stem, ang, glm::vec3(0.0f, 0.0f, 1.0f));
        stem = glm::scale(stem, glm::vec3(0.008f, 0.16f, 0.008f));
        drawPart(shader, assets.cylinder, stem, glm::vec3(0.26f, 0.18f, 0.14f), 12.0f, 0.12f);

        for (int f = 0; f < 3; ++f) {
            glm::mat4 blossom(1.0f);
            float fx = 0.54f + (b - 1.5f) * 0.040f + (f - 1) * 0.015f;
            float fy = topY + 0.25f + f * 0.042f;
            blossom = glm::translate(blossom, glm::vec3(fx, fy, shelfZ + 0.01f + (f % 2) * 0.015f));
            blossom = glm::scale(blossom, glm::vec3(0.036f, 0.032f, 0.032f));
            drawPart(shader, assets.sphere, blossom, (f % 2 == 0) ? SceneConfig::flowerColorA : SceneConfig::flowerColorB, 16.0f, 0.18f);
        }
    }
}

// 1. DESK: Beveled golden-oak tabletop with flush bullnose edge-banding, under-frame apron & 4 legs with glides
void drawDesk(const Shader& shader, const SceneAssets& assets) {
    // Beveled & rounded golden-oak tabletop (top surface at y = 0.0, thickness = 0.048m)
    glm::mat4 top(1.0f);
    top = glm::translate(top, glm::vec3(0.0f, -0.024f, 0.0f));
    drawPart(shader, assets.tabletopMesh, top, SceneConfig::deskWoodTint, 40.0f, 0.38f,
             assets.woodTex, glm::vec2(2.2f, 1.1f), 0.0f, SURFACE_WOOD);

    // Flush molded satin-black bullnose PVC edge-banding wrapping smoothly around all rounded corners & sides
    drawPart(shader, assets.tableEdgeTrimMesh, top, SceneConfig::deskEdgeTrimColor, 56.0f, 0.62f,
             assets.matteBlackTex, glm::vec2(1.0f, 1.0f));

    // Recessed black powder-coated steel structural apron rails beneath the tabletop
    const glm::vec3 steelBlack = SceneConfig::deskSteelColor;
    for (int side = -1; side <= 1; side += 2) {
        glm::mat4 longRail(1.0f);
        longRail = glm::translate(longRail, glm::vec3(0.0f, -0.066f, side * 0.48f));
        longRail = glm::scale(longRail, glm::vec3(2.34f, 0.036f, 0.032f));
        drawPart(shader, assets.cube, longRail, steelBlack, 36.0f, 0.38f, assets.matteBlackTex);

        glm::mat4 sideRail(1.0f);
        sideRail = glm::translate(sideRail, glm::vec3(side * 1.16f, -0.066f, 0.0f));
        sideRail = glm::scale(sideRail, glm::vec3(0.032f, 0.036f, 0.96f));
        drawPart(shader, assets.cube, sideRail, steelBlack, 36.0f, 0.38f, assets.matteBlackTex);
    }

    // Four black metal legs with top mounting collars & floor leveling glides (reaching y = -0.78)
    const float legPositions[4][2] = {
        {-1.16f, -0.48f}, {1.16f, -0.48f},
        {-1.16f,  0.48f}, {1.16f,  0.48f}
    };
    for (int i = 0; i < 4; ++i) {
        float lx = legPositions[i][0];
        float lz = legPositions[i][1];

        glm::mat4 leg(1.0f);
        leg = glm::translate(leg, glm::vec3(lx, -0.41f, lz));
        leg = glm::scale(leg, glm::vec3(0.052f, 0.715f, 0.052f));
        drawPart(shader, assets.cube, leg, steelBlack, 36.0f, 0.40f, assets.matteBlackTex);

        // Top mounting flange plate
        glm::mat4 flange(1.0f);
        flange = glm::translate(flange, glm::vec3(lx, -0.052f, lz));
        flange = glm::scale(flange, glm::vec3(0.085f, 0.008f, 0.085f));
        drawPart(shader, assets.cube, flange, steelBlack, 36.0f, 0.40f);

        // Brushed chrome & rubber adjustable leveling foot glide on the carpet
        glm::mat4 footChrome(1.0f);
        footChrome = glm::translate(footChrome, glm::vec3(lx, -0.764f, lz));
        footChrome = glm::scale(footChrome, glm::vec3(0.044f, 0.010f, 0.044f));
        drawPart(shader, assets.cylinder, footChrome, glm::vec3(0.55f, 0.57f, 0.60f), 72.0f, 0.75f);

        glm::mat4 footPad(1.0f);
        footPad = glm::translate(footPad, glm::vec3(lx, -0.772f, lz));
        footPad = glm::scale(footPad, glm::vec3(0.062f, 0.008f, 0.062f));
        drawPart(shader, assets.cylinder, footPad, glm::vec3(0.05f, 0.05f, 0.06f), 16.0f, 0.18f);
    }
}

// 2. DESKMAT: Rounded-corner black graphic deskmat with beveled stitched edge
void drawDeskMat(const Shader& shader, const SceneAssets& assets) {
    glm::mat4 mat(1.0f);
    mat = glm::translate(mat, glm::vec3(0.08f, 0.0035f, 0.23f));
    drawPart(shader, assets.deskMatMesh, mat, SceneConfig::deskMatTint, 8.0f, 0.08f, assets.deskMatTex);
}

// 3. MONITOR (Center): Black Acer-style monitor with circular round base + live video
void drawMonitor(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 blackPlastic = SceneConfig::monitorChassisColor;
    const glm::vec3 darkMetal    = SceneConfig::monitorStandColor;

    glm::mat4 base(1.0f);
    base = glm::translate(base, glm::vec3(0.0f, 0.007f, -0.32f));
    base = glm::scale(base, glm::vec3(0.28f, 0.014f, 0.26f));
    drawPart(shader, assets.cylinder, base, darkMetal, 42.0f, 0.48f, assets.matteBlackTex);

    glm::mat4 column(1.0f);
    column = glm::translate(column, glm::vec3(0.0f, 0.18f, -0.36f));
    column = glm::scale(column, glm::vec3(0.036f, 0.34f, 0.036f));
    drawPart(shader, assets.cylinder, column, darkMetal, 42.0f, 0.48f, assets.matteBlackTex);

    glm::mat4 arm(1.0f);
    arm = glm::translate(arm, glm::vec3(0.0f, 0.32f, -0.335f));
    arm = glm::scale(arm, glm::vec3(0.05f, 0.05f, 0.06f));
    drawPart(shader, assets.cube, arm, darkMetal, 40.0f, 0.45f, assets.matteBlackTex);

    glm::mat4 chassis(1.0f);
    chassis = glm::translate(chassis, glm::vec3(0.0f, 0.37f, -0.30f));
    chassis = glm::scale(chassis, glm::vec3(0.94f, 0.54f, 0.024f));
    drawPart(shader, assets.cube, chassis, blackPlastic, 28.0f, 0.35f, assets.matteBlackTex);

    glm::mat4 screen(1.0f);
    screen = glm::translate(screen, glm::vec3(0.0f, 0.375f, -0.287f));
    screen = glm::scale(screen, glm::vec3(0.91f, 0.505f, 1.0f));
    drawPart(shader, assets.quad, screen, glm::vec3(1.0f), 16.0f, 0.1f, assets.videoTex, glm::vec2(1.0f), 1.0f);
}

// 4. LAPTOP (Left side): Black Dell G15-style gaming laptop with vent deck, backlit keys, IDE screen
void drawLaptop(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 blackChassis = SceneConfig::laptopChassisColor;
    const glm::vec3 ventDark(0.04f, 0.04f, 0.05f);

    glm::mat4 root(1.0f);
    root = glm::translate(root, glm::vec3(-0.78f, 0.0f, -0.05f));
    root = glm::rotate(root, glm::radians(18.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 base = glm::translate(root, glm::vec3(0.0f, 0.010f, 0.0f));
    base = glm::scale(base, glm::vec3(0.45f, 0.020f, 0.31f));
    drawPart(shader, assets.cube, base, blackChassis, 38.0f, 0.44f, assets.matteBlackTex);

    for (int i = 0; i < 12; ++i) {
        float vx = -0.18f + i * 0.032f;
        glm::mat4 vent = glm::translate(root, glm::vec3(vx, 0.021f, -0.115f));
        vent = glm::scale(vent, glm::vec3(0.022f, 0.003f, 0.035f));
        drawPart(shader, assets.cube, vent, ventDark, 16.0f, 0.20f);
    }

    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 13; ++c) {
            float kx = -0.17f + c * 0.026f;
            float kz = -0.065f + r * 0.024f;
            glm::mat4 key = glm::translate(root, glm::vec3(kx, 0.021f, kz));
            key = glm::scale(key, glm::vec3(0.021f, 0.003f, 0.019f));
            drawPart(shader, assets.cube, key, SceneConfig::laptopKeyColor, 22.0f, 0.30f);
        }
    }

    glm::mat4 trackpad = glm::translate(root, glm::vec3(-0.02f, 0.021f, 0.085f));
    trackpad = glm::scale(trackpad, glm::vec3(0.13f, 0.002f, 0.082f));
    drawPart(shader, assets.cube, trackpad, glm::vec3(0.11f, 0.11f, 0.12f), 42.0f, 0.42f);

    glm::mat4 lidPivot = glm::translate(root, glm::vec3(0.0f, 0.020f, -0.150f));
    lidPivot = glm::rotate(lidPivot, glm::radians(-16.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 lid = glm::translate(lidPivot, glm::vec3(0.0f, 0.140f, 0.0f));
    lid = glm::scale(lid, glm::vec3(0.45f, 0.28f, 0.010f));
    drawPart(shader, assets.cube, lid, blackChassis, 38.0f, 0.44f, assets.matteBlackTex);

    glm::mat4 screen = glm::translate(lidPivot, glm::vec3(0.0f, 0.143f, 0.006f));
    screen = glm::scale(screen, glm::vec3(0.42f, 0.245f, 1.0f));
    drawPart(shader, assets.quad, screen, glm::vec3(1.0f), 16.0f, 0.1f, assets.laptopScreenTex, glm::vec2(1.0f), 0.85f);
}

// 5. SMALL MECHANICAL KEYBOARD (65% compact black) on the deskmat
void drawMechanicalKeyboard(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 caseBlack   = SceneConfig::keyboardCaseColor;
    const glm::vec3 keycapBlack = SceneConfig::keyboardKeyColor;

    glm::mat4 root(1.0f);
    root = glm::translate(root, glm::vec3(0.02f, 0.019f, 0.22f));
    root = glm::rotate(root, glm::radians(4.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 frame = glm::scale(root, glm::vec3(0.37f, 0.020f, 0.135f));
    drawPart(shader, assets.cube, frame, caseBlack, 36.0f, 0.42f, assets.matteBlackTex);

    glm::mat4 plate = glm::translate(root, glm::vec3(0.0f, 0.0105f, 0.0f));
    plate = glm::scale(plate, glm::vec3(0.345f, 0.002f, 0.115f));
    drawPart(shader, assets.cube, plate, SceneConfig::keyboardGlowColor, 16.0f, 0.2f, 0, glm::vec2(1.0f), 0.45f);

    const int rows = 5;
    const float keyPitchX = 0.023f;
    const float keyPitchZ = 0.022f;
    const float startX = -0.152f;
    const float startZ = -0.044f;

    for (int r = 0; r < rows; ++r) {
        float z = startZ + r * keyPitchZ;
        if (r < 4) {
            for (int c = 0; c < 14; ++c) {
                float x = startX + c * keyPitchX;
                glm::mat4 key = glm::translate(root, glm::vec3(x, 0.015f, z));
                key = glm::scale(key, glm::vec3(0.0185f, 0.009f, 0.0180f));
                drawPart(shader, assets.cube, key, keycapBlack, 28.0f, 0.38f, assets.matteBlackTex);
            }
        } else {
            const float bottomOffsets[] = {-0.142f, -0.114f, -0.086f, 0.000f, 0.086f, 0.114f, 0.142f};
            const float bottomWidths[]  = { 0.024f,  0.024f,  0.024f, 0.132f, 0.024f, 0.024f, 0.024f};
            for (int i = 0; i < 7; ++i) {
                glm::mat4 key = glm::translate(root, glm::vec3(bottomOffsets[i], 0.015f, z));
                key = glm::scale(key, glm::vec3(bottomWidths[i], 0.009f, 0.0180f));
                drawPart(shader, assets.cube, key, keycapBlack, 28.0f, 0.38f, assets.matteBlackTex);
            }
        }
    }
}

// 6. MOUSE (Black ergonomic wireless mouse) on the deskmat
void drawMouse(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 mouseBlack = SceneConfig::mouseBodyColor;

    glm::mat4 root(1.0f);
    root = glm::translate(root, glm::vec3(0.38f, 0.008f, 0.24f));

    glm::mat4 body = glm::translate(root, glm::vec3(0.0f, 0.015f, 0.0f));
    body = glm::scale(body, glm::vec3(0.068f, 0.036f, 0.115f));
    drawPart(shader, assets.sphere, body, mouseBlack, 52.0f, 0.58f, assets.matteBlackTex);

    glm::mat4 base = glm::translate(root, glm::vec3(0.0f, 0.004f, 0.0f));
    base = glm::scale(base, glm::vec3(0.056f, 0.008f, 0.096f));
    drawPart(shader, assets.cylinder, base, glm::vec3(0.04f, 0.04f, 0.05f), 16.0f, 0.2f);

    glm::mat4 wheel = glm::translate(root, glm::vec3(0.0f, 0.029f, -0.028f));
    wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    wheel = glm::scale(wheel, glm::vec3(0.016f, 0.007f, 0.016f));
    drawPart(shader, assets.cylinder, wheel, SceneConfig::mouseWheelColor, 64.0f, 0.65f);
}

// 7. WATER BOTTLE (Right side): Tall brushed stainless-steel thermos
void drawWaterBottleAt(const Shader& shader, const SceneAssets& assets, const glm::vec3& pos) {
    glm::mat4 root(1.0f);
    root = glm::translate(root, pos);

    glm::mat4 body = glm::translate(root, glm::vec3(0.0f, 0.135f, 0.0f));
    body = glm::scale(body, glm::vec3(0.095f, 0.27f, 0.095f));
    drawPart(shader, assets.cylinder, body, SceneConfig::bottleTint, 96.0f, 0.85f, assets.bottleTex);

    glm::mat4 shoulder = glm::translate(root, glm::vec3(0.0f, 0.270f, 0.0f));
    shoulder = glm::scale(shoulder, glm::vec3(0.095f, 0.036f, 0.095f));
    drawPart(shader, assets.sphere, shoulder, SceneConfig::bottleTint * 0.72f, 96.0f, 0.88f);

    glm::mat4 cap = glm::translate(root, glm::vec3(0.0f, 0.298f, 0.0f));
    cap = glm::scale(cap, glm::vec3(0.068f, 0.032f, 0.068f));
    drawPart(shader, assets.cylinder, cap, SceneConfig::bottleCapColor, 96.0f, 0.90f);
}

void drawWaterBottle(const Shader& shader, const SceneAssets& assets) {
    drawWaterBottleAt(shader, assets, glm::vec3(0.94f, 0.0f, -0.04f));
}

// 8. ERGONOMIC CHAIR (In front of desk):
//    5-star base + casters, gas lift, black cushion seat, grey ribbed mesh backrest & headrest, T-armrests
void drawChair(const Shader& shader, const SceneAssets& assets) {
    const glm::vec3 frameBlack   = SceneConfig::chairFrameColor;
    const glm::vec3 cushionBlack = SceneConfig::chairSeatColor;
    const glm::vec3 meshGrey     = SceneConfig::chairMeshColor;

    glm::mat4 root(1.0f);
    root = glm::translate(root, glm::vec3(0.04f, -0.78f, 0.84f));
    root = glm::rotate(root, glm::radians(-10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    for (int i = 0; i < 5; ++i) {
        float angle = glm::radians(i * 72.0f);
        glm::mat4 armPivot = glm::rotate(root, angle, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 leg = glm::translate(armPivot, glm::vec3(0.155f, 0.095f, 0.0f));
        leg = glm::rotate(leg, glm::radians(-12.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        leg = glm::scale(leg, glm::vec3(0.30f, 0.032f, 0.042f));
        drawPart(shader, assets.cube, leg, frameBlack, 36.0f, 0.42f, assets.matteBlackTex);

        glm::mat4 wheel = glm::translate(armPivot, glm::vec3(0.295f, 0.028f, 0.0f));
        wheel = glm::rotate(wheel, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        wheel = glm::scale(wheel, glm::vec3(0.056f, 0.042f, 0.056f));
        drawPart(shader, assets.cylinder, wheel, glm::vec3(0.05f, 0.05f, 0.06f), 24.0f, 0.30f);
    }

    glm::mat4 hub = glm::translate(root, glm::vec3(0.0f, 0.13f, 0.0f));
    hub = glm::scale(hub, glm::vec3(0.09f, 0.06f, 0.09f));
    drawPart(shader, assets.cylinder, hub, frameBlack, 36.0f, 0.42f);

    glm::mat4 gasLift = glm::translate(root, glm::vec3(0.0f, 0.26f, 0.0f));
    gasLift = glm::scale(gasLift, glm::vec3(0.048f, 0.24f, 0.048f));
    drawPart(shader, assets.cylinder, gasLift, frameBlack, 48.0f, 0.55f);

    glm::mat4 mech = glm::translate(root, glm::vec3(0.0f, 0.39f, 0.02f));
    mech = glm::scale(mech, glm::vec3(0.22f, 0.055f, 0.24f));
    drawPart(shader, assets.cube, mech, frameBlack, 32.0f, 0.38f, assets.matteBlackTex);

    glm::mat4 seat = glm::translate(root, glm::vec3(0.0f, 0.45f, -0.01f));
    seat = glm::scale(seat, glm::vec3(0.52f, 0.105f, 0.50f));
    drawPart(shader, assets.sphere, seat, cushionBlack, 10.0f, 0.14f, assets.matteBlackTex);

    glm::mat4 spineLower = glm::translate(root, glm::vec3(0.0f, 0.47f, 0.24f));
    spineLower = glm::rotate(spineLower, glm::radians(-26.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    spineLower = glm::scale(spineLower, glm::vec3(0.085f, 0.22f, 0.038f));
    drawPart(shader, assets.cube, spineLower, frameBlack, 36.0f, 0.42f, assets.matteBlackTex);

    glm::mat4 backTransform = glm::translate(root, glm::vec3(0.0f, 0.79f, 0.27f));
    backTransform = glm::rotate(backTransform, glm::radians(7.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 backFrame = glm::translate(backTransform, glm::vec3(0.0f, 0.0f, 0.020f));
    backFrame = glm::scale(backFrame, glm::vec3(0.49f, 0.61f, 0.85f));
    drawPart(shader, assets.chairBackMesh, backFrame, frameBlack, 36.0f, 0.42f, assets.matteBlackTex);

    glm::mat4 backMesh = glm::scale(backTransform, glm::vec3(0.455f, 0.575f, 0.90f));
    drawPart(shader, assets.chairBackMesh, backMesh, meshGrey, 14.0f, 0.18f, assets.chairMeshTex, glm::vec2(2.0f, 4.5f));

    glm::mat4 neck = glm::translate(root, glm::vec3(0.0f, 1.11f, 0.31f));
    neck = glm::rotate(neck, glm::radians(-12.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    neck = glm::scale(neck, glm::vec3(0.060f, 0.11f, 0.026f));
    drawPart(shader, assets.cube, neck, frameBlack, 36.0f, 0.42f);

    glm::mat4 headTransform = glm::translate(root, glm::vec3(0.0f, 1.21f, 0.27f));
    headTransform = glm::rotate(headTransform, glm::radians(-8.0f), glm::vec3(1.0f, 0.0f, 0.0f));

    glm::mat4 headFrame = glm::translate(headTransform, glm::vec3(0.0f, 0.0f, 0.018f));
    headFrame = glm::scale(headFrame, glm::vec3(0.30f, 0.155f, 0.85f));
    drawPart(shader, assets.chairHeadMesh, headFrame, frameBlack, 34.0f, 0.40f);

    glm::mat4 headMesh = glm::scale(headTransform, glm::vec3(0.28f, 0.138f, 0.90f));
    drawPart(shader, assets.chairHeadMesh, headMesh, meshGrey, 14.0f, 0.18f, assets.chairMeshTex, glm::vec2(1.5f, 1.5f));

    for (int side = -1; side <= 1; side += 2) {
        float ax = side * 0.28f;
        glm::mat4 bracket = glm::translate(root, glm::vec3(side * 0.20f, 0.41f, 0.02f));
        bracket = glm::scale(bracket, glm::vec3(0.18f, 0.030f, 0.045f));
        drawPart(shader, assets.cube, bracket, frameBlack, 34.0f, 0.40f);

        glm::mat4 post = glm::translate(root, glm::vec3(ax, 0.52f, 0.02f));
        post = glm::scale(post, glm::vec3(0.042f, 0.22f, 0.050f));
        drawPart(shader, assets.cube, post, frameBlack, 34.0f, 0.40f, assets.matteBlackTex);

        glm::mat4 pad = glm::translate(root, glm::vec3(ax, 0.635f, -0.01f));
        pad = glm::scale(pad, glm::vec3(0.085f, 0.032f, 0.24f));
        drawPart(shader, assets.sphere, pad, glm::vec3(0.08f, 0.08f, 0.09f), 26.0f, 0.30f, assets.matteBlackTex);
    }
}

// ============================================================================
// 9. QUICK CUSTOM OBJECTS (Add new objects here in 1-3 lines!)
// ============================================================================
// Tabletop surface is at y = 0.0. Room floor is at y = -0.78.
// Helpers: addBox / addCylinder / addSphere(shader, mesh, pos, size, color)
// ============================================================================
void drawCustomObjects(const Shader& shader, const SceneAssets& assets) {
    auto addShape = [&](const Mesh& mesh, glm::vec3 pos, glm::vec3 size, glm::vec3 color,
                        float shininess = 32.0f, float spec = 0.35f) {
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0f), pos), size);
        drawPart(shader, mesh, m, color, shininess, spec);
    };
    (void)addShape;
    (void)assets;

    // Extra similar water bottle on the right side of the desk (toggled live with the 'O' key)
    if (SceneConfig::showExtraBottle) {
        drawWaterBottleAt(shader, assets, glm::vec3(0.80f, 0.0f, 0.06f));
    }
}

} // namespace SceneObjects
