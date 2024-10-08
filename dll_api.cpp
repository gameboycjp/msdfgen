#include "msdfgen.h"
#include "msdfgen-ext.h"
#include "dll_exports.h"

#include "core/ShapeDistanceFinder.h"

using namespace msdfgen;

DLL_EXPORT Shape* DLL_API create_shape()
{
	Shape* shape = new Shape();
	return shape;
}

DLL_EXPORT void DLL_API free_shape(Shape* shape)
{
	if (shape)
		delete shape;
}

DLL_EXPORT void DLL_API shape_bounds(Shape* shape, double& left, double& bottom, double& right, double& top)
{
	Shape::Bounds bounds = shape->getBounds();
	left = bounds.l;
	bottom = bounds.b;
	right = bounds.r;
	top = bounds.t;
}

DLL_EXPORT void DLL_API shape_edge_coloring_simple(Shape* shape, double angleThreshold, unsigned long long seed)
{
	edgeColoringSimple(*shape, angleThreshold, seed);
}

DLL_EXPORT Contour* DLL_API shape_add_contour(Shape* shape)
{
	return &shape->addContour();
}

DLL_EXPORT void DLL_API contour_add_line(Contour* contour, double fromX, double fromY, double toX, double toY)
{
	contour->addEdge(new LinearSegment(Point2(fromX, fromY), Point2(toX, toY)));
}

DLL_EXPORT void DLL_API contour_add_conic(Contour* contour, double fromX, double fromY, 
	double controlX, double controlY, double toX, double toY)
{
	contour->addEdge(new QuadraticSegment(Point2(fromX, fromY), Point2(controlX, controlY), Point2(toX, toY)));
}

DLL_EXPORT void DLL_API contour_add_cubic(Contour* contour, double fromX, double fromY,
	double control0X, double control0Y, double control1X, double control1Y, double toX, double toY)
{
	contour->addEdge(new CubicSegment(Point2(fromX, fromY), Point2(control0X, control0Y), Point2(control1X, control1Y), Point2(toX, toY)));
}

DLL_EXPORT void DLL_API shape_generateMSDF(float* pixels, int width, int height,
	Shape* shape,
	double range, double scaleX, double scaleY, double offsetX, double offsetY,
	double edgeThreshold)
{
	shape->normalize();
	shape->inverseYAxis = true;

	BitmapRef<float, 3> msdf = BitmapRef<float, 3>(pixels, width, height);

	Projection projection = Projection(Vector2(scaleX, scaleY), Vector2(offsetX, offsetY));
	MSDFGeneratorConfig generatorConfig;
	MSDFGeneratorConfig postErrorCorrectionConfig(generatorConfig);
	generatorConfig.errorCorrection.mode = ErrorCorrectionConfig::DISABLED;
	postErrorCorrectionConfig.errorCorrection.distanceCheckMode = ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE;
	// TODO: Include edgeThreshold inside config.errorCorrection, as that feature is currently not implemented
	// and there's no documentation as to what the intent was.

	generateMSDF(msdf, *shape, projection, range, generatorConfig);
	distanceSignCorrection(msdf, *shape, projection, FILL_NONZERO);
	msdfErrorCorrection(msdf, *shape, projection, range, postErrorCorrectionConfig);
}
