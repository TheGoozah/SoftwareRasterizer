#include "ERenderer.h"
#include "ERGBColor.h"
#include "EManager.h"
#include "EOBJParser.h"

#include <SDL.h>
#include <SDL_image.h>

#include <thread>
#include <future>
#include <iostream>

Elite::Renderer::Renderer(SDL_Window * pWindow)
{
	//Initialize
	m_pWindow = pWindow;
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	int width, height = 0;
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	//Create depth buffer
	m_pDepthBufferPixels = new float[m_Width * (int)m_Height];

	//Load Model
	ParseOBJ("Resources/vehicle.obj", m_Vertices, m_Indices);

	//Load diffuse texture
	m_pDiffuseTexture = IMG_Load("Resources/vehicle_diffuse.png");
	m_pNormalTexture = IMG_Load("Resources/vehicle_normal.png");
	m_pSpecularMap = IMG_Load("Resources/vehicle_specular.png");
	m_pGlossMap = IMG_Load("Resources/vehicle_gloss.png");
}

Elite::Renderer::~Renderer()
{
	delete[] m_pDepthBufferPixels;
	if(m_pDiffuseTexture)
		SDL_FreeSurface(m_pDiffuseTexture);
	if (m_pNormalTexture)
		SDL_FreeSurface(m_pNormalTexture);
	if (m_pSpecularMap)
		SDL_FreeSurface(m_pSpecularMap);
	if (m_pGlossMap)
		SDL_FreeSurface(m_pGlossMap);
}

void Elite::Renderer::Update(Timer* pTimer)
{
	//Time
	float rotationSpeed = 1.f;
	float dt = pTimer->GetElapsed();
	m_RotationAngle += dt * rotationSpeed;

	m_RotationMatrix = MakeRotationY(m_RotationAngle);
}

