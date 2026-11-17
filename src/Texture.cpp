#include "Texture.h"
#include <algorithm>
#include <cmath>

namespace {

unsigned char clampByte(float v) {
    return static_cast<unsigned char>(std::clamp(v * 255.0f, 0.0f, 255.0f));
}

float hash2D(int x, int y) {
    int n = x * 374761393 + y * 668265263;
    n = (n ^ (n >> 13)) * 1274126177;
    return 1.0f - ((n ^ (n >> 16)) & 0x7fffffff) / 1073741824.0f;
}

float smoothNoise(float x, float y) {
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);

    float a = hash2D(ix, iy);
    float b = hash2D(ix + 1, iy);
    float c = hash2D(ix, iy + 1);
    float d = hash2D(ix + 1, iy + 1);

    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy;
}

int wrapMod(int v, int period) {
    int m = v % period;
    return (m < 0) ? (m + period) : m;
}

// Seamless tileable smooth noise with integer periods (px, py)
float smoothNoisePeriodic(float u, float v, int px, int py) {
    float x = u * px;
    float y = v * py;
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    float fx = x - ix;
    float fy = y - iy;
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);

    int x0 = wrapMod(ix, px);
    int x1 = wrapMod(ix + 1, px);
    int y0 = wrapMod(iy, py);
    int y1 = wrapMod(iy + 1, py);

    float a = hash2D(x0, y0);
    float b = hash2D(x1, y0);
    float c = hash2D(x0, y1);
    float d = hash2D(x1, y1);

    return (a * (1 - fx) + b * fx) * (1 - fy) + (c * (1 - fx) + d * fx) * fy;
}

} // namespace

namespace Texture {

GLuint createFromRGB(int width, int height, const std::vector<unsigned char>& rgb, bool repeat) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb.data());
    glGenerateMipmap(GL_TEXTURE_2D);

#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
    GLfloat maxAniso = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
    if (maxAniso > 1.0f) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(maxAniso, 8.0f));
    }
#endif

    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

// High-res seamless golden-oak / honey-brown wood grain with pores & medullary rays
GLuint createWoodTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;

            float warp1 = smoothNoisePeriodic(u, v, 6, 24) * 0.25f;
            float warp2 = smoothNoisePeriodic(u, v, 14, 48) * 0.08f;
            float ringPhase = (v * 28.0f + warp1 * 11.0f + warp2 * 7.0f) * 3.14159265f;
            float grain = 0.5f + 0.5f * std::sin(ringPhase);
            grain = std::pow(grain, 0.78f);

            float streak = smoothNoisePeriodic(u, v, 16, 120) * 0.12f;
            float pores = smoothNoisePeriodic(u, v, 48, 320);
            float poreDip = (pores < -0.52f) ? -0.09f : 0.0f;
            float fineFiber = smoothNoisePeriodic(u, v, 96, 256) * 0.045f;

            // Subtle plank variation across v
            int plankIdx = static_cast<int>(v * 5.0f);
            float plankTint = hash2D(plankIdx, 73) * 0.035f;

            float t = std::clamp(grain * 0.54f + streak + fineFiber + poreDip + plankTint + 0.24f, 0.0f, 1.0f);

            // Warm golden-oak / honey-walnut tones
            float r = 0.33f + t * 0.35f;
            float g = 0.17f + t * 0.21f;
            float b = 0.055f + t * 0.085f;

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(r);
            rgb[idx + 1] = clampByte(g);
            rgb[idx + 2] = clampByte(b);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

