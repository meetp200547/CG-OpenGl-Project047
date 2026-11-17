#include "Mesh.h"
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <utility>

namespace {
constexpr float PI = 3.14159265359f;

struct PerimPt {
    float x, z;
    float nx, nz;
    float s;
};

std::vector<PerimPt> buildRoundedRectPerimeter(float width, float depth, float cornerRadius, int cornerSegs) {
    const float hx = width * 0.5f;
    const float hz = depth * 0.5f;
    const float rXZ = std::min({cornerRadius, hx * 0.49f, hz * 0.49f});

    // 4 corners in CCW order when viewed from +Y:
    // 0: Front-Left (-X, +Z), theta: PI -> PI/2
    // 1: Front-Right (+X, +Z), theta: PI/2 -> 0
    // 2: Back-Right (+X, -Z), theta: 0 -> -PI/2
    // 3: Back-Left (-X, -Z), theta: -PI/2 -> -PI
    const float cx[4] = {-hx + rXZ,  hx - rXZ,  hx - rXZ, -hx + rXZ};
    const float cz[4] = { hz - rXZ,  hz - rXZ, -hz + rXZ, -hz + rXZ};
    const float a0[4] = { PI,        0.5f * PI, 0.0f,     -0.5f * PI};
    const float a1[4] = { 0.5f * PI, 0.0f,     -0.5f * PI, -PI};

    std::vector<PerimPt> pts;
    const int edgeSubdivs = 8;
    float accumS = 0.0f;

    auto pushPt = [&](float x, float z, float nx, float nz) {
        if (!pts.empty()) {
            float dx = x - pts.back().x;
            float dz = z - pts.back().z;
            accumS += std::sqrt(dx * dx + dz * dz);
        }
        pts.push_back({x, z, nx, nz, accumS});
    };

    for (int c = 0; c < 4; ++c) {
        for (int s = 0; s <= cornerSegs; ++s) {
            float t = static_cast<float>(s) / cornerSegs;
            float ang = a0[c] + t * (a1[c] - a0[c]);
            float nx = std::cos(ang);
            float nz = std::sin(ang);
            pushPt(cx[c] + rXZ * nx, cz[c] + rXZ * nz, nx, nz);
        }
        int nextC = (c + 1) % 4;
        float startX = cx[c] + rXZ * std::cos(a1[c]);
        float startZ = cz[c] + rXZ * std::sin(a1[c]);
        float endX = cx[nextC] + rXZ * std::cos(a0[nextC]);
        float endZ = cz[nextC] + rXZ * std::sin(a0[nextC]);
        float nx = std::cos(a1[c]);
        float nz = std::sin(a1[c]);
        for (int e = 1; e < edgeSubdivs; ++e) {
            float t = static_cast<float>(e) / edgeSubdivs;
            pushPt(startX + t * (endX - startX), startZ + t * (endZ - startZ), nx, nz);
        }
    }
    pushPt(pts.front().x, pts.front().z, pts.front().nx, pts.front().nz);
    return pts;
}
} // namespace

Mesh::~Mesh() {
    cleanup();
}

Mesh::Mesh(Mesh&& other) noexcept
    : vao(other.vao), vbo(other.vbo), ebo(other.ebo), indexCount(other.indexCount) {
    other.vao = 0;
    other.vbo = 0;
    other.ebo = 0;
    other.indexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        cleanup();
        vao = other.vao;
        vbo = other.vbo;
        ebo = other.ebo;
        indexCount = other.indexCount;
        other.vao = 0;
        other.vbo = 0;
        other.ebo = 0;
        other.indexCount = 0;
    }
    return *this;
}

void Mesh::cleanup() {
    if (ebo) glDeleteBuffers(1, &ebo);
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    vao = vbo = ebo = 0;
    indexCount = 0;
}

void Mesh::upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    cleanup();
    indexCount = static_cast<GLsizei>(indices.size());

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

