#include "BasicMesh.h"

using namespace DirectX;

namespace DRUtils::BasicMesh
{
	struct TangentSpace
	{
		XMFLOAT3 tangent;
		XMFLOAT3 bitangent;
	};

	static TangentSpace GetTangentSpace(const XMFLOAT3& posA, const XMFLOAT3& posB, const XMFLOAT3& posC, const XMFLOAT2& uvA, const XMFLOAT2& uvB, const XMFLOAT2& uvC)
	{
		XMVECTOR pos1 = XMLoadFloat3(&posA);
		XMVECTOR pos2 = XMLoadFloat3(&posB);
		XMVECTOR pos3 = XMLoadFloat3(&posC);
		XMVECTOR uv1 = XMLoadFloat2(&uvA);
		XMVECTOR uv2 = XMLoadFloat2(&uvB);
		XMVECTOR uv3 = XMLoadFloat2(&uvC);

		XMVECTOR edge1 = pos2 - pos1;
		XMVECTOR edge2 = pos3 - pos1;
		XMVECTOR deltaUV1 = uv2 - uv1;
		XMVECTOR deltaUV2 = uv3 - uv1;

		XMFLOAT3 edgeA;
		XMFLOAT3 edgeB;
		XMStoreFloat3(&edgeA, edge1);
		XMStoreFloat3(&edgeB, edge2);

		XMFLOAT2 deltaUVA;
		XMFLOAT2 deltaUVB;
		XMStoreFloat2(&deltaUVA, deltaUV1);
		XMStoreFloat2(&deltaUVB, deltaUV2);
		float f = 1.0f / (deltaUVA.x * deltaUVB.y - deltaUVB.x * deltaUVA.y);

		XMFLOAT3 tangent = {};
		tangent.x = f * (deltaUVB.y * edgeA.x - deltaUVA.y * edgeB.x);
		tangent.y = f * (deltaUVB.y * edgeA.y + deltaUVA.y * edgeB.y);
		tangent.z = f * (deltaUVB.y * edgeA.z + deltaUVA.y * edgeB.z);

		XMFLOAT3 bitangent = {};
		bitangent.x = f * (-deltaUVB.x * edgeA.x + deltaUVA.x * edgeB.x);
		bitangent.y = f * (-deltaUVB.x * edgeA.y - deltaUVA.x * edgeB.y);
		bitangent.z = f * (-deltaUVB.x * edgeA.z - deltaUVA.x * edgeB.z);

		return { tangent, bitangent };
	}

