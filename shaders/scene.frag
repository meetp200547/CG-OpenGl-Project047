#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

struct Material {
    vec3 baseColor;
    float shininess;
    float specularStrength;
    float emissiveStrength;
    int surfaceType; // 0 = default, 1 = wood, 2 = wall/plaster, 3 = carpet/fabric, 4 = floor
    bool useTexture;
};

struct TubeLight {
    vec3 startPos;
    vec3 endPos;
    vec3 color;
    float intensity;
    float ambientStrength;
    float constant;
    float linear;
    float quadratic;
};

struct ScreenLight {
    vec3 pos;
    vec3 forward;
    vec3 right;
    vec3 up;
    vec2 halfSize;
    vec3 color;
    float intensity;
    float linear;
    float quadratic;
};

uniform Material uMaterial;
uniform TubeLight uTubeLight;
uniform ScreenLight uMonitorLight;
uniform ScreenLight uLaptopLight;
uniform sampler2D uTexture;
uniform sampler2D uShadowMap;
uniform vec3 uViewPos;

// Fast procedural hash & 3D value noise for micro-surface detail
float hash3(vec3 p) {
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}

float noise3(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(mix(hash3(i + vec3(0,0,0)), hash3(i + vec3(1,0,0)), f.x),
            mix(hash3(i + vec3(0,1,0)), hash3(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash3(i + vec3(0,0,1)), hash3(i + vec3(1,0,1)), f.x),
            mix(hash3(i + vec3(0,1,1)), hash3(i + vec3(1,1,1)), f.x), f.y),
        f.z
    );
}

// Perturbs surface normal for realistic plaster walls, tufted carpet pile, and wood grain
vec3 applySurfaceMicroBump(vec3 norm, vec3 pos, vec2 uv, int surfaceType) {
    if (surfaceType == 2) {
        // Fine architectural plaster / roller-stipple micro-relief on walls
        float eps = 0.005;
        float h0 = noise3(pos * 42.0) * 0.6 + noise3(pos * 145.0) * 0.4;
        float hx = noise3((pos + vec3(eps, 0.0, 0.0)) * 42.0) * 0.6 + noise3((pos + vec3(eps, 0.0, 0.0)) * 145.0) * 0.4;
        float hy = noise3((pos + vec3(0.0, eps, 0.0)) * 42.0) * 0.6 + noise3((pos + vec3(0.0, eps, 0.0)) * 145.0) * 0.4;
        float hz = noise3((pos + vec3(0.0, 0.0, eps)) * 42.0) * 0.6 + noise3((pos + vec3(0.0, 0.0, eps)) * 145.0) * 0.4;
        vec3 grad = vec3(hx - h0, hy - h0, hz - h0) / eps;
        grad -= norm * dot(grad, norm);
        return normalize(norm - grad * 0.00048);
    } else if (surfaceType == 3) {
        // High-definition woven loop-pile carpet tuft relief
        float loopX = cos(uv.x * 260.0 * 6.28318);
        float loopZ = cos(uv.y * 190.0 * 6.28318);
        float n1 = noise3(pos * 165.0) - 0.5;
        float n2 = noise3(pos.zyx * 165.0) - 0.5;
        vec3 bump = vec3(loopX * 0.075 + n1 * 0.12, 0.0, loopZ * 0.075 + n2 * 0.12);
        bump -= norm * dot(bump, norm);
        return normalize(norm + bump);
    } else if (surfaceType == 1) {
        // Subtle wood-grain pore normal perturbation
        float g = sin(uv.y * 95.0 + noise3(pos * 14.0) * 5.0);
        vec3 bump = vec3(0.0, 0.0, g * 0.016);
        bump -= norm * dot(bump, norm);
        return normalize(norm + bump);
    }
    return norm;
}

