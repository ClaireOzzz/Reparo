#pragma once

#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MDagPathArray.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVectorArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

class RepairCmd : public MPxCommand
{
public:
	RepairCmd();
	~RepairCmd();
	static void* creator();
	static MSyntax newSyntax();

	MStatus doIt(const MArgList& args) override;
	MStatus redoIt() override;
	MStatus undoIt() override;
	bool isUndoable() const override;



private:
	int m_frames = 100;
	double m_rotAngle = 360.0;
	double m_arcHeight = 15.0;  // NEW: Control arc height of return animation
};
