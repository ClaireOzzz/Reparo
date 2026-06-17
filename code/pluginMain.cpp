#include "scatter.h"
#include "repair.h"
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>

// C:\Users\clair\source\repos\MayaReparoPlugin\x64\Debug

MStatus initializePlugin(MObject obj)
{
    MGlobal::displayInfo("Reparo build loaded: with-syntax v2");
    MFnPlugin plugin(obj, "Autodesk", "1.0", "Any");

    MStatus status = plugin.registerCommand("reparoScatter", ScatterCmd::creator, ScatterCmd::newSyntax);
    if (!status) return status;

    status = plugin.registerCommand("reparoRepair", RepairCmd::creator, RepairCmd::newSyntax);
    if (!status) return status;

    // Source the companion MEL script from the plugin's own directory
    MString pluginDir = plugin.loadPath();
    MString sourceCmd = "source \"" + pluginDir + "/reparo.mel\";";
    if (!MGlobal::executeCommand(sourceCmd))
        MGlobal::displayWarning("Reparo: could not source reparo.mel from " + pluginDir);

    MString createMenuMel = "showReparoUI";
    MGlobal::executeCommand(createMenuMel);

    return MS::kSuccess;
}


MStatus uninitializePlugin(MObject obj)
{
    MFnPlugin plugin(obj);

    MStatus status = plugin.deregisterCommand("reparoScatter");
    if (!status) return status;

    status = plugin.deregisterCommand("reparoRepair");
    if (!status) return status;

    MString removeMenuMel = 
        "if (`menu -exists reparoMenu`) deleteUI reparoMenu;\n"
        "if (`window -exists reparoWindow`) deleteUI reparoWindow;\n";
    
    MGlobal::executeCommand(removeMenuMel);

    return MS::kSuccess;
}