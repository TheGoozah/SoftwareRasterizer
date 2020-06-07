/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERGBColor.h: struct that represents a RGB color
/*=============================================================================*/
#ifndef ELITE_MATH_RGBCOLOR
#define	ELITE_MATH_RGBCOLOR

#include "EMathUtilities.h"
#include <algorithm>

namespace Elite
{
	struct RGBColor final
	{
		//=== Datamembers ===
		float r = 0.f;
		float g = 0.f;
		float b = 0.f;

		//=== Constructors & Destructor ===
		RGBColor() = default;
		RGBColor(float _r, float _g, float _b) :r(_r), g(_g), b(_b) {}
		RGBColor(const RGBColor& c) : r(c.r), g(c.g), b(c.b) {}
		RGBColor(RGBColor&& c) noexcept : r(std::move(c.r)), g(std::move(c.g)), b(std::move(c.b)) {}
		~RGBColor() = default;

		//=== Operators ===
		RGBColor& operator=(const RGBColor& c)
		{ r = c.r; g = c.g; b = c.b; return *this; }
		RGBColor& operator=(RGBColor&& c) noexcept
		{ r = std::move(c.r); g = std::move(c.g); b = std::move(c.b); return *this;	}

		//=== Arithmetic Operators ===
		inline RGBColor operator+(const RGBColor& c) const
		{ return RGBColor(r + c.r, g + c.g, b + c.b); }
		inline RGBColor operator-(const RGBColor& c) const 
		{ return RGBColor(r - c.r, g - c.g, b - c.b); }
		inline RGBColor operator*(const RGBColor& c) const 
		{ return RGBColor(r * c.r, g * c.g, b * c.b); }
		inline RGBColor operator/(float f) const
		{
			float rev = 1.0f / f;
			return RGBColor(r * rev, g * rev, b * rev);
		}
		inline RGBColor operator*(float f) const
		{ return RGBColor(r * f, g * f, b * f);	}
		inline RGBColor operator/(const RGBColor& c) const
		{ return RGBColor(r / c.r, g / c.g, b / c.b); }

		//=== Compound Assignment Operators ===
		inline RGBColor& operator+=(const RGBColor& c)
		{ r += c.r; g += c.g; b += c.b; return *this; }
		inline RGBColor& operator-=(const RGBColor& c)
		{ r -= c.r; g -= c.g; b -= c.b; return *this; }
		inline RGBColor& operator*=(const RGBColor& c)
		{ r *= c.r; g *= c.g; b *= c.b; return *this; }
		inline RGBColor& operator/=(const RGBColor& c)
		{ r /= c.r; g /= c.g; b /= c.b; return *this; }
		inline RGBColor& operator*=(float f)
		{ r *= f; g *= f; b *= f; return *this; }
		inline RGBColor& operator/=(float f)
		{
			float rev = 1.0f / f;
			r *= rev; g *= rev; b *= rev; return *this;
		}

		//=== Internal RGBColor Functions ===
		inline void Clamp()
		{
			r = Elite::Clamp(r, 0.0f, 1.0f);
			g = Elite::Clamp(g, 0.0f, 1.0f);
			b = Elite::Clamp(b, 0.0f, 1.0f);
		}

		inline void MaxToOne()
		{
			float maxValue = std::max(r, std::max(g, b));
			if (maxValue > 1.f)
				*this /= maxValue;
		}
	};

	//=== Global RGBColor Functions ===
	inline RGBColor Max(const RGBColor& c1, const RGBColor& c2)
	{
		RGBColor c = c1;
		if (c2.r > c.r) c.r = c2.r;
		if (c2.g > c.g) c.g = c2.g;
		if (c2.b > c.b) c.b = c2.b;
		return c;
	}

	inline RGBColor Min(const RGBColor& c1, const RGBColor& c2)
	{
		RGBColor c = c1;
		if (c2.r < c.r) c.r = c2.r;
		if (c2.g < c.g) c.g = c2.g;
		if (c2.b < c.b) c.b = c2.b;
		return c;
	}
}
#endif