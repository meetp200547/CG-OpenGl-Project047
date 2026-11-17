#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Texture.h"
#include "SceneObjects.h"
#include "SceneConfig.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {

struct CameraState {
    float yaw = 62.0f;
    float pitch = 13.0f;
    float distance = 2.95f;
    glm::vec3 target = glm::vec3(0.04f, 0.36f, 0.12f);

    bool dragging = false;
    double lastX = 0.0;
    double lastY = 0.0;

    glm::vec3 getPosition() const {
        float yr = glm::radians(yaw);
        float pr = glm::radians(pitch);
        return target + glm::vec3(
            distance * std::cos(pr) * std::cos(yr),
            distance * std::sin(pr),
            distance * std::cos(pr) * std::sin(yr)
        );
    }

    void reset() {
        yaw = 62.0f;
        pitch = 13.0f;
        distance = 2.95f;
        target = glm::vec3(0.04f, 0.36f, 0.12f);
    }
};

CameraState gCamera;
bool gWireframe = false;
bool gVideoPaused = false;
bool gSaveScreenshot = false;
int gWinWidth = 1280;
int gWinHeight = 720;

// Writes the current OpenGL framebuffer to a 24-bit BMP file
bool saveScreenshotBMP(const std::string& filename, int width, int height) {
    std::vector<unsigned char> pixels(static_cast<size_t>(width) * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, pixels.data());

    const uint32_t rowStride = (width * 3 + 3) & ~3u;
    const uint32_t imageSize = rowStride * height;
    const uint32_t fileSize = 54 + imageSize;

    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;

    uint8_t header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    std::memcpy(&header[2], &fileSize, 4);
    uint32_t offset = 54;
    std::memcpy(&header[10], &offset, 4);
    uint32_t infoSize = 40;
    std::memcpy(&header[14], &infoSize, 4);
    std::memcpy(&header[18], &width, 4);
    std::memcpy(&header[22], &height, 4);
    uint16_t planes = 1;
    std::memcpy(&header[26], &planes, 2);
    uint16_t bpp = 24;
    std::memcpy(&header[28], &bpp, 2);
    std::memcpy(&header[34], &imageSize, 4);

    out.write(reinterpret_cast<const char*>(header), 54);

    std::vector<unsigned char> row(rowStride, 0);
    for (int y = 0; y < height; ++y) {
        std::memcpy(row.data(), &pixels[static_cast<size_t>(y) * width * 3], static_cast<size_t>(width) * 3);
        out.write(reinterpret_cast<const char*>(row.data()), rowStride);
    }
    return true;
}

void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    gWinWidth = std::max(width, 1);
    gWinHeight = std::max(height, 1);
    glViewport(0, 0, gWinWidth, gWinHeight);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            gCamera.dragging = true;
            glfwGetCursorPos(window, &gCamera.lastX, &gCamera.lastY);
        } else if (action == GLFW_RELEASE) {
            gCamera.dragging = false;
        }
    }
}

void cursorPosCallback(GLFWwindow*, double xpos, double ypos) {
    if (!gCamera.dragging) return;
    float dx = static_cast<float>(xpos - gCamera.lastX);
    float dy = static_cast<float>(ypos - gCamera.lastY);
    gCamera.lastX = xpos;
    gCamera.lastY = ypos;

    gCamera.yaw += dx * 0.35f;
    gCamera.pitch = std::clamp(gCamera.pitch + dy * 0.35f, -15.0f, 80.0f);
}

void scrollCallback(GLFWwindow*, double, double yoffset) {
    gCamera.distance = std::clamp(gCamera.distance - static_cast<float>(yoffset) * 0.15f, 0.7f, 4.8f);
}