	std::array<Vertex, 36> CreateCubeVerticesEx(float minPos, float maxPos, float texCoordU, float texCoordV)
	{
		XMFLOAT2 uv0 = { 0.0f,	     0.0f };
		XMFLOAT2 uv1 = { texCoordU,  0.0f };
		XMFLOAT2 uv2 = { texCoordU,  texCoordV };
		XMFLOAT2 uv3 = { 0.0f,	     texCoordV };

		XMFLOAT3 backNormal = { 0.0f, 0.0f, 1.0f };
		XMFLOAT3 frontNormal = { 0.0f, 0.0f, -1.0f };
		XMFLOAT3 bottomNormal = { 0.0f, -1.0f, 0.0f };
		XMFLOAT3 topNormal = { 0.0f, 1.0f, 0.0f };
		XMFLOAT3 leftNormal = { -1.0f, 0.0f, 0.0f };
		XMFLOAT3 rightNormal = { 1.0f, 0.0f, 0.0f };

		// back
		XMFLOAT3 backPos0 = { maxPos, maxPos, maxPos };
		XMFLOAT3 backPos1 = { minPos, maxPos, maxPos };
		XMFLOAT3 backPos2 = { minPos, minPos, maxPos };
		XMFLOAT3 backPos3 = { maxPos, minPos, maxPos };

		auto backTS0 = GetTangentSpace(backPos0, backPos1, backPos2, uv0, uv1, uv2);
		auto backTS1 = GetTangentSpace(backPos0, backPos2, backPos3, uv0, uv2, uv3);

		// front
		XMFLOAT3 frontPos0 = { minPos, maxPos, minPos };
		XMFLOAT3 frontPos1 = { maxPos, maxPos, minPos };
		XMFLOAT3 frontPos2 = { maxPos, minPos, minPos };
		XMFLOAT3 frontPos3 = { minPos, minPos, minPos };

		auto frontTS0 = GetTangentSpace(frontPos0, frontPos1, frontPos2, uv0, uv1, uv2);
		auto frontTS1 = GetTangentSpace(frontPos0, frontPos2, frontPos3, uv0, uv2, uv3);

		// bottom
		XMFLOAT3 bottomPos0 = { maxPos, minPos, maxPos };
		XMFLOAT3 bottomPos1 = { minPos, minPos, maxPos };
		XMFLOAT3 bottomPos2 = { minPos, minPos, minPos };
		XMFLOAT3 bottomPos3 = { maxPos, minPos, minPos };

		auto bottomTS0 = GetTangentSpace(bottomPos0, bottomPos1, bottomPos2, uv0, uv1, uv2);
		auto bottomTS1 = GetTangentSpace(bottomPos0, bottomPos2, bottomPos3, uv0, uv2, uv3);

		// top
		XMFLOAT3 topPos0 = { minPos, maxPos, maxPos };
		XMFLOAT3 topPos1 = { maxPos, maxPos, maxPos };
		XMFLOAT3 topPos2 = { maxPos, maxPos, minPos };
		XMFLOAT3 topPos3 = { minPos, maxPos, minPos };

		auto topTS0 = GetTangentSpace(topPos0, topPos1, topPos2, uv0, uv1, uv2);
		auto topTS1 = GetTangentSpace(topPos0, topPos2, topPos3, uv0, uv2, uv3);

		// left
		XMFLOAT3 leftPos0 = { minPos, maxPos, maxPos };
		XMFLOAT3 leftPos1 = { minPos, maxPos, minPos };
		XMFLOAT3 leftPos2 = { minPos, minPos, minPos };
		XMFLOAT3 leftPos3 = { minPos, minPos, maxPos };

		auto leftTS0 = GetTangentSpace(leftPos0, leftPos1, leftPos2, uv0, uv1, uv2);
		auto leftTS1 = GetTangentSpace(leftPos0, leftPos2, leftPos3, uv0, uv2, uv3);

		// right
		XMFLOAT3 rightPos0 = { maxPos, maxPos, minPos };
		XMFLOAT3 rightPos1 = { maxPos, maxPos, maxPos };
		XMFLOAT3 rightPos2 = { maxPos, minPos, maxPos };
		XMFLOAT3 rightPos3 = { maxPos, minPos, minPos };

		auto rightTS0 = GetTangentSpace(rightPos0, rightPos1, rightPos2, uv0, uv1, uv2);
		auto rightTS1 = GetTangentSpace(rightPos0, rightPos2, rightPos3, uv0, uv2, uv3);

		return std::move(std::array<Vertex, 36>{
			// back
			backPos0, uv0, backNormal, backTS0.tangent, backTS0.bitangent,
				backPos1, uv1, backNormal, backTS0.tangent, backTS0.bitangent,
				backPos2, uv2, backNormal, backTS0.tangent, backTS0.bitangent,
				backPos2, uv2, backNormal, backTS1.tangent, backTS1.bitangent,
				backPos3, uv3, backNormal, backTS1.tangent, backTS1.bitangent,
				backPos0, uv0, backNormal, backTS1.tangent, backTS1.bitangent,

				// front
				frontPos0, uv0, frontNormal, frontTS0.tangent, frontTS0.bitangent,
				frontPos1, uv1, frontNormal, frontTS0.tangent, frontTS0.bitangent,
				frontPos2, uv2, frontNormal, frontTS0.tangent, frontTS0.bitangent,
				frontPos2, uv2, frontNormal, frontTS1.tangent, frontTS1.bitangent,
				frontPos3, uv3, frontNormal, frontTS1.tangent, frontTS1.bitangent,
				frontPos0, uv0, frontNormal, frontTS1.tangent, frontTS1.bitangent,

				// bottom
				bottomPos0, uv0, bottomNormal, bottomTS0.tangent, bottomTS0.bitangent,
				bottomPos1, uv1, bottomNormal, bottomTS0.tangent, bottomTS0.bitangent,
				bottomPos2, uv2, bottomNormal, bottomTS0.tangent, bottomTS0.bitangent,
				bottomPos2, uv2, bottomNormal, bottomTS1.tangent, bottomTS1.bitangent,
				bottomPos3, uv3, bottomNormal, bottomTS1.tangent, bottomTS1.bitangent,
				bottomPos0, uv0, bottomNormal, bottomTS1.tangent, bottomTS1.bitangent,

				// top
				topPos0, uv0, topNormal, topTS0.tangent, topTS0.bitangent,
				topPos1, uv1, topNormal, topTS0.tangent, topTS0.bitangent,
				topPos2, uv2, topNormal, topTS0.tangent, topTS0.bitangent,
				topPos2, uv2, topNormal, topTS1.tangent, topTS1.bitangent,
				topPos3, uv3, topNormal, topTS1.tangent, topTS1.bitangent,
				topPos0, uv0, topNormal, topTS1.tangent, topTS1.bitangent,

				// left
				leftPos0, uv0, leftNormal, leftTS0.tangent, leftTS0.bitangent,
				leftPos1, uv1, leftNormal, leftTS0.tangent, leftTS0.bitangent,
				leftPos2, uv2, leftNormal, leftTS0.tangent, leftTS0.bitangent,
				leftPos2, uv2, leftNormal, leftTS1.tangent, leftTS1.bitangent,
				leftPos3, uv3, leftNormal, leftTS1.tangent, leftTS1.bitangent,
				leftPos0, uv0, leftNormal, leftTS1.tangent, leftTS1.bitangent,

				// right
				rightPos0, uv0, rightNormal, rightTS0.tangent, rightTS0.bitangent,
				rightPos1, uv1, rightNormal, rightTS0.tangent, rightTS0.bitangent,
				rightPos2, uv2, rightNormal, rightTS0.tangent, rightTS0.bitangent,
				rightPos2, uv2, rightNormal, rightTS1.tangent, rightTS1.bitangent,
				rightPos3, uv3, rightNormal, rightTS1.tangent, rightTS1.bitangent,
				rightPos0, uv0, rightNormal, rightTS1.tangent, rightTS1.bitangent,
		});
	}