void Elite::Renderer::Render(Rasterizer::Camera& camera)
{
	SDL_LockSurface(m_pBackBuffer);

	RGBColor clearColor = { 0.3f, 0.3f, 0.3f };
	for (uint32_t r = 0; r < m_Height; ++r)
	{
		for (uint32_t c = 0; c < m_Width; ++c)
		{
			m_pDepthBufferPixels[c + (r * m_Width)] = std::numeric_limits<float>::max();
			m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(clearColor.r * 255),
				static_cast<uint8_t>(clearColor.g * 255),
				static_cast<uint8_t>(clearColor.b * 255));
		}
	}

	using namespace Rasterizer;
	//--- "VERTEX SHADER" ---
	//Transform all vertices into Raster (World -> Camera -> NDC -> Raster)
	std::vector<Vertex_Output> transformedVertices;
	VertexTransformationFunction(m_Vertices, transformedVertices, camera.LookAt(), 
		m_Width, m_Height, camera.FOVAngle, camera.NearPlane, camera.FarPlane);

	//--- RASTERIZER ---
	//Go over all the triangles
	uint32_t size = 0;
	if (m_TopologyType == PrimitiveTopology::TriangleList)
		size = (uint32_t)m_Indices.size();
	else if (m_TopologyType == PrimitiveTopology::TriangleStrip)
		size = (uint32_t)m_Indices.size() - 2;

	//For every primitive...
	for (uint32_t i = 0; i < size;)
	{
		uint32_t evenIndex = 0;
		if(m_TopologyType == PrimitiveTopology::TriangleStrip)
			evenIndex = i % 2;
		uint32_t index0 = m_Indices[i];
		uint32_t index1 = m_Indices[i + 1 + evenIndex];
		uint32_t index2 = m_Indices[i + 2 - evenIndex];

		//Loop based on topology
		if (m_TopologyType == PrimitiveTopology::TriangleList)
			i += 3;
		else if (m_TopologyType == PrimitiveTopology::TriangleStrip)
			i += 1;

		//Calculate the bounding box - make sure you don't go out of the screen with these values
		FPoint4 v0 = transformedVertices[index0].position;
		FPoint4 v1 = transformedVertices[index1].position;
		FPoint4 v2 = transformedVertices[index2].position;

		//Do Culling
		if(!PointInFrustum(v0) || !PointInFrustum(v1) || !PointInFrustum(v1))
			continue;

		//Precompute values for correct depth interpolation --> Depth Buffer will no longer be linear!
		//https://developer.nvidia.com/content/depth-precision-visualized
		//Divide by the original viewspace depth value, stored in the w component
		float v0InvDepth = 1.f / v0.w;
		float v1InvDepth = 1.f / v1.w;
		float v2InvDepth = 1.f / v2.w;

		//NDC to Raster Space
		NDCToRaster(v0);
		NDCToRaster(v1);
		NDCToRaster(v2);

		//Bounding Box
		float xMin = std::min(std::min(v0.x, v1.x), v2.x);
		float yMin = std::min(std::min(v0.y, v1.y), v2.y);
		float xMax = std::max(std::max(v0.x, v1.x), v2.x);
		float yMax = std::max(std::max(v0.y, v1.y), v2.y);
		BoundingBox bb = BoundingBox(
			FPoint2(std::max(0.f, xMin), std::max(0.f, yMin)),
			FPoint2(std::min(m_Width - 1.f, xMax), std::min(m_Height - 1.f, yMax)));

		//Use the bounding box to go over all the pixels: do the depth test (interpolated), 
		//if pixels on triangles interpolate other values and store a fragment (vertex_output for that pixel)
		for (uint32_t r = (uint32_t)bb.topLeft.y; (int)r < bb.rightBottom.y && r < m_Height; ++r)
		{
			for (uint32_t c = (uint32_t)bb.topLeft.x; (int)c < bb.rightBottom.x && c < m_Width; ++c)
			{
				//Color to render
				RGBColor finalColor = clearColor;

				//Pixel coordinates
				FPoint2 pixel = FPoint2((float)c, (float)r);

				//Check if this current pixel overlaps the triangle formed by the vertices
				//Use the Vector2 Cross product to know the sign for all edges
				FVector2 edge0 = FVector2(v1 - v0);
				FVector2 pointToEdge0 = pixel - FPoint2(v0);
				float w2 = Cross(pointToEdge0, edge0);

				FVector2 edge1 = FVector2(v2 - v1);
				FVector2 pointToEdge1 = pixel - FPoint2(v1);
				float w0 = Cross(pointToEdge1, edge1);

				FVector2 edge2 = FVector2(v0 - v2);
				FVector2 pointToEdge2 = pixel - FPoint2(v2);
				float w1 = Cross(pointToEdge2, edge2);

				//If inside triangle
				bool isInsideTriangle = w0 >= 0.f && w1 >= 0.f && w2 >= 0.f;
				if (isInsideTriangle)
				{
					//Barycentric Coordinates
					float totalArea = abs(Cross(FVector2(v0 - v1),
						FVector2(v0 - v2)));
					w0 /= totalArea;
					w1 /= totalArea;
					w2 /= totalArea;

					//Do depth test (with correct depth interpolation = 1 / (z/w * weight))
					float ZBuffer = (1.f / v0.z * w0) + (1.f / v1.z * w1) + (1.f / v2.z * w2);
					float invZBuffer = 1.f / ZBuffer;
					if (invZBuffer > 1.f || invZBuffer < 0.f)
						break;

					if (invZBuffer < m_pDepthBufferPixels[c + (r * m_Width)])
					{
						//Write to depth buffer
						m_pDepthBufferPixels[c + (r * m_Width)] = invZBuffer;

						//W-Buffer interpolation for attribute interpolation
						float depthInterpolated = 1.f /
							(v0InvDepth * w0 + v1InvDepth * w1 + v2InvDepth * w2);

						//Interpolate all values - STORING THESE ATTRIBUTES
						FVector2 uv = DepthInterpolateAttributes(
							transformedVertices[index0].uv, transformedVertices[index1].uv, transformedVertices[index2].uv,
							w0, w1, w2, v0InvDepth, v1InvDepth, v2InvDepth, depthInterpolated);
						FVector3 normal = DepthInterpolateAttributes(
							transformedVertices[index0].normal, transformedVertices[index1].normal, transformedVertices[index2].normal,
							w0, w1, w2, v0InvDepth, v1InvDepth, v2InvDepth, depthInterpolated);
						FVector3 tangent = DepthInterpolateAttributes(
							transformedVertices[index0].tangent, transformedVertices[index1].tangent, transformedVertices[index2].tangent,
							w0, w1, w2, v0InvDepth, v1InvDepth, v2InvDepth, depthInterpolated);
						//This could be done by interpolating the WorldPosition and calculating the viewDirection accordingly 
						FVector3 viewDirection = DepthInterpolateAttributes(
							transformedVertices[index0].viewDirection, transformedVertices[index1].viewDirection, transformedVertices[index2].viewDirection,
							w0, w1, w2, v0InvDepth, v1InvDepth, v2InvDepth, depthInterpolated);

						//Ouput vertex for this pixel/fragment
						Vertex_Output pixelInformation = {};
						pixelInformation.position = FPoint4(pixel, invZBuffer, depthInterpolated);
						pixelInformation.uv = uv;
						pixelInformation.normal = normal;
						pixelInformation.tangent = tangent;
						pixelInformation.viewDirection = viewDirection;

						//Render the pixel(SHADING - PIXEL SHADER)
						RenderPixel(pixelInformation);
					}
				}
			}
		}
	}

	//Render depth instead - enable/disable this
	if (m_VisualizeDepthBuffer)
	{
		for (int i = 0; i < int(m_Width * m_Height); ++i)
		{
			float depthValue = m_pDepthBufferPixels[i];
			float depthRemapped = Remap(depthValue, 0.997f, 1.f);
			m_pBackBufferPixels[i] = SDL_MapRGB(m_pBackBuffer->format,
				static_cast<uint8_t>(depthRemapped * 255),
				static_cast<uint8_t>(depthRemapped * 255),
				static_cast<uint8_t>(depthRemapped * 255));
		}
	}

	SDL_UnlockSurface(m_pBackBuffer);
	SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Elite::Renderer::SaveBackbufferToImage() const
{ return SDL_SaveBMP(m_pBackBuffer, "BackbufferRender.bmp"); }