void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    } else if (key == GLFW_KEY_F) {
        gWireframe = !gWireframe;
        glPolygonMode(GL_FRONT_AND_BACK, gWireframe ? GL_LINE : GL_FILL);
    } else if (key == GLFW_KEY_SPACE) {
        gVideoPaused = !gVideoPaused;
    } else if (key == GLFW_KEY_R) {
        gCamera.reset();
    } else if (key == GLFW_KEY_P) {
        gSaveScreenshot = true;
    } else if (key == GLFW_KEY_O || key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) {
        SceneConfig::showExtraBottle = !SceneConfig::showExtraBottle;
        std::cout << (SceneConfig::showExtraBottle ? "Added" : "Removed") << " extra water bottle\n";
    }
}

void processKeyboardPan(GLFWwindow* window, float dt) {
    const float orbitSpeed = 65.0f * dt;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        gCamera.yaw -= orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        gCamera.yaw += orbitSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        gCamera.pitch = std::clamp(gCamera.pitch + orbitSpeed * 0.7f, -15.0f, 80.0f);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        gCamera.pitch = std::clamp(gCamera.pitch - orbitSpeed * 0.7f, -15.0f, 80.0f);
    }
}

void renderShadowCasters(const Shader& activeShader, const SceneAssets& assets) {
    if (SceneConfig::showDoor)           SceneObjects::drawDoor(activeShader, assets);
    if (SceneConfig::showDesk)           SceneObjects::drawDesk(activeShader, assets);
    if (SceneConfig::showDeskMat)        SceneObjects::drawDeskMat(activeShader, assets);
    if (SceneConfig::showMonitor)        SceneObjects::drawMonitor(activeShader, assets);
    if (SceneConfig::showLaptop)         SceneObjects::drawLaptop(activeShader, assets);
    if (SceneConfig::showKeyboard)       SceneObjects::drawMechanicalKeyboard(activeShader, assets);
    if (SceneConfig::showMouse)          SceneObjects::drawMouse(activeShader, assets);
    if (SceneConfig::showWaterBottle)    SceneObjects::drawWaterBottle(activeShader, assets);
    if (SceneConfig::showChair)          SceneObjects::drawChair(activeShader, assets);
    SceneObjects::drawCustomObjects(activeShader, assets);
}

} // namespace

