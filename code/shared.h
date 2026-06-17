#pragma once
#include <maya/MDagPathArray.h>
#include <maya/MVectorArray.h>
#include <vector>

// State shared between the scatter and repair commands for one session.
namespace SharedSession {
    extern MDagPathArray objects;
    extern MVectorArray  originalPositions;   // world-space translate
    extern MVectorArray  originalRotations;   // radians
    extern std::vector<MTransformationMatrix::RotationOrder> originalRotationOrders;
    void clear();
}