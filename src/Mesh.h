#pragma once

#include <GL/glew.h>
#include <vector>

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

// Manages a VAO/VBO/EBO for a 3D primitive mesh.
class Mesh {
public:
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;

    Mesh() = default;
    ~Mesh();

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    void draw() const;

    static Mesh createCube();
    static Mesh createQuad();
    static Mesh createCylinder(int segments = 36);
    static Mesh createSphere(int stacks = 20, int slices = 32);
    static Mesh createRoundedBox(float width, float height, float depth,
                                 float cornerRadius, float bevelRadius,
                                 int cornerSegs = 10, int bevelSegs = 6);
    static Mesh createRoundedRim(float width, float height, float depth,
                                 float cornerRadius, float bevelRadius,
                                 int cornerSegs = 10, int bevelSegs = 6);
    static Mesh createCurvedPanel(int cols = 28, int rows = 36, float sCurveAmp = 0.08f,
                                  float cupDepth = 0.06f, float thickness = 0.03f,
                                  float waistPinch = 0.14f);

private:
    void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    void cleanup();
};

