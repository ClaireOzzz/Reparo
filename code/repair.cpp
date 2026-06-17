#include "repair.h"
#include "shared.h"
#include <maya/MGlobal.h>
#include <maya/MFnTransform.h>
#include <maya/MVector.h>
#include <maya/MDagPath.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

// Flag names for repairTransform
#define kFramesFlag "-f" // frames
#define kFramesLongFlag  "-frames"
#define kRotAngleFlag "-ra" //rotangle
#define kRotAngleLongFlag "-rotangle"
#define kArcHeightFlag "-ah" // arc height
#define kArcHeightLongFlag "-archeight"

RepairCmd::RepairCmd() {}
RepairCmd::~RepairCmd() {}
void* RepairCmd::creator() { return new RepairCmd(); }
bool RepairCmd::isUndoable() const { return true; }

MSyntax RepairCmd::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag(kFramesFlag, kFramesLongFlag, MSyntax::kLong);
    syntax.addFlag(kRotAngleFlag, kRotAngleLongFlag, MSyntax::kDouble);
    syntax.addFlag(kArcHeightFlag, kArcHeightLongFlag, MSyntax::kDouble);
    return syntax;
}

MStatus RepairCmd::doIt(const MArgList& args)
{
    MArgDatabase argData(syntax(), args);
    if (argData.isFlagSet(kFramesFlag)) argData.getFlagArgument(kFramesFlag, 0, m_frames);
    if (argData.isFlagSet(kRotAngleFlag)) argData.getFlagArgument(kRotAngleFlag, 0, m_rotAngle);
    if (argData.isFlagSet(kArcHeightFlag)) argData.getFlagArgument(kArcHeightFlag, 0, m_arcHeight);

    if (SharedSession::objects.length() == 0)
    {
        MGlobal::displayWarning("No active scatter session coordinates found to repair. Run randomizeTransform first.");
        return MS::kSuccess;
    }
    return redoIt();
}

MStatus RepairCmd::redoIt()
{
    // Build an ordering of objects by distance between their CURRENT position and true original position.
    // Furthest moves first, closest last. We'll stagger their start frames while ensuring all animations
    // are finished by m_frames.
    std::vector<std::pair<unsigned int, double>> order; // (index, distance)
    for (unsigned int i = 0; i < SharedSession::objects.length(); ++i) {
        if (!SharedSession::objects[i].isValid()) continue;
        MFnTransform fn(SharedSession::objects[i]);
        MVector curPos = fn.getTranslation(MSpace::kWorld); // takes into account user edits
        MVector origPos = SharedSession::originalPositions[i];
        double dist = (curPos - origPos).length();
        order.emplace_back(i, dist);
    }

    if (order.empty()) {
        MGlobal::displayWarning("No valid scattered objects for repair.");
        return MS::kSuccess;
    }

    // Sort descending so largest distance first
    std::sort(order.begin(), order.end(), [](const std::pair<unsigned int, double>& a, const std::pair<unsigned int, double>& b) {
        return a.second > b.second;
        });

    // Choose an animation duration per object (frames). Shorter than total so some end earlier.
    int animDuration = std::max(2, m_frames / 2);
    static const double kRad2Deg = 180.0 / 3.14159265358979323846;
    unsigned int n = static_cast<unsigned int>(order.size());
    MString prepCmd = "if (`objExists \"collision_plane\"`) { delete \"collision_plane\"; }";
    MGlobal::executeCommand(prepCmd, false, false);

    for (unsigned int rank = 0; rank < n; ++rank) {
        unsigned int idx = order[rank].first;
        MString obj = SharedSession::objects[idx].fullPathName();

        int startFrame = 1;
        if (n > 1) {
            startFrame = 1 + static_cast<int>(((m_frames - animDuration) * rank) / (n - 1));
        }
        int endFrame = startFrame + animDuration - 1;
        if (endFrame > m_frames) endFrame = m_frames;

        // Convert the required variables to strings
        MString strOrigX(std::to_string(SharedSession::originalPositions[idx].x).c_str());
        MString strOrigY(std::to_string(SharedSession::originalPositions[idx].y).c_str());
        MString strOrigZ(std::to_string(SharedSession::originalPositions[idx].z).c_str());

        MString strOrigRx(std::to_string(SharedSession::originalRotations[idx].x * kRad2Deg).c_str());
        MString strOrigRy(std::to_string(SharedSession::originalRotations[idx].y * kRad2Deg).c_str());
        MString strOrigRz(std::to_string(SharedSession::originalRotations[idx].z * kRad2Deg).c_str());

        MString strStartFrame(std::to_string(startFrame).c_str());
        MString strEndFrame(std::to_string(endFrame).c_str());
        MString strArcHeight(std::to_string(m_arcHeight).c_str());
        MString strRotAngle(std::to_string(m_rotAngle).c_str());

        MString melCall = "RepairMel(\"";
        melCall += obj + "\", ";
        melCall += strOrigX + ", " + strOrigY + ", " + strOrigZ + ", ";
        melCall += strOrigRx + ", " + strOrigRy + ", " + strOrigRz + ", ";
        melCall += strStartFrame + ", " + strEndFrame + ", " + strArcHeight + ", " + strRotAngle + ");";

        MGlobal::executeCommand(melCall, false, false);
    }

    MString postCmd = "playbackOptions -max " + MString(std::to_string(m_frames).c_str()) + "; currentTime 1;";
    return MGlobal::executeCommand(postCmd, false, false);
}

MStatus RepairCmd::undoIt()
{
    MString clearRepairCmd = "cutKey -time \"2:" + MString(std::to_string(m_frames).c_str()) + "\" -cl ";
    for (unsigned int i = 0; i < SharedSession::objects.length(); ++i) {
        clearRepairCmd += "\"" + SharedSession::objects[i].partialPathName() + "\" ";
    }
    return MGlobal::executeCommand(clearRepairCmd, false, false);
}