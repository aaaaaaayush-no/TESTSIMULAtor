#include "Gate.h"
#include <iostream>
#include <string> // Add this for string conversions

// Stream operator for GateType enum
std::ostream& operator<<(std::ostream& os, const GateType& type) {
    switch (type) {
    case GateType::INPUT:  return os << "INPUT";
    case GateType::OUTPUT: return os << "OUTPUT";
    case GateType::AND:    return os << "AND";
    case GateType::OR:     return os << "OR";
    case GateType::NOT:    return os << "NOT";
    case GateType::NAND:   return os << "NAND";
    case GateType::NOR:    return os << "NOR";
    default:               return os << "UNKNOWN";
    }
}

// ================================
// GATE CLASS IMPLEMENTATION
// ================================

// Constructor
Gate::Gate(GateType t, Vector2 pos) : type(t), position(pos) {
    info = GATE_DATA.at(t);
}

// Getters
GateType Gate::GetType() const {
    return type;
}

Vector2 Gate::GetSize() const {
    return info.size;
}

Color Gate::GetColor() const {
    return info.color;
}

const char* Gate::GetLabel() const {
    return info.label;
}

// Logic computation
void Gate::ComputeOutput() {
    switch (type) {
    case GateType::INPUT:
    case GateType::OUTPUT:
        output = input1;
        break;
    case GateType::AND:
        output = input1 && input2;
        break;
    case GateType::OR:
        output = input1 || input2;
        break;
    case GateType::NOT:
        output = !input1;
        break;
    case GateType::NAND:
        output = !(input1 && input2);
        break;
    case GateType::NOR:
        output = !(input1 || input2);
        break;
    }
}

// Collision and boundary methods
Rectangle Gate::GetBounds() const {
    return { position.x, position.y, info.size.x, info.size.y };
}

bool Gate::ContainsPoint(Vector2 point) const {
    return CheckCollisionPointRec(point, GetBounds());
}

bool Gate::CollidesWith(const Gate& other) const {
    return CheckCollisionRecs(GetBounds(), other.GetBounds());
}

// Connection point helpers
Vector2 Gate::GetInputPoint(int inputIndex) const {
    // For INPUT and OUTPUT gates, center the connection point
    if (type == GateType::INPUT || type == GateType::OUTPUT) {
        return { position.x - 8, position.y + info.size.y * 0.5f };
    }

    // For textured gates, place connection points further from gate edge
    if (info.texture.id != 0) {
        // Special case for NOT gate - center the single input pin
        if (type == GateType::NOT) {
            return { position.x - 12, position.y + info.size.y * 0.5f };
        }
        
        // For other gates with multiple inputs, use offset positions
        float offsetY = (inputIndex == 0) ? info.size.y * 0.33f : info.size.y * 0.66f;
        return { position.x - 12, position.y + offsetY }; // Moved 12 pixels away instead of at edge
    } else {
        // For non-textured gates, use original offset logic
        float offsetY = (inputIndex == 0) ? info.size.y * 0.33f : info.size.y * 0.66f;
        return { position.x - 8, position.y + offsetY };
    }
}

Vector2 Gate::GetOutputPoint() const {
    // Adjust output connection point to be further from gate edge
    if (info.texture.id != 0 && type != GateType::INPUT && type != GateType::OUTPUT) {
        // For textured gates, place connection point further from the actual gate output edge
        return { position.x + info.size.x + 4, position.y + info.size.y * 0.5f };
    } else {
        // For non-textured gates, keep original offset
        return { position.x + info.size.x + 8, position.y + info.size.y * 0.5f };
    }
}

int Gate::GetInputCount() const {
    switch (type) {
    case GateType::INPUT:
        return 0;  // No inputs
    case GateType::OUTPUT:
    case GateType::NOT:
        return 1;  // Single input
    case GateType::AND:
    case GateType::OR:
    case GateType::NAND:
    case GateType::NOR:
        return 2;  // Two inputs
    default:
        return 0;
    }
}

bool Gate::HasOutput() const {
    return type != GateType::OUTPUT;  // All gates except OUTPUT have outputs
}

// Get all connection points for this gate
std::vector<ConnectionPoint> Gate::GetConnectionPoints(int gateIndex) const {
    std::vector<ConnectionPoint> points;

    // Add input points
    int inputCount = GetInputCount();
    for (int i = 0; i < inputCount; i++) {
        points.emplace_back(GetInputPoint(i), true, gateIndex, i);
    }

    // Add output point
    if (HasOutput()) {
        points.emplace_back(GetOutputPoint(), false, gateIndex, 0);
    }

    return points;
}

