#ifndef WIRE_H
#define WIRE_H

#include "raylib.h"
#include "raymath.h"
#include <vector>

// ================================
// WIRE CLASS DECLARATION
// ================================
class Wire {
public:
    // Wire connection data
    int fromGateIndex;
    int toGateIndex;
    int toInputIndex;
    bool state = false;

    // L-routing waypoints
    std::vector<Vector2> waypoints;

    // Constructor
    Wire(int from, int to, int inputIdx)
        : fromGateIndex(from), toGateIndex(to), toInputIndex(inputIdx), state(false) {
    }

    // Calculate L-shaped route between two points
    void CalculateLRoute(Vector2 start, Vector2 end) {
        waypoints.clear();
        waypoints.push_back(start);

        // Calculate intermediate points for L-shaped routing
        float dx = end.x - start.x;
        float dy = end.y - start.y;

        // Determine routing style based on relative positions
        if (abs(dx) > abs(dy)) {
            // Horizontal-first routing
            Vector2 intermediate = { start.x + dx * 0.7f, start.y };
            waypoints.push_back(intermediate);
            waypoints.push_back({ intermediate.x, end.y });
        }
        else {
            // Vertical-first routing  
            Vector2 intermediate = { start.x, start.y + dy * 0.7f };
            waypoints.push_back(intermediate);
            waypoints.push_back({ end.x, intermediate.y });
        }

        waypoints.push_back(end);
    }

    // Draw the wire with L-routing
    void Draw(Color wireColor) const {
        if (waypoints.size() < 2) return;

        // Draw line segments between waypoints
        for (size_t i = 0; i < waypoints.size() - 1; i++) {
            DrawLineEx(waypoints[i], waypoints[i + 1], 3.0f, wireColor);
        }

        // Draw connection points
        if (!waypoints.empty()) {
            DrawCircleV(waypoints.front(), 3, wireColor);
            DrawCircleV(waypoints.back(), 3, wireColor);
        }

        // Draw corner points
        for (size_t i = 1; i < waypoints.size() - 1; i++) {
            DrawCircleV(waypoints[i], 2, wireColor);
        }
    }

    // Check if mouse position is near the wire path
    bool IsNearWirePath(Vector2 mousePos, float threshold = 10.0f) const {
        if (waypoints.size() < 2) return false;

        for (size_t i = 0; i < waypoints.size() - 1; i++) {
            float distance = DistanceToLineSegment(mousePos, waypoints[i], waypoints[i + 1]);
            if (distance <= threshold) {
                return true;
            }
        }

        return false;
    }

private:
    // Helper function to check distance from point to line segment
    float DistanceToLineSegment(Vector2 point, Vector2 lineStart, Vector2 lineEnd) const {
        Vector2 line = Vector2Subtract(lineEnd, lineStart);
        Vector2 pointToStart = Vector2Subtract(point, lineStart);

        float lineLength = Vector2Length(line);
        if (lineLength == 0) {
            return Vector2Distance(point, lineStart);
        }

        float t = Vector2DotProduct(pointToStart, line) / (lineLength * lineLength);
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t; // Clamp between 0 and 1

        Vector2 projection = Vector2Add(lineStart, Vector2Scale(line, t));
        return Vector2Distance(point, projection);
    }
};

#endif // WIRE_H