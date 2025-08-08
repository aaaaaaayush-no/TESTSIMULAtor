#include "WiringSystem.h"
#include <algorithm>

// ================================
// WIRING SYSTEM CLASS IMPLEMENTATION
// ================================

// Find connection point near mouse position
ConnectionPoint* WiringSystem::FindConnectionPoint(Vector2 mousePos, const std::vector<std::unique_ptr<Gate>>& gates) {
    static ConnectionPoint foundPoint({ 0, 0 }, false, -1);

    for (int i = 0; i < gates.size(); i++) {
        auto points = gates[i]->GetConnectionPoints(i);
        for (const auto& point : points) {
            float distance = Vector2Distance(mousePos, point.position);
            if (distance <= CONNECTION_SNAP_DISTANCE) {
                foundPoint = point;
                return &foundPoint;
            }
        }
    }
    return nullptr;
}

// Handle wire creation
bool WiringSystem::HandleWireClick(Vector2 mousePos, const std::vector<std::unique_ptr<Gate>>& gates) {
    ConnectionPoint* clickedPoint = FindConnectionPoint(mousePos, gates);

    if (!clickedPoint) {
        // Clicked empty space, cancel wire creation
        if (isCreatingWire) {
            isCreatingWire = false;
            wireSourceGate = -1;
            return true;
        }
        return false;
    }

    if (!isCreatingWire) {
        // Start wire creation - must click on output point
        if (!clickedPoint->isInput) {
            isCreatingWire = true;
            wireSourceGate = clickedPoint->gateIndex;
            return true;
        }
    }
    else {
        // Complete wire creation - must click on input point
        if (clickedPoint->isInput && clickedPoint->gateIndex != wireSourceGate) {
            // Check if this input is already connected
            bool alreadyConnected = false;
            for (const auto& wire : wires) {
                if (wire->toGateIndex == clickedPoint->gateIndex &&
                    wire->toInputIndex == clickedPoint->inputIndex) {
                    alreadyConnected = true;
                    break;
                }
            }

            if (!alreadyConnected) {
                // Create the wire
                wires.push_back(std::make_unique<Wire>(wireSourceGate, clickedPoint->gateIndex, clickedPoint->inputIndex));
            }
        }

        // End wire creation regardless
        isCreatingWire = false;
        wireSourceGate = -1;
        return true;
    }

    return false;
}

// Handle wire deletion
bool WiringSystem::HandleWireDeletion(Vector2 mousePos, const std::vector<std::unique_ptr<Gate>>& gates) {
    // Find wire close to mouse position
    for (int i = 0; i < wires.size(); i++) {
        Vector2 startPos = gates[wires[i]->fromGateIndex]->GetOutputPoint();
        Vector2 endPos = gates[wires[i]->toGateIndex]->GetInputPoint(wires[i]->toInputIndex);

        // Simple distance check to wire line (approximate)
        float distToStart = Vector2Distance(mousePos, startPos);
        float distToEnd = Vector2Distance(mousePos, endPos);
        float wireLength = Vector2Distance(startPos, endPos);

        // If mouse is close to the wire line
        if (distToStart + distToEnd <= wireLength + 10.0f) {
            wires.erase(wires.begin() + i);
            return true;
        }
    }
    return false;
}

// Update wire states and propagate signals
void WiringSystem::UpdateSignals(std::vector<std::unique_ptr<Gate>>& gates) {
    // First, compute outputs for INPUT gates only (they don't depend on inputs)
    for (auto& gate : gates) {
        if (gate->GetType() == GateType::INPUT) {
            gate->ComputeOutput();
        }
        else {
            // Reset inputs for non-INPUT gates
            gate->input1 = false;
            gate->input2 = false;
        }
    }

    // Propagate signals through wires
    for (auto& wire : wires) {
        if (wire->fromGateIndex < gates.size() && wire->toGateIndex < gates.size()) {
            // Get the current output state from source gate
            bool signal = gates[wire->fromGateIndex]->output;
            wire->state = signal;  // Update wire visual state

            // Apply signal to destination gate input
            if (wire->toInputIndex == 0) {
                gates[wire->toGateIndex]->input1 = signal;
            }
            else if (wire->toInputIndex == 1) {
                gates[wire->toGateIndex]->input2 = signal;
            }
        }
    }

    // Now compute outputs for all non-INPUT gates with their updated inputs
    for (auto& gate : gates) {
        if (gate->GetType() != GateType::INPUT) {
            gate->ComputeOutput();
        }
    }

    // Final pass: Update wire states again to reflect any changes from logic gates
    for (auto& wire : wires) {
        if (wire->fromGateIndex < gates.size()) {
            wire->state = gates[wire->fromGateIndex]->output;
        }
    }
}

