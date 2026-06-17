#include "scatter.h"
#include "shared.h"
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MFnTransform.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVector.h>
#include <maya/MDagPath.h>


// Flag names for scatter
#define kVelocityXFlag "-vx"
#define kVelocityXLongFlag "-velocityx"
#define kVelocityYFlag "-vy"
#define kVelocityYLongFlag "-velocityy"
#define kVelocityZFlag "-vz"
#define kVelocityZLongFlag "-velocityz"
#define kGravityFlag "-g" // gravity
#define kGravityLongFlag "-gravity"
#define kSimFramesFlag "-sf" // sim frames
#define kSimFramesLongFlag "-simframes"
#define kOffsetAmountFlag "-o" // offset amount
#define kOffsetAmountLongFlag "-offsetamount"

ScatterCmd::ScatterCmd() {}
ScatterCmd::~ScatterCmd() {}
void* ScatterCmd::creator() { return new ScatterCmd(); }
bool ScatterCmd::isUndoable() const { return true; }

MSyntax ScatterCmd::newSyntax()
{
    MSyntax syntax;
    syntax.addFlag(kVelocityXFlag, kVelocityXLongFlag, MSyntax::kDouble);
    syntax.addFlag(kVelocityYFlag, kVelocityYLongFlag, MSyntax::kDouble);
    syntax.addFlag(kVelocityZFlag, kVelocityZLongFlag, MSyntax::kDouble);
    syntax.addFlag(kGravityFlag, kGravityLongFlag, MSyntax::kDouble);
    syntax.addFlag(kSimFramesFlag, kSimFramesLongFlag, MSyntax::kLong);
    syntax.addFlag(kOffsetAmountFlag, kOffsetAmountLongFlag, MSyntax::kDouble);
    return syntax;
}


MStatus ScatterCmd::doIt(const MArgList& args)
{
    MStatus status;
    MArgDatabase argData(syntax(), args, &status);
    if (!status) return status;
    if (argData.isFlagSet(kVelocityXFlag)) argData.getFlagArgument(kVelocityXFlag, 0, m_velX);
    if (argData.isFlagSet(kVelocityYFlag)) argData.getFlagArgument(kVelocityYFlag, 0, m_velY);
    if (argData.isFlagSet(kVelocityZFlag)) argData.getFlagArgument(kVelocityZFlag, 0, m_velZ);
    if (argData.isFlagSet(kGravityFlag)) argData.getFlagArgument(kGravityFlag, 0, m_gravity);
    if (argData.isFlagSet(kSimFramesFlag)) argData.getFlagArgument(kSimFramesFlag, 0, m_simFrames);
    if (argData.isFlagSet(kOffsetAmountFlag)) argData.getFlagArgument(kOffsetAmountFlag, 0, m_offsetAmount);

	stashOriginalTransforms();
    
    if (SharedSession::objects.length() == 0)
    {
        MGlobal::displayWarning("No meshes selected");
        return MS::kSuccess;
    }

    return redoIt();
}

MStatus ScatterCmd::stashOriginalTransforms()
{
    // Clear global tracking caches
    SharedSession::clear();

    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);

    MItSelectionList iter(selection, MFn::kTransform);
    for (; !iter.isDone(); iter.next())
    {
        MDagPath dagPath;
        iter.getDagPath(dagPath);
        MFnTransform fnTransform(dagPath);

        SharedSession::objects.append(dagPath);
        SharedSession::originalPositions.append(fnTransform.getTranslation(MSpace::kWorld));

        double rot[3];
        MTransformationMatrix::RotationOrder order;
        fnTransform.getRotation(rot, order);
        SharedSession::originalRotations.append(MVector(rot[0], rot[1], rot[2]));
        SharedSession::originalRotationOrders.push_back(order);
    }
    return MS::kSuccess;
}

MStatus ScatterCmd::redoIt()
{
    MString cmd = "ScatterMel(";
    cmd += MString() + m_simFrames + ", " + m_offsetAmount + ", ";
    cmd += MString() + m_velX + ", " + m_velY + ", " + m_velZ + ", " + m_gravity + ");";
    return MGlobal::executeCommand(cmd, false, false);
}

MStatus ScatterCmd::undoIt()
{
    for (unsigned int i = 0; i < SharedSession::objects.length(); ++i)
    {
        if (!SharedSession::objects[i].isValid()) continue;
        MFnTransform fnTransform(SharedSession::objects[i]);
        fnTransform.setTranslation(SharedSession::originalPositions[i], MSpace::kWorld);
        double rotVals[3] = { SharedSession::originalRotations[i].x, SharedSession::originalRotations[i].y, SharedSession::originalRotations[i].z };
        fnTransform.setRotation(rotVals, MTransformationMatrix::kXYZ);
        fnTransform.setRotation(rotVals, SharedSession::originalRotationOrders[i]);
    }
    MString cleanBakeCmd = "cutKey -cl ";
    for (unsigned int i = 0; i < SharedSession::objects.length(); ++i) {
        cleanBakeCmd += "\"" + SharedSession::objects[i].partialPathName() + "\" ";
    }
    MGlobal::executeCommand(cleanBakeCmd, false, false);
    if (MGlobal::executeCommand("objExists \"collision_plane\"")) {
        MGlobal::executeCommand("delete \"collision_plane\"", false, false);
    }
    return MS::kSuccess;
}
