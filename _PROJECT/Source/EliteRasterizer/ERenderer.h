/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render to.
/*=============================================================================*/
#ifndef ELITE_RASTERIZER_RENDERER
#define	ELITE_RASTERIZER_RENDERER

#include <cstdint>
#include <vector>

#include "EPrimitives.h"
#include "ECamera.h"

struct SDL_Window;
#include <SDL_surface.h>

#include "ETimer.h"

namespace Elite
{
	enum class PrimitiveTopology
	{
		TriangleList,
		TriangleStrip
	};

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render(Rasterizer::Camera& camera);
		void Update(Timer* pTimer); //DIRTY FIX FOR DEMO, GET RID OF THIS :)

		void ToggleDepthVisualization() { m_VisualizeDepthBuffer = !m_VisualizeDepthBuffer; }
		bool SaveBackbufferToImage() const;

	private:
		//Window Resources
		SDL_Window* m_pWindow = nullptr;
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;
		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		//Depth Buffer
		float* m_pDepthBufferPixels = nullptr;
		bool m_VisualizeDepthBuffer = false;

		//Mesh Render Settings
		PrimitiveTopology m_TopologyType = PrimitiveTopology::TriangleList;
		std::vector<Rasterizer::Vertex_Input> m_Vertices = {};
		std::vector<uint32_t> m_Indices = {};
		SDL_Surface* m_pDiffuseTexture = nullptr; //DiffuseMap Texture
		SDL_Surface* m_pNormalTexture = nullptr; //NormalMap Texture
		SDL_Surface* m_pSpecularMap = nullptr;
		SDL_Surface* m_pGlossMap = nullptr;

		//DIRTY DEMO VARIABLES
		float m_RotationAngle = 0.f;
		FMatrix4 m_RotationMatrix = {};

		//Function that transforms the vertices from the mesh into camera space
		void VertexTransformationFunction(const std::vector<Rasterizer::Vertex_Input>& originalVertices,
			std::vector<Rasterizer::Vertex_Output>& transformedVertices, const FMatrix4& cameraToWorld,
			uint32_t width, uint32_t height, float fovAngle, float nearPlane = 1.f, float farPlane = 1000.f);
		
		inline void NDCToRaster(FPoint4& p)
		{
			//NDC projected to Raster Space -> [-1,1] to [0,1] and take into account screen size
			p.x = (p.x + 1) / 2.f * m_Width;
			p.y = (1 - p.y) / 2.f * m_Height;
		}

		inline bool PointInFrustum(const FPoint4& p)
		{ return !(p.x < -1.f || p.x > 1.f || p.y < -1.f || p.y > 1.f || p.z < 0.f || p.z > 1.f); }

		inline bool SampleSurface(const SDL_Surface* pSurface, const FVector2& uv, RGBColor& outputColor)
		{
			int width = pSurface->w;
			int height = pSurface->h;

			uint32_t* pixels = (uint32_t*)pSurface->pixels;
			int xTex = int(uv.x * width + 0.5f);
			int yTex = int(uv.y * height + 0.5f);

			//Safety - not reading out of bounds - Should be in Sample in Texture class
			if (xTex < 0.f || xTex > width || yTex < 0.f || yTex > height)
				return false;

			uint8_t rC, gC, bC = 0;
			SDL_GetRGB(pixels[xTex + (yTex * width)], pSurface->format, &rC, &gC, &bC);
			outputColor = RGBColor(rC / 255.f, gC / 255.f, bC / 255.f);
			return true;
		}

		inline void RenderPixel(const Rasterizer::Vertex_Output& v); //PIXEL SHADER

		template<typename T>
		T DepthInterpolateAttributes(const T& attribute_0, const T& attribute_1, const T& attribute_2,
			const float w0, const float w1, const float w2,
			const float invDepth_0, const float invDepth_1, const float invDepth_2, float invInterpolatedDepth)
		{
			//(attr / v.w -- > attr * 1 / v.w)
			T attr0 = attribute_0 * invDepth_0;
			T attr1 = attribute_1 * invDepth_1;
			T attr2 = attribute_2 * invDepth_2;
			return (attr0 * w0 + attr1 * w1 + attr2 * w2) * invInterpolatedDepth; //--> uvInterpolated / depthInterpolated
		}
	};
}

#endif