	std::array<Vertex, 6> CreatePlaneVerticesEx(float minPos, float maxPos, float texCoordU, float texCoordV)
	{
		XMFLOAT2 uv0 = { 0.0f,	     0.0f };
		XMFLOAT2 uv1 = { texCoordU,  0.0f };
		XMFLOAT2 uv2 = { texCoordU,  texCoordV };
		XMFLOAT2 uv3 = { 0.0f,	     texCoordV };

		XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

		XMFLOAT3 pos0 = { minPos, 0.0f, maxPos };
		XMFLOAT3 pos1 = { maxPos, 0.0f, maxPos };
		XMFLOAT3 pos2 = { maxPos, 0.0f, minPos };
		XMFLOAT3 pos3 = { minPos, 0.0f, minPos };

		auto ts0 = GetTangentSpace(pos0, pos1, pos2, uv0, uv1, uv2);
		auto ts1 = GetTangentSpace(pos0, pos2, pos3, uv0, uv2, uv3);

		return std::move(std::array<Vertex, 6>{
			pos0, uv0, normal, ts0.tangent, ts0.bitangent,
				pos1, uv1, normal, ts0.tangent, ts0.bitangent,
				pos2, uv2, normal, ts0.tangent, ts0.bitangent,
				pos2, uv2, normal, ts1.tangent, ts1.bitangent,
				pos3, uv3, normal, ts1.tangent, ts1.bitangent,
				pos0, uv0, normal, ts1.tangent, ts1.bitangent,
		});
	}