// Check if an input is connected
bool Gate::IsInputConnected(int inputIndex, const std::vector<std::unique_ptr<Wire>>& wires) const {
    for (const auto& wire : wires) {
        if (wire->toInputIndex == inputIndex) {
            return true;
        }
    }
    return false;
}

// Private method for drawing connection points
void Gate::DrawConnectionPoints() const {
    // Draw input points for all gates that have them
    int inputCount = GetInputCount();
    for (int i = 0; i < inputCount; i++) {
        Vector2 inputPos = GetInputPoint(i);
        bool inputState = (i == 0) ? input1 : input2;

        // Draw clean, smooth connection points
        Color innerColor = inputState ? RED : DARKGRAY;
        Color outerColor = WHITE;
        
        // Draw layered circles for smooth appearance
        DrawCircleV(inputPos, CONNECTION_POINT_RADIUS, outerColor);
        DrawCircleV(inputPos, CONNECTION_POINT_RADIUS - 1, innerColor);
        
        // Draw a clean border
        DrawCircleLinesV(inputPos, CONNECTION_POINT_RADIUS, BLACK);
    }

    // Draw output points for NAND, NOR, NOT, and INPUT gates
    if (HasOutput() && (type == GateType::NAND || type == GateType::NOR || type == GateType::NOT || type == GateType::INPUT)) {
        Vector2 outputPos = GetOutputPoint();

        // Draw clean, smooth output point
        Color innerColor = output ? RED : DARKGRAY;
        Color outerColor = WHITE;
        
        // Draw layered circles for smooth appearance
        DrawCircleV(outputPos, CONNECTION_POINT_RADIUS, outerColor);
        DrawCircleV(outputPos, CONNECTION_POINT_RADIUS - 1, innerColor);
        
        // Draw a clean border
        DrawCircleLinesV(outputPos, CONNECTION_POINT_RADIUS, BLACK);
    }
}

// Rendering
void Gate::Draw(bool preview, bool highlight) const {
    Rectangle body = GetBounds();
    Color drawColor = info.color;

    if (preview) drawColor.a = 128; // Semi-transparent for preview
    if (highlight) drawColor = ColorBrightness(drawColor, 0.3f); // Brighter when highlighted

    // Check if texture is valid and loaded
    if (info.texture.id != 0 && type != GateType::INPUT && type != GateType::OUTPUT) {
        Color tint = WHITE;
        if (preview) tint.a = 128; // Make transparent if preview
        
        // Draw the texture without color tinting
        DrawTexturePro(
            info.texture,
            {0, 0, (float)info.texture.width, (float)info.texture.height}, // source rectangle
            body, // destination rectangle
            {0, 0}, // origin
            0.0f, // rotation
            tint // pure white tint to preserve original colors
        );
        
        // Only show selection highlight if gate is highlighted (being dragged)
        if (highlight) {
            DrawRectangleLinesEx(body, 3, YELLOW);
        }
    } else {
        // No texture available - draw the colored background and show the label
        DrawRectangleRec(body, drawColor);
        
        if (type == GateType::INPUT || type == GateType::OUTPUT) {
            // For INPUT and OUTPUT gates, show digital state
            if (!preview) {
                const char* stateText = output ? "1" : "0";
                int fontSize = 24;
                int stateWidth = MeasureText(stateText, fontSize);
                Vector2 statePos = {
                    position.x + (info.size.x - stateWidth) / 2,
                    position.y + (info.size.y - fontSize) / 2
                };
                DrawText(stateText, (int)statePos.x, (int)statePos.y, fontSize, output ? LIME : RED);
            }
        } else {
            // Show the label for gates without textures
            int fontSize = 18;
            int textWidth = MeasureText(info.label, fontSize);
            Vector2 textPos = {
                position.x + (info.size.x - textWidth) / 2,
                position.y + (info.size.y - fontSize) / 2
            };
            DrawText(info.label, (int)textPos.x, (int)textPos.y, fontSize, WHITE);
        }
        
        // Draw border for non-textured gates
        DrawRectangleLinesEx(body, highlight ? 3 : 2, highlight ? YELLOW : BLACK);
    }

    // Draw connection points (only if not preview)
    if (!preview) {
        DrawConnectionPoints();
    }
}