// 16-tap anisotropic Poisson Disk PCF shadow mapping (wider horizontally to match the 1.3m linear tubelight)
float calcShadowMap(vec4 fragPosLightSpace, vec3 norm, vec3 lightDir) {
    if (fragPosLightSpace.w <= 0.0) return 0.0;
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    if (projCoords.x <= 0.01 || projCoords.x >= 0.99 ||
        projCoords.y <= 0.01 || projCoords.y >= 0.99 ||
        projCoords.z >= 1.0 || projCoords.z <= 0.0) {
        return 0.0;
    }

    // Smooth fade near ortho frustum borders
    vec2 edgeFade2 = smoothstep(vec2(0.01), vec2(0.08), projCoords.xy) *
                     (vec2(1.0) - smoothstep(vec2(0.92), vec2(0.99), projCoords.xy));
    float frustumWeight = edgeFade2.x * edgeFade2.y;

    float cosTheta = clamp(dot(norm, lightDir), 0.0, 1.0);
    float bias = max(0.0018 * (1.0 - cosTheta), 0.00055);

    const vec2 poissonDisk[16] = vec2[](
        vec2(-0.94201624, -0.39906216), vec2( 0.94558609, -0.76890725),
        vec2(-0.09418410, -0.92938870), vec2( 0.34495938,  0.29387760),
        vec2(-0.91588581,  0.45771432), vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543,  0.27676845), vec2( 0.97484398,  0.75648379),
        vec2( 0.44323325, -0.97511554), vec2( 0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023), vec2( 0.79197514,  0.19090188),
        vec2(-0.24188840,  0.99706507), vec2(-0.81409955,  0.91437590),
        vec2( 0.19984126,  0.78641367), vec2( 0.14383161, -0.14100790)
    );

    float angle = hash3(FragPos * 23.3) * 6.2831853;
    float s = sin(angle);
    float c = cos(angle);
    mat2 rot = mat2(c, -s, s, c);

    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
    // Soft anisotropic penumbra: wider in X (horizontal tube) than Y
    vec2 penumbraScale = vec2(9.5, 6.0);
    float shadow = 0.0;

    for (int i = 0; i < 16; ++i) {
        vec2 offset = (rot * poissonDisk[i]) * texelSize * penumbraScale;
        float pcfDepth = texture(uShadowMap, projCoords.xy + offset).r;
        shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
    }
    shadow /= 16.0;

    return shadow * frustumWeight;
}

// Helper for soft box distance in 2D XZ
float sdBox2D(vec2 p, vec2 b) {
    vec2 d = abs(p) - b;
    return length(max(d, vec2(0.0))) + min(max(d.x, d.y), 0.0);
}