	std::vector<float> CreateCubeVertices(bool hasTexCoords, bool hasNormals, float minPos, float maxPos, float texCoordU, float texCoordV)
	{
		std::vector<float> vertices;
		if (hasTexCoords && hasNormals)
		{
			vertices =
			{
				//  back		             						  
				  maxPos, maxPos, maxPos,    0.0f,	     0.0f,           0.0f, 0.0f, 1.0f,
				  minPos, maxPos, maxPos,    texCoordU,  0.0f,           0.0f, 0.0f, 1.0f,
				  minPos, minPos, maxPos,    texCoordU,  texCoordV,	     0.0f, 0.0f, 1.0f,
				  maxPos, minPos, maxPos,    0.0f,	     texCoordV,	     0.0f, 0.0f, 1.0f,

				  // front
				minPos, maxPos, minPos,    0.0f,	     0.0f,          0.0f, 0.0f, -1.0f,
				maxPos, maxPos, minPos,    texCoordU,  0.0f,          0.0f, 0.0f, -1.0f,
				maxPos, minPos, minPos,    texCoordU,  texCoordV,     0.0f, 0.0f, -1.0f,
				minPos, minPos, minPos,    0.0f,	     texCoordV,     0.0f, 0.0f, -1.0f,

				// bottom			     	      
			  maxPos, minPos, maxPos,    0.0f,	     0.0f,          0.0f, -1.0f,  0.0,
			  minPos, minPos, maxPos,    texCoordU,  0.0f,          0.0f, -1.0f,  0.0,
			  minPos, minPos, minPos,    texCoordU,  texCoordV,     0.0f, -1.0f,  0.0,
			  maxPos, minPos, minPos,    0.0f,	     texCoordV,     0.0f, -1.0f,  0.0,

			  // top		      	     					  
			minPos, maxPos, maxPos,    0.0f,	     0.0f,           0.0f, 1.0f, 0.0f,
			maxPos, maxPos, maxPos,    texCoordU,  0.0f,           0.0f, 1.0f, 0.0f,
			maxPos, maxPos, minPos,    texCoordU,  texCoordV,      0.0f, 1.0f, 0.0f,
			minPos, maxPos, minPos,    0.0f,	     texCoordV,      0.0f, 1.0f, 0.0f,

			// left				     	     
		  minPos, maxPos, maxPos,    0.0f,	     0.0f,     	    -1.0f, 0.0f,  0.0,
		  minPos, maxPos, minPos,    texCoordU,  0.0f,     	    -1.0f, 0.0f,  0.0,
		  minPos, minPos, minPos,    texCoordU,  texCoordV,     -1.0f, 0.0f,  0.0,
		  minPos, minPos, maxPos,    0.0f,	     texCoordV,     -1.0f, 0.0f,  0.0,

		  // right			             
		maxPos, maxPos, minPos,    0.0f,	     0.0f,           1.0f, 0.0f, 0.0f,
		maxPos, maxPos, maxPos,    texCoordU,  0.0f,           1.0f, 0.0f, 0.0f,
		maxPos, minPos, maxPos,    texCoordU,  texCoordV,      1.0f, 0.0f, 0.0f,
		maxPos, minPos, minPos,    0.0f,	     texCoordV,      1.0f, 0.0f, 0.0f,
			};
		}
		else if (hasTexCoords && !hasNormals)
		{
			vertices =
			{
				//  back		             						  
				  maxPos, maxPos, maxPos,    0.0f,	     0.0f,
				  minPos, maxPos, maxPos,    texCoordU,  0.0f,
				  minPos, minPos, maxPos,    texCoordU,  texCoordV,
				  maxPos, minPos, maxPos,    0.0f,	     texCoordV,

				  // front
				minPos, maxPos, minPos,    0.0f,	     0.0f,
				maxPos, maxPos, minPos,    texCoordU,  0.0f,
				maxPos, minPos, minPos,    texCoordU,  texCoordV,
				minPos, minPos, minPos,    0.0f,	     texCoordV,

				// bottom			     	      					 
			  maxPos, minPos, maxPos,    0.0f,	     0.0f,
			  minPos, minPos, maxPos,    texCoordU,  0.0f,
			  minPos, minPos, minPos,    texCoordU,  texCoordV,
			  maxPos, minPos, minPos,    0.0f,	     texCoordV,

			  // top		      	     					  
			minPos, maxPos, maxPos,    0.0f,	     0.0f,
			maxPos, maxPos, maxPos,    texCoordU,  0.0f,
			maxPos, maxPos, minPos,    texCoordU,  texCoordV,
			minPos, maxPos, minPos,    0.0f,	     texCoordV,

			// left				     	     
		  minPos, maxPos, maxPos,    0.0f,	     0.0f,
		  minPos, maxPos, minPos,    texCoordU,  0.0f,
		  minPos, minPos, minPos,    texCoordU,  texCoordV,
		  minPos, minPos, maxPos,    0.0f,	     texCoordV,

		  // right			             
		maxPos, maxPos, minPos,    0.0f,	     0.0f,
		maxPos, maxPos, maxPos,    texCoordU,  0.0f,
		maxPos, minPos, maxPos,    texCoordU,  texCoordV,
		maxPos, minPos, minPos,    0.0f,	     texCoordV,
			};
		}
		else if (!hasTexCoords && hasNormals)
		{
			vertices =
			{
				//  back		             						  
				  maxPos, maxPos, maxPos,    0.0f, 0.0f, 1.0f,
				  minPos, maxPos, maxPos,    0.0f, 0.0f, 1.0f,
				  minPos, minPos, maxPos,    0.0f, 0.0f, 1.0f,
				  maxPos, minPos, maxPos,    0.0f, 0.0f, 1.0f,

				  // front
				minPos, maxPos, minPos,    0.0f, 0.0f, -1.0f,
				maxPos, maxPos, minPos,    0.0f, 0.0f, -1.0f,
				maxPos, minPos, minPos,    0.0f, 0.0f, -1.0f,
				minPos, minPos, minPos,    0.0f, 0.0f, -1.0f,

				// bottom			     	      
			  maxPos, minPos, maxPos,    0.0f, -1.0f,  0.0,
			  minPos, minPos, maxPos,    0.0f, -1.0f,  0.0,
			  minPos, minPos, minPos,    0.0f, -1.0f,  0.0,
			  maxPos, minPos, minPos,    0.0f, -1.0f,  0.0,

			  // top		      	     					  
			minPos, maxPos, maxPos,    0.0f, 1.0f, 0.0f,
			maxPos, maxPos, maxPos,    0.0f, 1.0f, 0.0f,
			maxPos, maxPos, minPos,    0.0f, 1.0f, 0.0f,
			minPos, maxPos, minPos,    0.0f, 1.0f, 0.0f,

			// left				     	     
		  minPos, maxPos, maxPos,   -1.0f, 0.0f,  0.0,
		  minPos, maxPos, minPos,   -1.0f, 0.0f,  0.0,
		  minPos, minPos, minPos,   -1.0f, 0.0f,  0.0,
		  minPos, minPos, maxPos,   -1.0f, 0.0f,  0.0,

		  // right			             
		maxPos, maxPos, minPos,    1.0f, 0.0f, 0.0f,
		maxPos, maxPos, maxPos,    1.0f, 0.0f, 0.0f,
		maxPos, minPos, maxPos,    1.0f, 0.0f, 0.0f,
		maxPos, minPos, minPos,    1.0f, 0.0f, 0.0f,
			};

		}
		else
		{
			vertices =
			{
				//  back		             						  
				  maxPos, maxPos, maxPos,
				  minPos, maxPos, maxPos,
				  minPos, minPos, maxPos,
				  maxPos, minPos, maxPos,

				  // front
				minPos, maxPos, minPos,
				maxPos, maxPos, minPos,
				maxPos, minPos, minPos,
				minPos, minPos, minPos,

				// bottom			     	      
			  maxPos, minPos, maxPos,
			  minPos, minPos, maxPos,
			  minPos, minPos, minPos,
			  maxPos, minPos, minPos,

			  // top		      	     					  
			minPos, maxPos, maxPos,
			maxPos, maxPos, maxPos,
			maxPos, maxPos, minPos,
			minPos, maxPos, minPos,

			// left				     	     
		  minPos, maxPos, maxPos,
		  minPos, maxPos, minPos,
		  minPos, minPos, minPos,
		  minPos, minPos, maxPos,

		  // right			             
		maxPos, maxPos, minPos,
		maxPos, maxPos, maxPos,
		maxPos, minPos, maxPos,
		maxPos, minPos, minPos,
			};
		}

		return vertices;
	}