// Black graphic deskmat with cool steel-blue/silver sci-fi accents & stitched border
GLuint createDeskMatTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;
            float v = static_cast<float>(y) / size;

            int weave = ((x + y) % 2 == 0) ? 1 : 0;
            float noise = hash2D(x, y) * 0.015f;
            float base = (weave ? 0.085f : 0.070f) + noise;

            float r = base;
            float g = base * 1.04f;
            float b = base * 1.12f;

            float dx = (u - 0.54f);
            float dy = (v - 0.52f) * 1.8f;
            float dist = std::sqrt(dx * dx + dy * dy);
            float art = std::exp(-dist * 4.5f) * (0.5f + 0.5f * std::sin(u * 26.0f + v * 14.0f));
            float techLine = std::exp(-std::abs(v - 0.48f - 0.05f * std::sin(u * 12.0f)) * 45.0f);

            r += art * 0.14f + techLine * 0.10f;
            g += art * 0.20f + techLine * 0.18f;
            b += art * 0.28f + techLine * 0.26f;

            // Fine braided overlock stitched border
            float edgeDist = std::min({u, 1.0f - u, (v) * 0.55f, (1.0f - v) * 0.55f});
            if (edgeDist < 0.016f) {
                float stitch = 0.5f + 0.5f * std::sin((u + v) * 420.0f);
                r = 0.10f + 0.05f * stitch;
                g = 0.11f + 0.05f * stitch;
                b = 0.13f + 0.06f * stitch;
            }

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(r);
            rgb[idx + 1] = clampByte(g);
            rgb[idx + 2] = clampByte(b);
        }
    }
    return createFromRGB(size, size, rgb, false);
}

GLuint createMatteBlackTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;
            float micro = hash2D(x, y) * 0.025f;
            float brushed = smoothNoisePeriodic(u, v, 32, 8) * 0.035f;
            float val = std::clamp(0.88f + micro + brushed, 0.78f, 1.0f);

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(val);
            rgb[idx + 1] = clampByte(val);
            rgb[idx + 2] = clampByte(val);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

GLuint createBottleTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;
            float brush = smoothNoisePeriodic(u, v, 128, 12) * 0.055f;
            float sheen = 0.70f + 0.08f * std::sin(u * 6.28318f * 2.0f) + brush;

            if (v > 0.88f && v < 0.90f) {
                sheen *= 0.65f;
            }

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(sheen * 0.98f);
            rgb[idx + 1] = clampByte(sheen * 0.99f);
            rgb[idx + 2] = clampByte(sheen * 1.02f);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

GLuint createChairMeshTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float rib = ((y / 3) % 2 == 0) ? 0.56f : 0.34f;
            float weave = ((x / 2) % 2 == 0) ? 0.03f : -0.03f;
            float val = std::clamp(rib + weave, 0.22f, 0.72f);

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(val);
            rgb[idx + 1] = clampByte(val * 1.01f);
            rgb[idx + 2] = clampByte(val * 1.04f);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

// Seamless architectural matte plaster / eggshell painted wall texture (zero tiling seams)
GLuint createWallTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;

            // Multi-scale seamless periodic plaster variation + fine roller stipple
            float macro = smoothNoisePeriodic(u, v, 8, 8) * 0.014f;
            float meso  = smoothNoisePeriodic(u, v, 28, 28) * 0.016f;
            float micro = smoothNoisePeriodic(u, v, 112, 112) * 0.018f;
            float stipple = hash2D(x, y) * 0.014f;

            float val = std::clamp(0.54f + macro + meso + micro + stipple, 0.46f, 0.62f);

            // Subtle warm-neutral architectural grey paint tint
            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(val * 0.99f);
            rgb[idx + 1] = clampByte(val * 1.00f);
            rgb[idx + 2] = clampByte(val * 1.02f);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

// Large-format dark architectural slate tile floor with fine stone grain & dark grout lines
GLuint createFloorTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;

            float stone = smoothNoisePeriodic(u, v, 10, 10) * 0.022f
                        + smoothNoisePeriodic(u, v, 36, 36) * 0.015f
                        + hash2D(x, y) * 0.008f;
            float val = std::clamp(0.19f + stone, 0.13f, 0.25f);

            // Subtle recessed tile grout joints (2x2 tiles per repeat)
            float tileU = std::fmod(u * 2.0f, 1.0f);
            float tileV = std::fmod(v * 2.0f, 1.0f);
            if (tileU < 0.010f || tileU > 0.990f || tileV < 0.010f || tileV > 0.990f) {
                val *= 0.58f;
            }

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(val * 0.98f);
            rgb[idx + 1] = clampByte(val * 0.99f);
            rgb[idx + 2] = clampByte(val * 1.03f);
        }
    }
    return createFromRGB(size, size, rgb, true);
}