// Analytical soft area-light shadows & contact ambient occlusion
void calcAnalyticalShadowsAndAO(vec3 pos, vec3 norm, out float directShadow, out float ambientOcclusion) {
    directShadow = 0.0;
    float ao = 1.0;

    // 1. Room architectural corner & crevice Ambient Occlusion
    float dFloor   = clamp((pos.y - (-0.78)) / 0.24, 0.0, 1.0);
    float dCeiling = clamp((1.82 - pos.y) / 0.28, 0.0, 1.0);
    float dFrontW  = clamp((pos.z - (-0.72)) / 0.24, 0.0, 1.0);
    float dBackW   = clamp((3.60 - pos.z) / 0.28, 0.0, 1.0);
    float dLeftW   = clamp((pos.x - (-2.10)) / 0.28, 0.0, 1.0);
    float dRightW  = clamp((2.10 - pos.x) / 0.28, 0.0, 1.0);

    float wallProx = (1.0 - dFrontW) + (1.0 - dBackW) + (1.0 - dLeftW) + (1.0 - dRightW);
    float vertProx = (1.0 - dFloor) + (1.0 - dCeiling);
    float cornerCrevice = clamp(wallProx * vertProx * 0.48 + max(wallProx - 1.0, 0.0) * 0.44, 0.0, 0.58);
    ao *= (1.0 - cornerCrevice);

    // 2. Floating shelf soft shadow directly beneath shelf on the front wall (y in [0.55, 0.94])
    if (pos.y < 0.94 && pos.y > 0.45 && pos.z < -0.45) {
        float xBox = sdBox2D(vec2(pos.x, 0.0), vec2(0.76, 0.1));
        float dy = 0.94 - pos.y;
        float shelfWallShadow = (1.0 - smoothstep(-0.04, 0.10, xBox)) *
                                exp(-dy * 4.5) *
                                (1.0 - smoothstep(-0.72, -0.46, pos.z));
        directShadow = max(directShadow, shelfWallShadow * 0.72);
        if (dy < 0.12 && xBox < 0.04) {
            ao *= mix(0.42, 1.0, dy / 0.12);
        }
    }

    // Contact shadows for items resting on top of the floating shelf (y ~= 0.958)
    if (abs(pos.y - 0.959) < 0.015 && pos.z < -0.44) {
        float dBook1 = sdBox2D(pos.xz - vec2(-0.62, -0.62), vec2(0.022, 0.090));
        float dPot1  = length(pos.xz - vec2(-0.40, -0.58)) - 0.038;
        float dPot2  = length(pos.xz - vec2(-0.22, -0.58)) - 0.048;
        float dHBook = sdBox2D(pos.xz - vec2( 0.28, -0.58), vec2(0.118, 0.082));
        float dVBook = sdBox2D(pos.xz - vec2( 0.42, -0.58), vec2(0.018, 0.078));
        float dVase  = length(pos.xz - vec2( 0.54, -0.58)) - 0.056;
        float dMinShelf = min(min(min(dBook1, dPot1), min(dPot2, dHBook)), min(dVBook, dVase));
        float shelfContact = 1.0 - smoothstep(-0.005, 0.032, dMinShelf);
        ao *= (1.0 - shelfContact * 0.65);
        directShadow = max(directShadow, shelfContact * 0.65);
    }

    // 3. Tabletop & Deskmat contact shadows (for upward-facing surfaces near y = 0.0 to 0.008)
    if (pos.y > -0.006 && pos.y < 0.009 && norm.y > 0.5) {
        // Monitor circular base at (0.0, -0.32)
        float dMonBase = length((pos.xz - vec2(0.0, -0.32)) * vec2(1.0, 1.08)) - 0.135;
        // Monitor screen soft cast shadow forward onto desk
        float dMonCast = sdBox2D(pos.xz - vec2(0.0, -0.20), vec2(0.42, 0.10));

        // Laptop angled base at (-0.78, -0.05), rotated +18 deg around Y
        vec2 lp = pos.xz - vec2(-0.78, -0.05);
        float c18 = 0.9510565, s18 = 0.3090170;
        vec2 lpRot = vec2(c18 * lp.x - s18 * lp.y, s18 * lp.x + c18 * lp.y);
        float dLaptop = sdBox2D(lpRot, vec2(0.222, 0.152));

        // Mechanical keyboard on deskmat at (0.02, 0.22)
        float dKeyb = sdBox2D(pos.xz - vec2(0.02, 0.22), vec2(0.184, 0.066));

        // Wireless mouse on deskmat at (0.38, 0.24)
        float dMouse = length((pos.xz - vec2(0.38, 0.24)) * vec2(1.45, 0.88)) - 0.046;

        // Water bottle at (0.94, -0.04)
        float dBottle = length(pos.xz - vec2(0.94, -0.04)) - 0.046;
        float dBottleCast = length((pos.xz - vec2(0.95, 0.03)) * vec2(0.95, 0.55)) - 0.048;

        float dContact = min(min(min(dMonBase, dLaptop), min(dKeyb, dMouse)), dBottle);
        float contactOcc = 1.0 - smoothstep(-0.005, 0.024, dContact);
        ao *= (1.0 - contactOcc * 0.78);
        directShadow = max(directShadow, contactOcc * 0.78);

        float castOcc = max(
            (1.0 - smoothstep(-0.02, 0.18, dMonCast)) * 0.22,
            (1.0 - smoothstep(-0.01, 0.065, dBottleCast)) * 0.45
        );
        directShadow = max(directShadow, castOcc);

        // Deskmat perimeter contact shadow on the wooden tabletop right around the mat
        if (pos.y < 0.002) {
            float dMat = sdBox2D(pos.xz - vec2(0.08, 0.23), vec2(0.54, 0.23));
            if (dMat > -0.005 && dMat < 0.020) {
                float matEdge = 1.0 - smoothstep(0.0, 0.018, abs(dMat));
                ao *= (1.0 - matEdge * 0.48);
            }
        }
    }

    // 4. Under-desk shadow on the front wall behind the desk
    if (pos.z < -0.55 && pos.y < 0.02 && pos.y > -0.78 && abs(pos.x) < 1.28) {
        float wallUnderDesk = (1.0 - smoothstep(1.15, 1.32, abs(pos.x))) *
                              smoothstep(0.04, -0.15, pos.y);
        directShadow = max(directShadow, wallUnderDesk * 0.68);
        ao *= (1.0 - wallUnderDesk * 0.35);
    }

    // 5. Floor & Carpet shadows: Under-desk shadow pool, 4 desk legs, chair seat & 5 casters
    if (pos.y < -0.755) {
        // Large soft shadow cast by the 2.50m x 1.16m desk tabletop
        float dUnderDesk = sdBox2D(pos.xz - vec2(0.0, 0.12), vec2(1.22, 0.56));
        float deskPool = 1.0 - smoothstep(-0.12, 0.38, dUnderDesk);
        directShadow = max(directShadow, deskPool * 0.72);
        ao *= (1.0 - deskPool * 0.32);

        // 4 Desk legs contact occlusion on carpet
        const vec2 legPts[4] = vec2[](
            vec2(-1.16, -0.48), vec2( 1.16, -0.48),
            vec2(-1.16,  0.48), vec2( 1.16,  0.48)
        );
        for (int i = 0; i < 4; ++i) {
            float dLeg = length(pos.xz - legPts[i]) - 0.034;
            float legOcc = 1.0 - smoothstep(-0.005, 0.045, dLeg);
            ao *= (1.0 - legOcc * 0.78);
            directShadow = max(directShadow, legOcc * 0.82);
        }

        // Ergonomic chair at (0.04, 0.84): soft shadow pool from seat/backrest + 5 caster wheel contact shadows
        float dChairPool = length((pos.xz - vec2(0.05, 0.96)) * vec2(1.0, 0.82)) - 0.28;
        float chairPool = 1.0 - smoothstep(-0.06, 0.28, dChairPool);
        directShadow = max(directShadow, chairPool * 0.70);
        ao *= (1.0 - chairPool * 0.35);

        // Central chair hub & 5 star legs/casters
        vec2 chairRoot = vec2(0.04, 0.84);
        float dHub = length(pos.xz - chairRoot) - 0.06;
        float hubOcc = 1.0 - smoothstep(0.0, 0.08, dHub);
        ao *= (1.0 - hubOcc * 0.55);

        for (int i = 0; i < 5; ++i) {
            float ang = float(i) * 1.25663706 - 0.1745329; // i*72 deg - 10 deg
            vec2 dir = vec2(cos(ang), -sin(ang));
            vec2 casterPos = chairRoot + dir * 0.295;
            float dCaster = length(pos.xz - casterPos) - 0.032;
            float casterOcc = 1.0 - smoothstep(-0.004, 0.040, dCaster);
            ao *= (1.0 - casterOcc * 0.80);
            directShadow = max(directShadow, casterOcc * 0.84);

            // Star leg spoke shadow
            vec2 pa = pos.xz - chairRoot;
            float h = clamp(dot(pa, dir) / 0.295, 0.0, 1.0);
            float dSpoke = length(pa - dir * (0.295 * h));
            float spokeOcc = (1.0 - smoothstep(0.015, 0.065, dSpoke)) * (1.0 - h * 0.3);
            directShadow = max(directShadow, spokeOcc * 0.50);
        }

        // Carpet edge contact shadow on the slate floor right outside the carpet border
        if (uMaterial.surfaceType == 4) {
            float dCarpet = sdBox2D(pos.xz - vec2(0.0, 0.45), vec2(1.45, 1.05));
            if (dCarpet > -0.01 && dCarpet < 0.035) {
                float rugEdge = 1.0 - smoothstep(0.0, 0.032, abs(dCarpet));
                ao *= (1.0 - rugEdge * 0.58);
            }
        }
    }

    ambientOcclusion = clamp(ao, 0.15, 1.0);
    directShadow = clamp(directShadow, 0.0, 0.86);
}