int main(int argc, char** argv) {
    std::string screenshotPath;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--screenshot") {
            screenshotPath = argv[i + 1];
        } else if (std::string(argv[i]) == "--yaw") {
            gCamera.yaw = std::stof(argv[i + 1]);
        } else if (std::string(argv[i]) == "--pitch") {
            gCamera.pitch = std::stof(argv[i + 1]);
        } else if (std::string(argv[i]) == "--dist") {
            gCamera.distance = std::stof(argv[i + 1]);
        }
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    if (!screenshotPath.empty()) {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    }

    GLFWwindow* window = glfwCreateWindow(gWinWidth, gWinHeight, "3D Desktop Setup - OpenGL CG Project", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);

    Shader shader("shaders/scene.vert", "shaders/scene.frag");
    Shader shadowShader("shaders/shadow.vert", "shaders/shadow.frag");

    shader.use();
    shader.setInt("uTexture", 0);
    shader.setInt("uShadowMap", 1);

    // High-resolution Shadow Map Framebuffer (4096x4096) for the tubelight
    const int kShadowMapSize = 4096;
    GLuint shadowFBO = 0;
    GLuint shadowDepthTex = 0;
    glGenFramebuffers(1, &shadowFBO);
    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, kShadowMapSize, kShadowMapSize,
                 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const int videoWidth = 320;
    const int videoHeight = 180;
    SceneAssets assets;
    assets.init(videoWidth, videoHeight);
    std::vector<unsigned char> videoBuffer;

    if (screenshotPath.empty()) {
        std::cout << "=== 3D Desktop Setup Controls ===\n"
                  << "  Mouse Drag / WASD / Arrows : Orbit camera\n"
                  << "  Mouse Scroll               : Zoom in / out\n"
                  << "  Space                      : Play / Pause monitor video\n"
                  << "  O / 0                      : Add / Remove extra water bottle\n"
                  << "  F                          : Toggle wireframe mode\n"
                  << "  P                          : Save screenshot.bmp\n"
                  << "  R                          : Reset camera\n"
                  << "  Esc                        : Exit\n";
    }

    float lastTime = static_cast<float>(glfwGetTime());
    float videoTime = 2.4f;
    int frameCount = 0;

    // Orthographic linear-tubelight shadow frustum covering shelf, front wall, desk, peripherals, chair & carpet
    const glm::vec3 shadowLightPos(0.0f, 1.45f, -0.63f);
    const glm::vec3 shadowLightTarget(0.0f, 0.0f, 0.22f);
    const glm::mat4 lightProjection = glm::ortho(-1.65f, 1.65f, -1.65f, 1.45f, 0.05f, 3.80f);
    const glm::mat4 lightView = glm::lookAt(shadowLightPos, shadowLightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // Laptop screen world-space orientation (root at (-0.78, 0.0, -0.05) rotY +18 deg, lid rotX -16 deg)
    const float yawLap = glm::radians(18.0f);
    const float tiltLap = glm::radians(-16.0f);
    const glm::vec3 laptopRight(std::cos(yawLap), 0.0f, -std::sin(yawLap));
    const glm::vec3 laptopForward = glm::normalize(glm::vec3(
        std::sin(yawLap) * std::cos(tiltLap),
        std::sin(tiltLap),
        std::cos(yawLap) * std::cos(tiltLap)
    ));
    const glm::vec3 laptopUp = glm::normalize(glm::cross(laptopRight, laptopForward));
    const glm::vec3 laptopScreenPos(-0.74f, 0.16f, -0.18f);

    while (!glfwWindowShouldClose(window)) {
        float now = static_cast<float>(glfwGetTime());
        float dt = now - lastTime;
        lastTime = now;

        if (!gVideoPaused) {
            videoTime += dt;
        }

        processKeyboardPan(window, dt);

        glm::vec3 avgVideoColor = Texture::updateVideoTexture(
            assets.videoTex, videoWidth, videoHeight, videoTime, gVideoPaused, videoBuffer
        );

        float fanAngle = now * 160.0f;

        // --- PASS 1: Render Depth Shadow Map from the Tubelight ---
        if (!gWireframe) {
            glViewport(0, 0, kShadowMapSize, kShadowMapSize);
            glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(2.2f, 4.5f);
            shadowShader.use();
            shadowShader.setMat4("uLightSpaceMatrix", lightSpaceMatrix);
            renderShadowCasters(shadowShader, assets);
            glDisable(GL_POLYGON_OFFSET_FILL);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // --- PASS 2: Render Scene with Shadows, Tubelight & Subtle Screen Lights ---
        glViewport(0, 0, gWinWidth, gWinHeight);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

        glm::vec3 camPos = gCamera.getPosition();
        glm::mat4 view = glm::lookAt(camPos, gCamera.target, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            static_cast<float>(gWinWidth) / static_cast<float>(gWinHeight),
            0.05f,
            30.0f
        );

        shader.setMat4("uView", view);
        shader.setMat4("uProjection", projection);
        shader.setMat4("uLightSpaceMatrix", lightSpaceMatrix);
        shader.setVec3("uViewPos", camPos);

        // 1. Primary Light Source: Wall-mounted linear LED tubelight
        shader.setVec3("uTubeLight.startPos", glm::vec3(-0.65f, 1.45f, -0.63f));
        shader.setVec3("uTubeLight.endPos",   glm::vec3( 0.65f, 1.45f, -0.63f));
        shader.setVec3("uTubeLight.color",    SceneConfig::tubeLightColor);
        shader.setFloat("uTubeLight.intensity", SceneConfig::showTubeLight ? SceneConfig::tubeLightIntensity : 0.0f);
        shader.setFloat("uTubeLight.ambientStrength", 0.20f);
        shader.setFloat("uTubeLight.constant", 1.0f);
        shader.setFloat("uTubeLight.linear", 0.22f);
        shader.setFloat("uTubeLight.quadratic", 0.065f);

        // 2. Subtle Screen Glow: Center Monitor (tinted dynamically by live video frame)
        glm::vec3 monitorGlowColor = glm::mix(avgVideoColor, glm::vec3(0.45f, 0.65f, 0.96f), 0.35f);
        shader.setVec3("uMonitorLight.pos",      glm::vec3(0.0f, 0.375f, -0.285f));
        shader.setVec3("uMonitorLight.forward",  glm::vec3(0.0f, 0.0f, 1.0f));
        shader.setVec3("uMonitorLight.right",    glm::vec3(1.0f, 0.0f, 0.0f));
        shader.setVec3("uMonitorLight.up",       glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setVec2("uMonitorLight.halfSize", glm::vec2(0.45f, 0.25f));
        shader.setVec3("uMonitorLight.color",    monitorGlowColor);
        shader.setFloat("uMonitorLight.intensity", SceneConfig::showMonitor ? SceneConfig::monitorLightIntensity : 0.0f);
        shader.setFloat("uMonitorLight.linear",    1.8f);
        shader.setFloat("uMonitorLight.quadratic", 2.8f);

        // 3. Subtle Screen Glow: Left Laptop Display (cool IDE code glow onto keyboard deck & left desk)
        shader.setVec3("uLaptopLight.pos",      laptopScreenPos);
        shader.setVec3("uLaptopLight.forward",  laptopForward);
        shader.setVec3("uLaptopLight.right",    laptopRight);
        shader.setVec3("uLaptopLight.up",       laptopUp);
        shader.setVec2("uLaptopLight.halfSize", glm::vec2(0.21f, 0.12f));
        shader.setVec3("uLaptopLight.color",    glm::vec3(0.38f, 0.60f, 0.92f));
        shader.setFloat("uLaptopLight.intensity", SceneConfig::showLaptop ? SceneConfig::laptopLightIntensity : 0.0f);
        shader.setFloat("uLaptopLight.linear",    2.4f);
        shader.setFloat("uLaptopLight.quadratic", 4.2f);

        // Render room backdrop & all components
        if (SceneConfig::showRoom) SceneObjects::drawRoom(shader, assets);
        // Hide the right-wall door if the camera orbits outside the right wall (x >= 2.05) so it never blocks exterior viewing
        if (SceneConfig::showDoor && camPos.x < 2.05f) {
            SceneObjects::drawDoor(shader, assets);
        }
        if (SceneConfig::showCeilingFan)     SceneObjects::drawCeilingFan(shader, assets, fanAngle);
        if (SceneConfig::showTubeLight)      SceneObjects::drawTubeLight(shader, assets);
        if (SceneConfig::showShelfAndPlants) SceneObjects::drawShelfAndPlants(shader, assets);
        if (SceneConfig::showDesk)           SceneObjects::drawDesk(shader, assets);
        if (SceneConfig::showDeskMat)        SceneObjects::drawDeskMat(shader, assets);
        if (SceneConfig::showMonitor)        SceneObjects::drawMonitor(shader, assets);
        if (SceneConfig::showLaptop)         SceneObjects::drawLaptop(shader, assets);
        if (SceneConfig::showKeyboard)       SceneObjects::drawMechanicalKeyboard(shader, assets);
        if (SceneConfig::showMouse)          SceneObjects::drawMouse(shader, assets);
        if (SceneConfig::showWaterBottle)    SceneObjects::drawWaterBottle(shader, assets);
        if (SceneConfig::showChair)          SceneObjects::drawChair(shader, assets);
        SceneObjects::drawCustomObjects(shader, assets);

        glfwSwapBuffers(window);
        glfwPollEvents();
        ++frameCount;

        if (!screenshotPath.empty() && frameCount >= 2) {
            saveScreenshotBMP(screenshotPath, gWinWidth, gWinHeight);
            break;
        }
        if (gSaveScreenshot) {
            gSaveScreenshot = false;
            saveScreenshotBMP("screenshot.bmp", gWinWidth, gWinHeight);
            std::cout << "Saved screenshot.bmp\n";
        }
    }

    glDeleteFramebuffers(1, &shadowFBO);
    glDeleteTextures(1, &shadowDepthTex);
    assets.destroy();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