// High-res realistic tufted heather-grey woven loop-pile carpet with serged overlock border
GLuint createCarpetTexture(int size) {
    std::vector<unsigned char> rgb(static_cast<size_t>(size) * size * 3);
    for (int y = 0; y < size; ++y) {
        float v = static_cast<float>(y) / size;
        for (int x = 0; x < size; ++x) {
            float u = static_cast<float>(x) / size;

            // Fine woven yarn loop rows (~220 loops across the carpet)
            float loopX = std::sin(u * 220.0f * 6.28318f);
            float loopY = std::sin((v * 160.0f + 0.5f * ((x / 2) % 2)) * 6.28318f);
            float weaveLoop = (loopX * 0.5f + loopY * 0.5f) * 0.028f;

            // Multi-tonal heather yarn fibers (charcoal, mid-grey, and light silver flecks)
            float heather1 = hash2D(x, y) * 0.065f;
            float heather2 = smoothNoisePeriodic(u, v, 180, 140) * 0.048f;

            // Subtle natural pile nap / vacuum brush shading
            float nap = smoothNoisePeriodic(u, v, 12, 10) * 0.028f
                      + smoothNoisePeriodic(u, v, 36, 28) * 0.018f;

            float val = std::clamp(0.41f + weaveLoop + heather1 + heather2 + nap, 0.24f, 0.58f);

            // Serged / overlock woven edge binding around the perimeter
            float borderDist = std::min({u * 1.38f, (1.0f - u) * 1.38f, v, 1.0f - v});
            if (borderDist < 0.026f) {
                // Inner recessed groove between pile and serged binding
                if (borderDist > 0.020f) {
                    val *= 0.68f;
                } else {
                    // Raised serged yarn binding with perpendicular thread ribs
                    float stitchCoord = ( std::min(u, 1.0f - u) * 1.38f < std::min(v, 1.0f - v) ) ? v : u * 1.38f;
                    float sergeRib = 0.5f + 0.5f * std::sin(stitchCoord * 520.0f);
                    float profile = std::sin((borderDist / 0.020f) * 3.14159f);
                    val = std::clamp(0.28f + 0.08f * profile + 0.045f * sergeRib + heather1 * 0.3f, 0.18f, 0.44f);
                }
            }

            size_t idx = (static_cast<size_t>(y) * size + x) * 3;
            rgb[idx + 0] = clampByte(val * 0.99f);
            rgb[idx + 1] = clampByte(val * 1.00f);
            rgb[idx + 2] = clampByte(val * 1.03f);
        }
    }
    return createFromRGB(size, size, rgb, false);
}

// Dark IDE screen matching the left laptop
GLuint createLaptopScreenTexture(int width, int height) {
    std::vector<unsigned char> rgb(static_cast<size_t>(width) * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / width;
            float v = static_cast<float>(y) / height;

            float r = 0.08f, g = 0.09f, b = 0.13f;

            if (u < 0.16f) {
                r = 0.06f; g = 0.07f; b = 0.10f;
            }
            if (u > 0.72f) {
                r = 0.09f; g = 0.10f; b = 0.14f;
            }
            if (v > 0.93f || v < 0.06f) {
                r = 0.05f; g = 0.06f; b = 0.08f;
            }

            if (((u > 0.20f && u < 0.68f) || (u > 0.75f && u < 0.96f)) && v > 0.12f && v < 0.88f) {
                int line = static_cast<int>((0.88f - v) * 26.0f);
                float h = hash2D(line, 19) * 0.5f + 0.5f;
                float localY = std::fmod((0.88f - v) * 26.0f, 1.0f);
                if (localY > 0.36f && localY < 0.66f && h > 0.25f) {
                    if (u < 0.68f && u < 0.22f + h * 0.42f) {
                        r = 0.45f; g = 0.68f; b = 0.92f;
                    } else if (u > 0.75f && u < 0.78f + h * 0.17f) {
                        r = 0.65f; g = 0.70f; b = 0.78f;
                    }
                }
            }

            size_t idx = (static_cast<size_t>(y) * width + x) * 3;
            rgb[idx + 0] = clampByte(r);
            rgb[idx + 1] = clampByte(g);
            rgb[idx + 2] = clampByte(b);
        }
    }
    return createFromRGB(width, height, rgb, false);
}