// Multi-sample linear area tubelight evaluation with shadows & surface-specific BRDF
vec3 calcTubeLight(TubeLight tube, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo,
                   float shadowFactor, float aoFactor)
{
    vec3 ab = tube.endPos - tube.startPos;

    // 5-point integration along the 1.3m linear LED tube for authentic wide area-light diffuse wrap
    vec3 diffuseAccum = vec3(0.0);
    const int SAMPLES = 5;
    for (int i = 0; i < SAMPLES; ++i) {
        float frac = (float(i) + 0.5) / float(SAMPLES);
        vec3 samplePt = tube.startPos + frac * ab;
        vec3 lVec = samplePt - fragPos;
        float dist = length(lVec);
        vec3 lDir = lVec / max(dist, 0.0001);
        float atten = 1.0 / (tube.constant + tube.linear * dist + tube.quadratic * (dist * dist));

        float nDotL = max(dot(normal, lDir), 0.0);
        // Translucent frosted tubelight diffuser wraps light softly onto the mounting wall behind it
        if (uMaterial.surfaceType == 2 && fragPos.z < -0.68) {
            float wallWash = exp(-dist * 0.85) * 0.52;
            nDotL = max(nDotL, wallWash);
        } else if (uMaterial.surfaceType == 3) {
            nDotL = max(dot(normal, lDir) * 0.85 + 0.15, 0.0);
        } else if (uMaterial.surfaceType == 2) {
            nDotL = pow(nDotL, 0.90);
        }
        diffuseAccum += (nDotL * atten) * tube.color;
    }
    diffuseAccum /= float(SAMPLES);

    // Closest point on the tube segment for specular & shadow alignment
    float tSpec = clamp(dot(fragPos - tube.startPos, ab) / dot(ab, ab), 0.0, 1.0);
    vec3 closestPt = tube.startPos + tSpec * ab;
    vec3 lightDir = normalize(closestPt - fragPos);
    float distance = length(closestPt - fragPos);
    float attenuation = 1.0 / (tube.constant + tube.linear * distance + tube.quadratic * (distance * distance));

    // Ambient room bounce from the tubelight modulated by Ambient Occlusion
    vec3 ambient = tube.color * albedo * tube.ambientStrength * tube.intensity * aoFactor;

    // Diffuse contribution
    vec3 diffuse = diffuseAccum * albedo * tube.intensity;

    // Specular (Blinn-Phong half-vector, modulated by wood grain / beveled edge / carpet sheen)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), uMaterial.shininess);
    float specMod = uMaterial.specularStrength;
    if (uMaterial.surfaceType == 1) {
        // Wood pores have lower specular, smooth grain bands & beveled edges have richer satin sheen
        float luma = dot(albedo, vec3(0.299, 0.587, 0.114));
        float bevelBoost = (abs(normal.y) < 0.985 && normal.y > 0.15) ? 1.55 : 1.0;
        specMod *= clamp(0.65 + luma * 1.15, 0.5, 1.45) * bevelBoost;
    }
    vec3 specular = tube.color * (spec * specMod) * tube.intensity * attenuation;

    // Cloth / carpet grazing-angle fiber sheen (Charlie/velvet-style retro-reflection)
    if (uMaterial.surfaceType == 3) {
        float nDotV = clamp(dot(normal, viewDir), 0.0, 1.0);
        float nDotL = clamp(dot(normal, lightDir), 0.0, 1.0);
        float fiberSheen = pow(1.0 - nDotV, 3.0) * nDotL * 0.15;
        specular += tube.color * albedo * fiberSheen * tube.intensity * attenuation;
    }

    float visibility = 1.0 - shadowFactor;
    return ambient + (diffuse + specular) * visibility;
}

