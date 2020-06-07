/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// EOBJParser.h: most basic OBJParser!
/*=============================================================================*/
#ifndef ELITE_RASTERIZER_OBJParser
#define	ELITE_RASTERIZER_OBJParser

#include <string>
#include <fstream>
#include <vector>
#include "EMath.h"
#include "EPrimitives.h"

namespace Elite
{
	//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
	static bool ParseOBJ(const std::string& filename, std::vector<Rasterizer::Vertex_Input>& vertices, std::vector<uint32_t>& indices)
	{
		std::ifstream file(filename);
		if (!file)
			return false;

		std::vector<FPoint3> positions;
		std::vector<FVector3> normals;
		std::vector<FVector2> UVs;

		std::vector<Rasterizer::Vertex_Input> rawVertices = {};

		std::string sCommand;
		// start a while iteration ending when the end of file is reached (ios::eof)
		while (!file.eof())
		{
			//read the first word of the string, use the >> operator (istream::operator>>) 
			file >> sCommand;
			//use conditional statements to process the different commands	
			if (sCommand == "#")
			{
				// Ignore Comment
			}
			else if (sCommand == "v")
			{
				//Vertex
				float x, y, z;
				file >> x >> y >> z;
				positions.push_back(FPoint3(x, y, z));
			}
			else if (sCommand == "vt")
			{
				// Vertex TexCoord
				float u, v;
				file >> u >> v;
				UVs.push_back(FVector2(u, 1 - v));
			}
			else if (sCommand == "vn")
			{
				// Vertex Normal
				float x, y, z;
				file >> x >> y >> z;
				normals.push_back(FVector3(x, y, z));
			}
			else if (sCommand == "f")
			{
				//if a face is read:
				//construct the 3 vertices, add them to the vertex array
				//add three indices to the index array
				//add the material index as attibute to the attribute array
				//
				// Faces or triangles
				Rasterizer::Vertex_Input vertex;
				size_t iPosition, iTexCoord, iNormal;
				for (size_t iFace = 0; iFace < 3; iFace++)
				{
					// OBJ format uses 1-based arrays
					file >> iPosition;
					vertex.position = positions[iPosition - 1];

					if ('/' == file.peek())//is next in buffer ==  '/' ?
					{
						file.ignore();//read and ignore one element ('/')

						if ('/' != file.peek())
						{
							// Optional texture coordinate
							file >> iTexCoord;
							vertex.uv = UVs[iTexCoord - 1];
						}

						if ('/' == file.peek())
						{
							file.ignore();

							// Optional vertex normal
							file >> iNormal;
							vertex.normal = normals[iNormal - 1];
						}
					}
					rawVertices.push_back(vertex);
				}
			}
			//read till end of line and ignore all remaining chars
			file.ignore(1000, '\n');
		}
		
		//Assemble - DIRTYYYYYY AF! Needed to do this real fast!
		//Remove duplicates
		for (auto& vertex : rawVertices)
		{
			auto it = find_if(vertices.begin(), vertices.end(),
				[&](const Rasterizer::Vertex_Input& v)
				{
					return v.position == vertex.position && v.uv == vertex.uv && v.normal == vertex.normal;
				});
			if (it == vertices.end()) //not found
			{
				//Add + add to index buffer
				vertices.push_back(vertex);
				indices.push_back(uint32_t(vertices.size() - 1));
			}
			else
			{
				//add current index to index buffer
				indices.push_back(uint32_t(it - vertices.begin()));
			}
		}

		//DIRTY AS WELL
		//The index buffer is a trianglelist, calculate the proper tangents of all triangles and store in the vertices
		//https://stackoverflow.com/questions/5255806/how-to-calculate-tangent-and-binormal
		for (uint32_t i = 0; i < indices.size(); i += 3)
		{
			uint32_t index0 = indices[i];
			uint32_t index1 = indices[i + 1];
			uint32_t index2 = indices[i + 2];

			const FPoint3& p0 = vertices[index0].position;
			const FPoint3& p1 = vertices[index1].position;
			const FPoint3& p2 = vertices[index2].position;
			const FVector2& uv0 = vertices[index0].uv;
			const FVector2& uv1 = vertices[index1].uv;
			const FVector2& uv2 = vertices[index2].uv;

			const FVector3 edge0 = p1 - p0;
			const FVector3 edge1 = p2 - p0;
			const FVector2 diffX = FVector2(uv1.x - uv0.x, uv2.x - uv0.x);
			const FVector2 diffY = FVector2(uv1.y - uv0.y, uv2.y - uv0.y);
			float r = 1.f / Cross(diffX, diffY);

			FVector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
			vertices[index0].tangent += tangent;
			vertices[index1].tangent += tangent;
			vertices[index2].tangent += tangent;
		}
		//Fix the tangents per vertex now because we accumulated
		for (auto& v : vertices)
			v.tangent = GetNormalized(Reject(v.tangent, v.normal));

		return true;
	}
#pragma warning(pop)
}
#endif