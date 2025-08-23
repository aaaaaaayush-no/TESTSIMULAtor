#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "raylib.h"
#include <map>

// CONSTANTS AND CONFIGURATION
// ================================
const int SCREEN_WIDTH = 1800;
const int SCREEN_HEIGHT = 880;
const int SIDEBAR_WIDTH = 200;
const float CONNECTION_POINT_RADIUS = 6.0f;
const float CONNECTION_SNAP_DISTANCE = 15.0f;

// Grid system for visual aid and alignment
const int GRID_SIZE = 30;
const bool SHOW_GRID_DEFAULT = true;


// ENUMS AND DATA STRUCTURES
// ================================
enum class GateType {
    INPUT, OUTPUT, AND, OR, NOT, NAND, NOR
};

enum class SimulatorMode {
    PLACEMENT, WIRING
};

struct GateInfo {
    Vector2 size;
    Color color;
    const char* label;
    const char* imagePath;  // Path to image file
    Texture2D texture;      // Texture will be loaded at runtime
};

// Lookup table for gate properties - DECLARATION ONLY
// Note: INPUT and OUTPUT don't have images, all others use the provided paths
extern std::map<GateType, GateInfo> GATE_DATA;

// CONNECTION POINT HELPER
// ================================
struct ConnectionPoint {
    Vector2 position;
    bool isInput;
    int gateIndex;
    int inputIndex;  // For input points: 0 or 1, for output: always 0

    ConnectionPoint(Vector2 pos, bool input, int gate, int idx = 0)
        : position(pos), isInput(input), gateIndex(gate), inputIndex(idx) {
    }
};

// Function declarations for texture management
void LoadGateTextures();
void UnloadGateTextures();

#endif // CONSTANTS_H