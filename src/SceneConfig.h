#pragma once

#include <glm/glm.hpp>

// ============================================================================
// QUICK SCENE CONFIGURATION (Instant Object Toggles & Color Changes)
// ============================================================================
// Change any `true` to `false` to hide an object (and its shadow).
// Change any `glm::vec3(r, g, b)` (0.0 to 1.0) to recolor an object.
//
// BASIC COLOR CHEAT SHEET (Copy & Paste):
//   Red      : glm::vec3(0.85f, 0.15f, 0.15f)   |  White  : glm::vec3(0.95f, 0.95f, 0.95f)
//   Green    : glm::vec3(0.15f, 0.70f, 0.25f)   |  Black  : glm::vec3(0.06f, 0.06f, 0.07f)
//   Blue     : glm::vec3(0.15f, 0.40f, 0.85f)   |  Grey   : glm::vec3(0.50f, 0.50f, 0.52f)
//   Yellow   : glm::vec3(0.95f, 0.85f, 0.15f)   |  Brown  : glm::vec3(0.45f, 0.26f, 0.14f)
//   Orange   : glm::vec3(0.95f, 0.45f, 0.10f)   |  Beige  : glm::vec3(0.88f, 0.82f, 0.70f)
//   Purple   : glm::vec3(0.55f, 0.20f, 0.80f)   |  Gold   : glm::vec3(0.85f, 0.68f, 0.25f)
//   Cyan     : glm::vec3(0.15f, 0.80f, 0.85f)   |  Silver : glm::vec3(0.78f, 0.80f, 0.83f)
//   Pink     : glm::vec3(0.92f, 0.45f, 0.65f)   |  Navy   : glm::vec3(0.10f, 0.18f, 0.38f)
// ============================================================================
namespace SceneConfig {

// --- 1. OBJECT VISIBILITY TOGGLES ---
inline bool showRoom           = true;
inline bool showDoor           = true;
inline bool showCeilingFan     = true;
inline bool showTubeLight      = true;
inline bool showShelfAndPlants = true;
inline bool showDesk           = true;
inline bool showDeskMat        = true;
inline bool showMonitor        = true;
inline bool showLaptop         = true;
inline bool showKeyboard       = true;
inline bool showMouse          = true;
inline bool showWaterBottle    = true;
inline bool showChair          = true;
inline bool showExtraBottle    = false; // Toggle live with 'O' key

// --- 2. ROOM, WALLS, FLOOR & CARPET COLORS ---
inline glm::vec3 wallTint       = glm::vec3(1.00f, 1.00f, 1.00f); // Neutral grey plaster
inline glm::vec3 ceilingTint    = glm::vec3(0.96f, 0.96f, 0.96f);
inline glm::vec3 floorTint      = glm::vec3(1.00f, 1.00f, 1.00f); // Dark slate tiles
inline glm::vec3 carpetTint     = glm::vec3(1.00f, 1.00f, 1.00f); // Heather-grey carpet
inline glm::vec3 baseboardColor = glm::vec3(0.22f, 0.23f, 0.25f);

// --- 3. DOOR & CEILING FAN COLORS ---
inline glm::vec3 doorWoodColor  = glm::vec3(0.68f, 0.52f, 0.38f);
inline glm::vec3 doorFrameColor = glm::vec3(0.45f, 0.32f, 0.22f);
inline glm::vec3 doorKnobColor  = glm::vec3(0.82f, 0.78f, 0.68f);

inline glm::vec3 fanBodyColor   = glm::vec3(0.14f, 0.14f, 0.16f);
inline glm::vec3 fanBladeColor  = glm::vec3(0.10f, 0.10f, 0.11f);
inline glm::vec3 fanTrimColor   = glm::vec3(0.65f, 0.67f, 0.70f);

// --- 4. DESK & DESKMAT COLORS ---
inline glm::vec3 deskWoodTint      = glm::vec3(1.00f, 1.00f, 1.00f); // Golden oak
inline glm::vec3 deskEdgeTrimColor = glm::vec3(0.12f, 0.12f, 0.14f); // Black bullnose trim
inline glm::vec3 deskSteelColor    = glm::vec3(0.08f, 0.08f, 0.09f); // Legs & apron
inline glm::vec3 deskMatTint       = glm::vec3(0.95f, 0.95f, 0.95f);

// --- 5. MONITOR, LAPTOP, KEYBOARD & MOUSE COLORS ---
inline glm::vec3 monitorChassisColor = glm::vec3(0.07f, 0.07f, 0.08f);
inline glm::vec3 monitorStandColor   = glm::vec3(0.09f, 0.09f, 0.10f);

inline glm::vec3 laptopChassisColor  = glm::vec3(0.08f, 0.08f, 0.09f);
inline glm::vec3 laptopKeyColor      = glm::vec3(0.14f, 0.06f, 0.06f);

inline glm::vec3 keyboardCaseColor   = glm::vec3(0.06f, 0.06f, 0.07f);
inline glm::vec3 keyboardKeyColor    = glm::vec3(0.10f, 0.10f, 0.11f);
inline glm::vec3 keyboardGlowColor   = glm::vec3(0.28f, 0.34f, 0.38f);

inline glm::vec3 mouseBodyColor      = glm::vec3(0.08f, 0.08f, 0.09f);
inline glm::vec3 mouseWheelColor     = glm::vec3(0.16f, 0.16f, 0.18f);

// --- 6. WATER BOTTLE & ERGONOMIC CHAIR COLORS ---
inline glm::vec3 bottleTint       = glm::vec3(1.00f, 1.00f, 1.00f); // Brushed stainless steel
inline glm::vec3 bottleCapColor   = glm::vec3(0.78f, 0.80f, 0.82f);

inline glm::vec3 chairFrameColor  = glm::vec3(0.07f, 0.07f, 0.08f);
inline glm::vec3 chairSeatColor   = glm::vec3(0.06f, 0.06f, 0.07f);
inline glm::vec3 chairMeshColor   = glm::vec3(0.75f, 0.77f, 0.80f);

// --- 7. SHELF & PLANTS COLORS ---
inline glm::vec3 shelfWoodTint    = glm::vec3(0.96f, 0.96f, 0.96f);
inline glm::vec3 plantLeafColor   = glm::vec3(0.13f, 0.34f, 0.13f);
inline glm::vec3 plantLeafLight   = glm::vec3(0.19f, 0.42f, 0.16f);
inline glm::vec3 flowerColorA     = glm::vec3(0.86f, 0.56f, 0.68f);
inline glm::vec3 flowerColorB     = glm::vec3(0.92f, 0.72f, 0.80f);
inline glm::vec3 vaseColor        = glm::vec3(0.92f, 0.90f, 0.86f);
inline glm::vec3 potColor         = glm::vec3(0.94f, 0.93f, 0.90f);

// --- 8. LIGHTING SETTINGS ---
inline glm::vec3 tubeLightColor       = glm::vec3(1.00f, 0.985f, 0.96f);
inline float     tubeLightIntensity   = 1.72f;
inline float     monitorLightIntensity = 0.36f;
inline float     laptopLightIntensity  = 0.25f;

} // namespace SceneConfig