Mesh Mesh::createCube() {
    std::vector<Vertex> v = {
        // Front (+Z)
        {-0.5f, -0.5f,  0.5f,  0,  0,  1, 0, 0},
        { 0.5f, -0.5f,  0.5f,  0,  0,  1, 1, 0},
        { 0.5f,  0.5f,  0.5f,  0,  0,  1, 1, 1},
        {-0.5f,  0.5f,  0.5f,  0,  0,  1, 0, 1},
        // Back (-Z)
        { 0.5f, -0.5f, -0.5f,  0,  0, -1, 0, 0},
        {-0.5f, -0.5f, -0.5f,  0,  0, -1, 1, 0},
        {-0.5f,  0.5f, -0.5f,  0,  0, -1, 1, 1},
        { 0.5f,  0.5f, -0.5f,  0,  0, -1, 0, 1},
        // Left (-X)
        {-0.5f, -0.5f, -0.5f, -1,  0,  0, 0, 0},
        {-0.5f, -0.5f,  0.5f, -1,  0,  0, 1, 0},
        {-0.5f,  0.5f,  0.5f, -1,  0,  0, 1, 1},
        {-0.5f,  0.5f, -0.5f, -1,  0,  0, 0, 1},
        // Right (+X)
        { 0.5f, -0.5f,  0.5f,  1,  0,  0, 0, 0},
        { 0.5f, -0.5f, -0.5f,  1,  0,  0, 1, 0},
        { 0.5f,  0.5f, -0.5f,  1,  0,  0, 1, 1},
        { 0.5f,  0.5f,  0.5f,  1,  0,  0, 0, 1},
        // Top (+Y)
        {-0.5f,  0.5f,  0.5f,  0,  1,  0, 0, 0},
        { 0.5f,  0.5f,  0.5f,  0,  1,  0, 1, 0},
        { 0.5f,  0.5f, -0.5f,  0,  1,  0, 1, 1},
        {-0.5f,  0.5f, -0.5f,  0,  1,  0, 0, 1},
        // Bottom (-Y)
        {-0.5f, -0.5f, -0.5f,  0, -1,  0, 0, 0},
        { 0.5f, -0.5f, -0.5f,  0, -1,  0, 1, 0},
        { 0.5f, -0.5f,  0.5f,  0, -1,  0, 1, 1},
        {-0.5f, -0.5f,  0.5f,  0, -1,  0, 0, 1},
    };

    std::vector<unsigned int> idx;
    idx.reserve(36);
    for (unsigned int face = 0; face < 6; ++face) {
        unsigned int base = face * 4;
        idx.push_back(base + 0); idx.push_back(base + 1); idx.push_back(base + 2);
        idx.push_back(base + 0); idx.push_back(base + 2); idx.push_back(base + 3);
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createQuad() {
    std::vector<Vertex> v = {
        {-0.5f, -0.5f, 0.0f, 0, 0, 1, 0.0f, 0.0f},
        { 0.5f, -0.5f, 0.0f, 0, 0, 1, 1.0f, 0.0f},
        { 0.5f,  0.5f, 0.0f, 0, 0, 1, 1.0f, 1.0f},
        {-0.5f,  0.5f, 0.0f, 0, 0, 1, 0.0f, 1.0f},
    };
    std::vector<unsigned int> idx = {0, 1, 2, 0, 2, 3};
    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createCylinder(int segments) {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;
    const float r = 0.5f;
    const float halfH = 0.5f;

    for (int i = 0; i <= segments; ++i) {
        float u = static_cast<float>(i) / segments;
        float theta = u * 2.0f * PI;
        float cx = std::cos(theta);
        float cz = std::sin(theta);

        v.push_back({r * cx, -halfH, r * cz, cx, 0.0f, cz, u, 0.0f});
        v.push_back({r * cx,  halfH, r * cz, cx, 0.0f, cz, u, 1.0f});
    }
    for (int i = 0; i < segments; ++i) {
        unsigned int b0 = i * 2;
        unsigned int t0 = b0 + 1;
        unsigned int b1 = b0 + 2;
        unsigned int t1 = b0 + 3;
        idx.push_back(b0); idx.push_back(t0); idx.push_back(t1);
        idx.push_back(b0); idx.push_back(t1); idx.push_back(b1);
    }

    unsigned int topCenter = static_cast<unsigned int>(v.size());
    v.push_back({0.0f, halfH, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f});
    unsigned int topRingStart = static_cast<unsigned int>(v.size());
    for (int i = 0; i <= segments; ++i) {
        float theta = (static_cast<float>(i) / segments) * 2.0f * PI;
        float cx = std::cos(theta);
        float cz = std::sin(theta);
        v.push_back({r * cx, halfH, r * cz, 0.0f, 1.0f, 0.0f, 0.5f + 0.5f * cx, 0.5f + 0.5f * cz});
    }
    for (int i = 0; i < segments; ++i) {
        idx.push_back(topCenter);
        idx.push_back(topRingStart + i + 1);
        idx.push_back(topRingStart + i);
    }

    unsigned int botCenter = static_cast<unsigned int>(v.size());
    v.push_back({0.0f, -halfH, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f});
    unsigned int botRingStart = static_cast<unsigned int>(v.size());
    for (int i = 0; i <= segments; ++i) {
        float theta = (static_cast<float>(i) / segments) * 2.0f * PI;
        float cx = std::cos(theta);
        float cz = std::sin(theta);
        v.push_back({r * cx, -halfH, r * cz, 0.0f, -1.0f, 0.0f, 0.5f + 0.5f * cx, 0.5f + 0.5f * cz});
    }
    for (int i = 0; i < segments; ++i) {
        idx.push_back(botCenter);
        idx.push_back(botRingStart + i);
        idx.push_back(botRingStart + i + 1);
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createSphere(int stacks, int slices) {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;
    const float r = 0.5f;

    for (int i = 0; i <= stacks; ++i) {
        float vCoord = static_cast<float>(i) / stacks;
        float phi = vCoord * PI;
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);

        for (int j = 0; j <= slices; ++j) {
            float uCoord = static_cast<float>(j) / slices;
            float theta = uCoord * 2.0f * PI;
            float nx = sinPhi * std::cos(theta);
            float ny = cosPhi;
            float nz = sinPhi * std::sin(theta);
            v.push_back({r * nx, r * ny, r * nz, nx, ny, nz, uCoord, 1.0f - vCoord});
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            unsigned int first = i * (slices + 1) + j;
            unsigned int second = first + slices + 1;
            idx.push_back(first);  idx.push_back(second);     idx.push_back(first + 1);
            idx.push_back(second); idx.push_back(second + 1); idx.push_back(first + 1);
        }
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createRoundedBox(float width, float height, float depth,
                            float cornerRadius, float bevelRadius,
                            int cornerSegs, int bevelSegs) {
    const float hy = height * 0.5f;
    const float rY = std::min({bevelRadius, hy * 0.49f, cornerRadius * 0.98f});
    std::vector<PerimPt> perim = buildRoundedRectPerimeter(width, depth, cornerRadius, cornerSegs);
    const int P = static_cast<int>(perim.size());

    struct RingSpec {
        float alpha;
        float yCenter;
    };
    std::vector<RingSpec> rings;
    for (int i = 0; i <= bevelSegs; ++i) {
        float alpha = (static_cast<float>(i) / bevelSegs) * (0.5f * PI);
        rings.push_back({alpha, hy - rY});
    }
    for (int i = 0; i <= bevelSegs; ++i) {
        float alpha = 0.5f * PI + (static_cast<float>(i) / bevelSegs) * (0.5f * PI);
        rings.push_back({alpha, -hy + rY});
    }
    const int R = static_cast<int>(rings.size());

    std::vector<Vertex> v;
    std::vector<unsigned int> idx;
    v.reserve(R * P + 2);

    for (int r = 0; r < R; ++r) {
        float alpha = rings[r].alpha;
        float sinA = std::sin(alpha);
        float cosA = std::cos(alpha);
        float inset = rY * (1.0f - sinA);
        float y = rings[r].yCenter + rY * cosA;

        for (int p = 0; p < P; ++p) {
            float x = perim[p].x - perim[p].nx * inset;
            float z = perim[p].z - perim[p].nz * inset;
            glm::vec3 n = glm::normalize(glm::vec3(perim[p].nx * sinA, cosA, perim[p].nz * sinA));
            float u = (x / width) + 0.5f;
            float vCoord = 0.5f - (z / depth);
            v.push_back({x, y, z, n.x, n.y, n.z, u, vCoord});
        }
    }

    for (int r = 0; r < R - 1; ++r) {
        for (int p = 0; p < P - 1; ++p) {
            unsigned int i00 = r * P + p;
            unsigned int i01 = i00 + 1;
            unsigned int i10 = (r + 1) * P + p;
            unsigned int i11 = i10 + 1;
            idx.push_back(i00); idx.push_back(i10); idx.push_back(i11);
            idx.push_back(i00); idx.push_back(i11); idx.push_back(i01);
        }
    }

    // Top flat cap
    unsigned int topCenter = static_cast<unsigned int>(v.size());
    v.push_back({0.0f, hy, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f});
    for (int p = 0; p < P - 1; ++p) {
        idx.push_back(topCenter);
        idx.push_back(p);
        idx.push_back(p + 1);
    }

    // Bottom flat cap
    unsigned int botCenter = static_cast<unsigned int>(v.size());
    v.push_back({0.0f, -hy, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f});
    unsigned int lastRingOffset = (R - 1) * P;
    for (int p = 0; p < P - 1; ++p) {
        idx.push_back(botCenter);
        idx.push_back(lastRingOffset + p + 1);
        idx.push_back(lastRingOffset + p);
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createRoundedRim(float width, float height, float depth,
                            float cornerRadius, float bevelRadius,
                            int cornerSegs, int bevelSegs) {
    const float hy = height * 0.5f;
    const float rY = std::min({bevelRadius, hy * 0.49f, cornerRadius * 0.98f});
    std::vector<PerimPt> perim = buildRoundedRectPerimeter(width, depth, cornerRadius, cornerSegs);
    const int P = static_cast<int>(perim.size());
    const float totalLen = std::max(perim.back().s, 0.001f);

    struct RingSpec {
        float alpha;
        float yCenter;
        float vCoord;
    };
    std::vector<RingSpec> rings;
    const float startAngle = 0.28f * PI;
    const float endAngle = 0.72f * PI;
    for (int i = 0; i <= bevelSegs; ++i) {
        float t = static_cast<float>(i) / bevelSegs;
        float alpha = startAngle + t * (0.5f * PI - startAngle);
        rings.push_back({alpha, hy - rY, t * 0.35f});
    }
    for (int i = 0; i <= bevelSegs; ++i) {
        float t = static_cast<float>(i) / bevelSegs;
        float alpha = 0.5f * PI + t * (endAngle - 0.5f * PI);
        rings.push_back({alpha, -hy + rY, 0.65f + t * 0.35f});
    }
    const int R = static_cast<int>(rings.size());

    std::vector<Vertex> v;
    std::vector<unsigned int> idx;
    v.reserve(R * P);

    for (int r = 0; r < R; ++r) {
        float alpha = rings[r].alpha;
        float sinA = std::sin(alpha);
        float cosA = std::cos(alpha);
        float inset = rY * (1.0f - sinA) - 0.0008f;
        float y = rings[r].yCenter + rY * cosA;

        for (int p = 0; p < P; ++p) {
            float x = perim[p].x - perim[p].nx * inset;
            float z = perim[p].z - perim[p].nz * inset;
            glm::vec3 n = glm::normalize(glm::vec3(perim[p].nx * sinA, cosA, perim[p].nz * sinA));
            float u = (perim[p].s / totalLen) * 8.0f;
            v.push_back({x, y, z, n.x, n.y, n.z, u, rings[r].vCoord});
        }
    }

    for (int r = 0; r < R - 1; ++r) {
        for (int p = 0; p < P - 1; ++p) {
            unsigned int i00 = r * P + p;
            unsigned int i01 = i00 + 1;
            unsigned int i10 = (r + 1) * P + p;
            unsigned int i11 = i10 + 1;
            idx.push_back(i00); idx.push_back(i10); idx.push_back(i11);
            idx.push_back(i00); idx.push_back(i11); idx.push_back(i01);
        }
    }

    Mesh m;
    m.upload(v, idx);
    return m;
}

Mesh Mesh::createCurvedPanel(int cols, int rows, float sCurveAmp, float cupDepth,
                              float thickness, float waistPinch) {
    std::vector<Vertex> v;
    std::vector<unsigned int> idx;

    auto evalPoint = [&](float u, float t, float zOffset) -> glm::vec3 {
        float xNorm = (u - 0.5f) * 2.0f;
        float yNorm = t - 0.5f;

        float cornerRound = 1.0f - 0.22f * std::pow(std::abs(yNorm * 2.0f), 4.0f);
        float waist = 1.0f - waistPinch * std::exp(-std::pow((t - 0.35f) / 0.22f, 2.0f));
        float x = (u - 0.5f) * cornerRound * waist;

        float sCurve = -sCurveAmp * std::sin((t - 0.32f) * PI * 1.45f);
        float cup = cupDepth * (xNorm * xNorm);

        return glm::vec3(x, yNorm, sCurve + cup + zOffset);
    };

    auto buildSurface = [&](float zOffset, bool flipNormal) {
        unsigned int baseIdx = static_cast<unsigned int>(v.size());
        for (int r = 0; r <= rows; ++r) {
            float t = static_cast<float>(r) / rows;
            for (int c = 0; c <= cols; ++c) {
                float u = static_cast<float>(c) / cols;
                glm::vec3 p = evalPoint(u, t, zOffset);

                float eps = 0.01f;
                glm::vec3 pu = evalPoint(std::min(u + eps, 1.0f), t, zOffset) - evalPoint(std::max(u - eps, 0.0f), t, zOffset);
                glm::vec3 pt = evalPoint(u, std::min(t + eps, 1.0f), zOffset) - evalPoint(u, std::max(t - eps, 0.0f), zOffset);
                glm::vec3 n = glm::normalize(glm::cross(pt, pu));
                if (flipNormal) n = -n;

                v.push_back({p.x, p.y, p.z, n.x, n.y, n.z, u, t});
            }
        }
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                unsigned int i0 = baseIdx + r * (cols + 1) + c;
                unsigned int i1 = i0 + 1;
                unsigned int i2 = i0 + (cols + 1);
                unsigned int i3 = i2 + 1;
                if (!flipNormal) {
                    idx.push_back(i0); idx.push_back(i2); idx.push_back(i1);
                    idx.push_back(i1); idx.push_back(i2); idx.push_back(i3);
                } else {
                    idx.push_back(i0); idx.push_back(i1); idx.push_back(i2);
                    idx.push_back(i1); idx.push_back(i3); idx.push_back(i2);
                }
            }
        }
    };

    buildSurface(-thickness * 0.5f, false);
    buildSurface(+thickness * 0.5f, true);

    auto addRimStrip = [&](int r0, int c0, int dr, int dc, int count, glm::vec3 approxNormal) {
        unsigned int base = static_cast<unsigned int>(v.size());
        for (int k = 0; k <= count; ++k) {
            float u = static_cast<float>(c0 + k * dc) / cols;
            float t = static_cast<float>(r0 + k * dr) / rows;
            glm::vec3 pf = evalPoint(u, t, -thickness * 0.5f);
            glm::vec3 pb = evalPoint(u, t, +thickness * 0.5f);
            v.push_back({pf.x, pf.y, pf.z, approxNormal.x, approxNormal.y, approxNormal.z, u, 0.0f});
            v.push_back({pb.x, pb.y, pb.z, approxNormal.x, approxNormal.y, approxNormal.z, u, 1.0f});
        }
        for (int k = 0; k < count; ++k) {
            unsigned int f0 = base + k * 2;
            unsigned int b0 = f0 + 1;
            unsigned int f1 = f0 + 2;
            unsigned int b1 = f0 + 3;
            idx.push_back(f0); idx.push_back(f1); idx.push_back(b1);
            idx.push_back(f0); idx.push_back(b1); idx.push_back(b0);
        }
    };

    addRimStrip(0, 0, 0, 1, cols, {0, -1, 0});
    addRimStrip(rows, cols, 0, -1, cols, {0, 1, 0});
    addRimStrip(rows, 0, -1, 0, rows, {-1, 0, 0});
    addRimStrip(0, cols, 1, 0, rows, {1, 0, 0});

    Mesh m;
    m.upload(v, idx);
    return m;
}