// Evaluates subtle rectangular area light emitted by the monitor screen or laptop screen
vec3 calcScreenAreaLight(ScreenLight scr, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float aoFactor) {
    vec3 rel = fragPos - scr.pos;
    float frontDist = dot(rel, scr.forward);
    if (frontDist <= 0.005) {
        return vec3(0.0);
    }

    // Closest point on the rectangular screen panel to fragPos
    float uOff = clamp(dot(rel, scr.right), -scr.halfSize.x, scr.halfSize.x);
    float vOff = clamp(dot(rel, scr.up),    -scr.halfSize.y, scr.halfSize.y);
    vec3 closestScreenPt = scr.pos + scr.right * uOff + scr.up * vOff;

    vec3 toLight = closestScreenPt - fragPos;
    float dist = length(toLight);
    vec3 lDir = toLight / max(dist, 0.001);

    float emitCos = max(dot(-lDir, scr.forward), 0.0);
    emitCos = pow(emitCos, 1.15);

    float nDotL = max(dot(normal, lDir), 0.0);
    if (nDotL <= 0.0 || emitCos <= 0.0) {
        return vec3(0.0);
    }

    float atten = emitCos / (1.0 + scr.linear * dist + scr.quadratic * dist * dist);

    // Tabletop blocks screen light from reaching the floor beneath the desk
    if (fragPos.y < -0.03 && abs(fragPos.x) < 1.25 && abs(fragPos.z) < 0.58) {
        atten *= 0.08;
    }

    vec3 diffuse = scr.color * albedo * nDotL * scr.intensity * atten * aoFactor;

    vec3 halfwayDir = normalize(lDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), max(uMaterial.shininess * 0.75, 8.0));
    vec3 specular = scr.color * (spec * uMaterial.specularStrength * 1.25) * scr.intensity * atten * aoFactor;

    return diffuse + specular;
}

