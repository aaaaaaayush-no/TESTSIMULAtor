#ifndef WIRE_H
#define WIRE_H

// ================================
// WIRE STRUCTURE
// ================================
struct Wire {
    int fromGateIndex;
    int toGateIndex;
    int toInputIndex;  // Which input of the destination gate (0 or 1)
    bool state = false;

    Wire(int from, int to, int inputIdx)
        : fromGateIndex(from), toGateIndex(to), toInputIndex(inputIdx) {
    }
};

#endif // WIRE_H