// Draw all wires
void WiringSystem::DrawWires(const std::vector<std::unique_ptr<Gate>>& gates, Vector2 mousePos) {
    // Draw existing wires
    for (const auto& wire : wires) {
        if (wire->fromGateIndex < gates.size() && wire->toGateIndex < gates.size()) {
            Vector2 startPos = gates[wire->fromGateIndex]->GetOutputPoint();
            Vector2 endPos = gates[wire->toGateIndex]->GetInputPoint(wire->toInputIndex);

            Color wireColor = wire->state ? RED : DARKGRAY;
            DrawLineEx(startPos, endPos, 3.0f, wireColor);

            // Draw small circles at connection points
            DrawCircleV(startPos, 3, wireColor);
            DrawCircleV(endPos, 3, wireColor);
        }
    }

    // Draw temporary wire being created
    if (isCreatingWire && wireSourceGate >= 0 && wireSourceGate < gates.size()) {
        Vector2 startPos = gates[wireSourceGate]->GetOutputPoint();
        DrawLineEx(startPos, mousePos, 3.0f, YELLOW);
        DrawCircleV(startPos, 3, YELLOW);
    }
}

// Highlight connection points when in wiring mode
void WiringSystem::HighlightConnectionPoints(const std::vector<std::unique_ptr<Gate>>& gates, Vector2 mousePos) {
    ConnectionPoint* nearbyPoint = FindConnectionPoint(mousePos, gates);

    for (int i = 0; i < gates.size(); i++) {
        auto points = gates[i]->GetConnectionPoints(i);
        for (const auto& point : points) {
            bool isNearby = (nearbyPoint && nearbyPoint->gateIndex == point.gateIndex &&
                nearbyPoint->inputIndex == point.inputIndex &&
                nearbyPoint->isInput == point.isInput);

            if (isNearby) {
                Color highlightColor = YELLOW;
                if (isCreatingWire) {
                    // Show valid/invalid connections
                    if (point.isInput && point.gateIndex != wireSourceGate) {
                        // Check if already connected
                        bool alreadyConnected = false;
                        for (const auto& wire : wires) {
                            if (wire->toGateIndex == point.gateIndex &&
                                wire->toInputIndex == point.inputIndex) {
                                alreadyConnected = true;
                                break;
                            }
                        }
                        highlightColor = alreadyConnected ? RED : GREEN;
                    }
                    else {
                        highlightColor = RED; // Invalid connection
                    }
                }

                DrawCircleV(point.position, CONNECTION_POINT_RADIUS + 3, highlightColor);
            }
        }
    }
}

// Remove wires connected to a gate (when gate is deleted)
void WiringSystem::RemoveWiresForGate(int gateIndex) {
    wires.erase(
        std::remove_if(wires.begin(), wires.end(),
            [gateIndex](const std::unique_ptr<Wire>& wire) {
                return wire->fromGateIndex == gateIndex || wire->toGateIndex == gateIndex;
            }),
        wires.end()
    );
}

// Update wire indices when gates are rearranged
void WiringSystem::UpdateWireIndices(int removedIndex) {
    for (auto& wire : wires) {
        if (wire->fromGateIndex > removedIndex) wire->fromGateIndex--;
        if (wire->toGateIndex > removedIndex) wire->toGateIndex--;
    }
}