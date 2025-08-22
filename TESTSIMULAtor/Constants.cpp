#include "Constants.h"

// Define the actual GATE_DATA map
std::map<GateType, GateInfo> GATE_DATA = {
    {GateType::INPUT,  {{60, 40}, LIGHTGRAY, "INP", nullptr, {0}}},
    {GateType::OUTPUT, {{60, 40}, SKYBLUE,   "OUT", nullptr, {0}}},
    {GateType::AND,    {{75, 50}, DARKGREEN, "AND", "resources/and_gate.png", {0}}},
    {GateType::OR,     {{75, 50}, DARKBLUE,  "OR",  "resources/or_gate.png", {0}}},
    {GateType::NOT,    {{75, 50}, MAROON,    "NOT", "resources/not_gate.png", {0}}},
    {GateType::NAND,   {{75, 50}, LIME,      "NAND","resources/nand_gate.png", {0}}},
    {GateType::NOR,    {{75, 50}, PURPLE,    "NOR", "resources/nor_gate.png", {0}}}
};