	std::vector<float> CreatePlaneVertices(bool hasTexCoords, bool hasNormals, float minPos, float maxPos, float texCoordU, float texCoordV)
	{
		std::vector<float> vertices;

		if (hasTexCoords && hasNormals)
		{
			vertices =
			{
				  minPos, 0.0f, maxPos,   0.0f,      0.0f,       0.0f, 1.0f, 0.0f,
				  maxPos, 0.0f, maxPos,   texCoordU, 0.0f,       0.0f, 1.0f, 0.0f,
				  maxPos, 0.0f, minPos,   texCoordU, texCoordV,  0.0f, 1.0f, 0.0f,
				  minPos, 0.0f, minPos,   0.0f,      texCoordV,  0.0f, 1.0f, 0.0f,
			};
		}
		else if (hasTexCoords && !hasNormals)
		{
			vertices =
			{
				  minPos, 0.0f, maxPos,   0.0f,      0.0f,
				  maxPos, 0.0f, maxPos,   texCoordU, 0.0f,
				  maxPos, 0.0f, minPos,   texCoordU, texCoordV,
				  minPos, 0.0f, minPos,   0.0f,      texCoordV,
			};
		}
		else if (!hasTexCoords && hasNormals)
		{
			vertices =
			{
				  minPos, 0.0f, maxPos,   0.0f, 1.0f, 0.0f,
				  maxPos, 0.0f, maxPos,   0.0f, 1.0f, 0.0f,
				  maxPos, 0.0f, minPos,   0.0f, 1.0f, 0.0f,
				  minPos, 0.0f, minPos,   0.0f, 1.0f, 0.0f,
			};
		}
		else
		{
			vertices =
			{
				  minPos, 0.0f, maxPos,
				  maxPos, 0.0f, maxPos,
				  maxPos, 0.0f, minPos,
				  minPos, 0.0f, minPos,
			};
		}

		return vertices;
	}

	std::array<uint32_t, 36> CreateCubeIndicesEx()
	{
		std::array<uint32_t, 36> indices;
		for (uint32_t i = 0; i < indices.size(); i++)
			indices[i] = i;

		return indices;
	}

	std::array<uint32_t, 6> CreatePlaneIndicesEx()
	{
		std::array<uint32_t, 6> indices;
		for (uint32_t i = 0; i < indices.size(); i++)
			indices[i] = i;

		return indices;
	}

	std::array<uint32_t, 36> CreateCubeIndices()
	{
		return std::move(std::array<uint32_t, 36>
		{
			// back
			0, 1, 2,
				2, 3, 0,

				// front
				4, 5, 6,
				6, 7, 4,

				// left
				8, 9, 10,
				10, 11, 8,

				// right
				12, 13, 14,
				14, 15, 12,

				// bottom
				16, 17, 18,
				18, 19, 16,

				// top
				20, 21, 22,
				22, 23, 20
		});
	}

	std::array<uint32_t, 6> CreatePlaneIndices()
	{
		return std::move(std::array<uint32_t, 6>
		{
			0, 1, 2,
				2, 3, 0,
		});
	}

}