void main()
{
    vec3 geomNorm = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    vec3 texColor = uMaterial.useTexture ? texture(uTexture, TexCoord).rgb : vec3(1.0);
    vec3 albedo = uMaterial.baseColor * texColor;

    // Procedural micro-fiber texture detail on carpet so yarn loops are crisp even close-up
    if (uMaterial.surfaceType == 3) {
        float microTuft = (noise3(FragPos * 190.0) - 0.5) * 0.09 +
                          sin(FragPos.x * 540.0) * sin(FragPos.z * 540.0) * 0.025;
        albedo = clamp(albedo + vec3(microTuft), vec3(0.05), vec3(0.95));
    } else if (uMaterial.surfaceType == 2) {
        // Fine procedural plaster micro-grain on walls
        float plasterGrain = (noise3(FragPos * 95.0) - 0.5) * 0.035;
        albedo = clamp(albedo + vec3(plasterGrain), vec3(0.05), vec3(0.95));
    } else if (uMaterial.surfaceType == 1) {
        // Subtle warm bevel rim shading on rounded wooden table edges
        if (abs(geomNorm.y) < 0.98 && geomNorm.y > 0.0) {
            albedo *= mix(0.88, 1.06, geomNorm.y);
        }
    }

    // Emissive surfaces (tubelight tube, monitor video, laptop display, keyboard backlight plate)
    if (uMaterial.emissiveStrength > 0.0) {
        vec3 result = albedo * uMaterial.emissiveStrength;
        FragColor = vec4(result, 1.0);
        return;
    }

    // Apply fine surface micro-bump (plaster walls, tufted carpet pile, wood grain)
    vec3 norm = applySurfaceMicroBump(geomNorm, FragPos, TexCoord, uMaterial.surfaceType);

    // Compute shadow map + analytical soft area/contact shadows
    vec3 ab = uTubeLight.endPos - uTubeLight.startPos;
    float tClosest = clamp(dot(FragPos - uTubeLight.startPos, ab) / dot(ab, ab), 0.0, 1.0);
    vec3 tubePt = uTubeLight.startPos + tClosest * ab;
    vec3 mainLightDir = normalize(tubePt - FragPos);

    float mapShadow = calcShadowMap(FragPosLightSpace, geomNorm, mainLightDir);
    float analyticShadow = 0.0;
    float aoFactor = 1.0;
    calcAnalyticalShadowsAndAO(FragPos, geomNorm, analyticShadow, aoFactor);

    // Combine real-time depth shadow map with soft area/contact shadow
    // On the tabletop, elevated blockers (like the monitor screen) cast a softer, lower-opacity penumbra under the wide 1.3m tubelight
    float mapWeight = (FragPos.y > -0.05 && FragPos.y < 0.05) ? 0.25 : 0.68;
    float totalShadow = clamp(max(mapShadow * mapWeight, analyticShadow), 0.0, 0.85);

    // 1. Primary illumination from the linear LED tubelight
    vec3 result = calcTubeLight(uTubeLight, norm, FragPos, viewDir, albedo, totalShadow, aoFactor);

    // 2. Subtle screen glow from the center monitor & left laptop screen
    result += calcScreenAreaLight(uMonitorLight, norm, FragPos, viewDir, albedo, aoFactor);
    result += calcScreenAreaLight(uLaptopLight, norm, FragPos, viewDir, albedo, aoFactor);

    // Subtle Fresnel rim highlight for edge definition on beveled table edges & peripherals
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 3.6);
    result += uTubeLight.color * 0.038 * fresnel * uMaterial.specularStrength * aoFactor;

    // Filmic-Reinhard tone mapping preserving rich contrast, shadows, and deep blacks
    result = result / (result + vec3(0.58));
    result = pow(max(result, vec3(0.0)), vec3(1.0 / 1.06));
    FragColor = vec4(result, 1.0);
}
