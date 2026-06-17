// ScatterSession.cpp
#include "shared.h"

namespace SharedSession {
    MDagPathArray objects;
    MVectorArray  originalPositions;
    MVectorArray  originalRotations;
    std::vector<MTransformationMatrix::RotationOrder> originalRotationOrders;

    void clear() {
        objects.clear();
        originalPositions.clear();
        originalRotations.clear();
        originalRotationOrders.clear();
    }
}