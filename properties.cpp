#include <iostream>
#include <fstream>
#include <valarray>

#include "properties.h"
GlobalProperties* GlobalProperties::instance = NULL;

GlobalProperties::GlobalProperties()
{
	// forest parameters
	fP.treeCount = 3;
	fP.maxDepth = 64;

	fP.acPasses = 1;
	fP.acSubsample = 1;

	fP.featureCount = 1000;
	fP.maxOffset = 20;

	fP.fBGRWeight = 1;
	fP.fACCWeight = 1;
	fP.fACRWeight = 1;

	fP.maxLeafPoints = 2000;
	fP.minSamples = 50;

	fP.trainingPixelsPerObject = 500000;
	fP.trainingPixelFactorRegression = 5;
	fP.trainingPixelFactorBG = 3;

	fP.sessionString = "";
	fP.config = "default";

	fP.scaleMin = 0.5;
	fP.scaleMax = 2;
	fP.scaleRel = false;

	fP.meanShiftBandWidth = 100.0;

	//dataset parameters 
	fP.focalLength = 572.4f;
	fP.xShift = 0.f;
	fP.yShift = 0.f;

	fP.rawData = false;
	fP.secondaryFocalLength = 573.5f;
	fP.rawXShift = 0;
	fP.rawYShift = 0;

	fP.fullScreenObject = false;

	fP.imageWidth = 640;
	fP.imageHeight = 480;

	fP.objectCount = 0;
	fP.cellSplit = 5;
	fP.maxImageCount = -1;

	fP.angleMax = 0;
	fP.angleMin = 0;

	fP.useDepth = false;

	fP.training = false;

	// testing parameters
	tP.displayWhileTesting = true;
	tP.rotationObject = false;

	tP.ransacIterations = 256;
	tP.ransacMaxDraws = 10000000;
	tP.ransacCoarseRefinementIterations = 8;
	tP.ransacRefinementIterations = 100;
	tP.ransacBatchSize = 1000;
	tP.ransacMaxInliers = 1000;
	tP.ransacMinInliers = 10;
	tP.ransacRefine = true;
	tP.ransacInlierThreshold2D = 10;
	tP.ransacInlierThreshold3D = 100;

	tP.imageSubSample = 1;

	tP.testObject = 1;
	tP.searchObject = -1;
}

GlobalProperties* GlobalProperties::getInstance()
{
	if (instance == NULL)
		instance = new GlobalProperties();
	return instance;
}