void Elite::Renderer::VertexTransformationFunction(
	const std::vector<Rasterizer::Vertex_Input>& originalVertices,
	std::vector<Rasterizer::Vertex_Output>& transformedVertices, const FMatrix4& cameraToWorld,
	uint32_t width, uint32_t height, float fovAngle, float nearPlane, float farPlane)
{
	//Camera settings
	fovAngle = fovAngle * static_cast<float>(E_TO_RADIANS);
	float fov = tan(fovAngle / 2.f);
	float aspectRatio = width / (float)height;

	//View matrix - Inverse ONB
	FMatrix4 worldToCamera = Inverse(cameraToWorld);

	//Perspective Projection Matrix - RH
	FMatrix4 projectionMatrix = FMatrix4(
		FVector4(1.f / (aspectRatio * fov), 0, 0, 0),
		FVector4(0, 1.f / fov, 0, 0),
		FVector4(0, 0, farPlane / (nearPlane - farPlane), -1.f),
		FVector4(0, 0, (nearPlane * farPlane) / (nearPlane - farPlane), 0));

	//(World)ViewProjection Matrix
	FMatrix4 viewProjectionMatrix = projectionMatrix * worldToCamera;
	FMatrix4 worldMatrix = m_RotationMatrix;

	for (const Rasterizer::Vertex_Input& v : originalVertices)
	{
		//Put position in ViewSpace
		FPoint4 p = viewProjectionMatrix * worldMatrix * FPoint4(v.position);

		//Normal and Tangent in WorldSpace
		FMatrix3 worldMatrix3x3 = FMatrix3(worldMatrix);
		FVector3 normal = worldMatrix3x3 * v.normal;
		FVector3 tangent = worldMatrix3x3 * v.tangent;

		FVector3 cameraPosition = FVector3(cameraToWorld(0, 3), cameraToWorld(1, 3), cameraToWorld(2, 3));
		FVector3 viewDirection = FVector3(FVector4(worldMatrix * FPoint4(v.position))) - cameraPosition;
		Normalize(viewDirection);

		//Perspective Divide
		p.x /= p.w;
		p.y /= p.w;
		p.z /= p.w;

		//Store the resulting point in Raster Space
		Rasterizer::Vertex_Output vertex = Rasterizer::Vertex_Output(p, normal, tangent, v.color, v.uv);
		vertex.viewDirection = viewDirection;
		transformedVertices.push_back(vertex);
	}
}

void Elite::Renderer::RenderPixel(const Rasterizer::Vertex_Output& v)
{
	bool succeeded = true;
	RGBColor finalColor = { 0.f, 0.f, 0.f };

	FVector3 lightDirection = { .577f, -.577f, -.577f };
	float lightIntensity = 7.0f;

	//--- NORMAL MAPPING ---
	//Calculate local axis for normal mapping (tangent space)
	FVector3 binormal = Cross(v.tangent, v.normal);
	bool flipGreenChannel = false;
	if (flipGreenChannel)
		binormal = -binormal;
	FMatrix3 tangentAxis = FMatrix3(v.tangent, binormal, v.normal);

	//Sample Normal Map, remap [0,1] uv range into [-1, 1] vector range
	RGBColor normalMap = { 0, 0, 0 };
	succeeded = SampleSurface(m_pNormalTexture, v.uv, normalMap);
	FVector3 normal = { 2.f * normalMap.r - 1.f, 2.f * normalMap.g - 1.f, 2.f * normalMap.b - 1.f };
	FVector3 finalNormal = tangentAxis * normal;

	//--- DIFFUSE COLOR ---
	RGBColor diffuseMap = {};
	succeeded = SampleSurface(m_pDiffuseTexture, v.uv, diffuseMap);
	float diffuseStrength = Dot(-finalNormal, lightDirection);
	diffuseStrength = std::max(0.f, diffuseStrength); //make sure no negative values
	diffuseStrength /= static_cast<float>(E_PI);
	diffuseStrength *= lightIntensity;
	RGBColor diffuseColor = diffuseMap * diffuseStrength;

	//--- PHONG SPECULAR
	FVector3 reflect = Reflect(lightDirection, finalNormal);
	float specularStrength = Dot(v.viewDirection, reflect);
	specularStrength = Clamp(specularStrength, 0.f, 1.f);
	RGBColor glossMap = {};
	SampleSurface(m_pGlossMap, v.uv, glossMap);
	float shininess = 25.f;
	specularStrength = std::pow(specularStrength, glossMap.r * shininess); //every channel of glossmap should have same value
	RGBColor specularMap = {};
	SampleSurface(m_pSpecularMap, v.uv, specularMap);
	RGBColor specularColor = specularMap * specularStrength;

	//--- FINAL COLOR ---
	RGBColor ambientColor = { 0.05f, 0.05f, 0.05f };
	finalColor = ambientColor + diffuseColor + specularColor;
	finalColor.MaxToOne();

	//Get the actual pixel we are processing and color it
	m_pBackBufferPixels[int(v.position.x + (v.position.y * m_Width))] = SDL_MapRGB(m_pBackBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));
}