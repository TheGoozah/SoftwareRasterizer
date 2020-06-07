/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EPrimitives.h: class holds representations of different things like triangles, 
// vertices, etc.
/*=============================================================================*/
#ifndef ELITE_RASTERIZER_PRIMITIVES
#define	ELITE_RASTERIZER_PRIMITIVES

#include "EMath.h"
#include "ERGBColor.h"

namespace Elite
{
	namespace Rasterizer
	{
		struct BoundingBox
		{
			FPoint2 topLeft = {};
			FPoint2 rightBottom = {};

			BoundingBox() = default;
			BoundingBox(const FPoint2& _topLeft, const FPoint2& _rightBottom)
				:topLeft(_topLeft), rightBottom(_rightBottom) {}
		};

		//Vertex needs at least a position
		struct Vertex_Input
		{
			FPoint3 position = {};
			FPoint3 worldPosition = {};
			FVector3 normal = {};
			FVector3 tangent = {};
			RGBColor color = {};
			FVector2 uv = {};

			Vertex_Input() = default;
			Vertex_Input(const FPoint3& p, const FVector3& _normal, const RGBColor& _color, const FVector2& _uv) :
				position(p), normal(_normal), color(_color), uv(_uv)
			{}
		};

		struct Vertex_Output
		{
			FPoint4 position = {};
			FVector3 viewDirection = {};
			FVector3 normal = {};
			FVector3 tangent = {};
			RGBColor color = {};
			FVector2 uv = {};

			Vertex_Output() = default;
			Vertex_Output(const FPoint4& _position, const FVector3& _normal, const FVector3& _tangent, const RGBColor& _color, const FVector2& _uv) :
				position(_position), normal(_normal), tangent(_tangent), color(_color), uv(_uv)
			{}
		};
	}
}
#endif