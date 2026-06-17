#pragma once

#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MDagPathArray.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVectorArray.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>

class ScatterCmd : public MPxCommand
{
public:
	ScatterCmd();
	~ScatterCmd();
	static void* creator(); // required by Maya to instatiate the command
	static MSyntax newSyntax(); // Interface to accept flags

	MStatus doIt(const MArgList& args) override; // extry point when the cmd is run
	MStatus stashOriginalTransforms();
	MStatus redoIt() override;
	MStatus undoIt() override;
	bool isUndoable() const override;

private:
	MDagPathArray m_selectedObjects;
	MVectorArray m_oldPositions;
	MVectorArray m_oldRotations; // Stored as Euler vectors (degrees)

	// Command configurable parameters
	double m_velX = 0.2;
	double m_velY = 0.1;
	double m_velZ = 0.2;
	double m_gravity = 15.8;
	int m_simFrames = 120;
	double m_offsetAmount = 1.5;
};