GLuint createVideoTexture(int width, int height) {
    std::vector<unsigned char> empty(static_cast<size_t>(width) * height * 3, 0);
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, empty.data());
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

glm::vec3 updateVideoTexture(GLuint texId, int width, int height, float timeSec,
                             bool paused, std::vector<unsigned char>& pixelBuffer) {
    pixelBuffer.resize(static_cast<size_t>(width) * height * 3);

    glm::vec3 avgColor(0.0f);
    const float progress = std::fmod(timeSec * 0.08f, 1.0f);

    for (int y = 0; y < height; ++y) {
        float v = static_cast<float>(y) / height;
        for (int x = 0; x < width; ++x) {
            float u = static_cast<float>(x) / width;

            float skyMix = std::clamp((v - 0.35f) / 0.65f, 0.0f, 1.0f);
            float wave = 0.5f + 0.5f * std::sin(u * 6.0f + timeSec * 1.4f + v * 3.0f);

            float r = 0.10f + 0.38f * (1.0f - skyMix) + 0.14f * wave;
            float g = 0.08f + 0.16f * skyMix + 0.16f * std::sin(timeSec * 0.7f + u * 4.0f) * 0.5f;
            float b = 0.24f + 0.46f * skyMix + 0.15f * wave;

            float sunX = 0.5f + 0.22f * std::sin(timeSec * 0.55f);
            float sunY = 0.62f + 0.08f * std::cos(timeSec * 0.8f);
            float dx = (u - sunX) * (static_cast<float>(width) / height);
            float dy = v - sunY;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < 0.17f) {
                float glow = 1.0f - (dist / 0.17f);
                r = std::max(r, 0.96f);
                g = std::max(g, 0.42f + 0.48f * glow);
                b = std::max(b, 0.18f + 0.25f * glow);
            }

            float hill1 = 0.34f + 0.06f * std::sin(u * 7.5f + timeSec * 1.1f);
            float hill2 = 0.24f + 0.05f * std::cos(u * 11.0f - timeSec * 1.7f);
            if (v < hill1) {
                r *= 0.35f; g *= 0.35f; b = b * 0.45f + 0.08f;
            }
            if (v < hill2) {
                r = 0.05f; g = 0.06f; b = 0.11f;
            }

            if (v < 0.07f) {
                r *= 0.22f; g *= 0.22f; b *= 0.22f;
                if (v > 0.025f && v < 0.045f && u > 0.06f && u < 0.94f) {
                    float barU = (u - 0.06f) / 0.88f;
                    if (barU <= progress) {
                        r = 0.95f; g = 0.20f; b = 0.22f;
                    } else {
                        r = 0.35f; g = 0.35f; b = 0.38f;
                    }
                }
            }

            if (paused && std::abs(v - 0.5f) < 0.10f) {
                if ((u > 0.44f && u < 0.47f) || (u > 0.53f && u < 0.56f)) {
                    r = 0.95f; g = 0.95f; b = 0.95f;
                }
            }

            size_t idx = (static_cast<size_t>(y) * width + x) * 3;
            pixelBuffer[idx + 0] = clampByte(r);
            pixelBuffer[idx + 1] = clampByte(g);
            pixelBuffer[idx + 2] = clampByte(b);

            avgColor += glm::vec3(r, g, b);
        }
    }

    avgColor /= static_cast<float>(width * height);

    glBindTexture(GL_TEXTURE_2D, texId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixelBuffer.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    return avgColor;
}

} // namespace Texture
