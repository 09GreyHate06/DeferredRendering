#pragma once
#include <array>
#include <DirectXMath.h>
#include <vector>

namespace DRUtils::BasicMesh
{
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT3 tangent;
		DirectX::XMFLOAT3 bitangent;
	};

	// has tangent/bitangent
	std::array<Vertex, 36> CreateCubeVerticesEx(float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);
	// has tangent/bitangent
	std::array<Vertex, 6> CreatePlaneVerticesEx(float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);
	std::vector<float> CreateCubeVertices(bool hasTexCoords, bool hasNormals, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);
	std::vector<float> CreatePlaneVertices(bool hasTexCoords, bool hasNormals, float minPos = -0.5f, float maxPos = 0.5f, float texCoordU = 1.0f, float texCoordV = 1.0f);

	// has tangent/bitangent
	std::array<uint32_t, 36> CreateCubeIndicesEx();
	// has tangent/bitangent
	std::array<uint32_t, 6> CreatePlaneIndicesEx();

	std::array<uint32_t, 36> CreateCubeIndices();
	std::array<uint32_t, 6> CreatePlaneIndices();
}