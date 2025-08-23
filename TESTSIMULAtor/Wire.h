#ifndef WIRE_H
#define WIRE_H

#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

// Forward declaration to avoid circular dependency
class Gate;

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

    // Calculate L-shaped route between two points with gate avoidance
    void CalculateLRoute(Vector2 start, Vector2 end, const std::vector<std::unique_ptr<Gate>>* gates = nullptr) {
        waypoints.clear();
        waypoints.push_back(start);

        // If no gates provided, use simple L-routing
        if (!gates || gates->empty()) {
            CalculateSimpleLRoute(start, end);
            waypoints.push_back(end);
            return;
        }

        const float CLEARANCE = 15.0f; // Reasonable clearance for gate avoidance

        // Try simple avoidance routing
        Vector2 avoidanceRoute = CalculateAvoidanceRoute(start, end, *gates, CLEARANCE);

        if (avoidanceRoute.x != -1) {
            // Use the avoidance route
            waypoints.push_back(avoidanceRoute);
            waypoints.push_back({ avoidanceRoute.x, end.y });
        }
        else {
            // Fallback to simple L-routing if avoidance fails
            CalculateSimpleLRoute(start, end);
        }

        waypoints.push_back(end);
    }

    // Draw the wire with L-routing
    void Draw(Color wireColor) const;

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
    // Simple L-routing without gate avoidance
    void CalculateSimpleLRoute(Vector2 start, Vector2 end);

    // More precise line-rectangle intersection check
    bool DoesLineIntersectRect(Vector2 start, Vector2 end, Rectangle rect) const;

    // Calculate route that avoids gates
    Vector2 CalculateAvoidanceRoute(Vector2 start, Vector2 end, const std::vector<std::unique_ptr<Gate>>& gates, float clearance) const;

    // Check if the L-route intersects with any gates
    bool DoesRouteIntersectGates(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4,
        const std::vector<std::unique_ptr<Gate>>& gates, float clearance) const;

    // Check if a line segment intersects with any gates
    bool DoesLineIntersectGates(Vector2 start, Vector2 end, const std::vector<std::unique_ptr<Gate>>& gates, float clearance) const;

    // Find alternative route around obstacles
    Vector2 FindAvoidanceRoute(Vector2 start, Vector2 end, const std::vector<std::unique_ptr<Gate>>& gates, float clearance) const;

    // Helper function to check distance from point to line segment
    float DistanceToLineSegment(Vector2 point, Vector2 lineStart, Vector2 lineEnd) const;

    // Validate that all segments are orthogonal (no diagonals)
    void ValidateOrthogonalRouting();
};

#endif // WIRE_H