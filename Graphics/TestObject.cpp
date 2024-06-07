#include "pch.h"
#include "TestObject.h"
TestObject::TestObject(ComPtr<ID3D11Device> _device)
{
	m_pNode = std::make_shared<StaticNode>();
	m_pNode->m_Meshs.push_back(StaticMesh());
	m_pNode->m_Meshs.back().m_Vertex.resize(24);
	auto& boxVertices = m_pNode->m_Meshs.back().m_Vertex;

	boxVertices[0] = { {-1.0f, 1.0f, -1.0f},	{1.0f, 0.0f,0}, {0.f, 1.f, 0.f} };
	boxVertices[1] = { {1.0f, 1.0f, -1.0f},		{0.0f, 0.0f,0}, {0.f, 1.f, 0.f} };
	boxVertices[2] = { {1.0f, 1.0f, 1.0f},		{0.0f, 1.0f,0}, {0.f, 1.f, 0.f} };
	boxVertices[3] = { {-1.0f, 1.0f, 1.0f},		{1.0f, 1.0f,0}, {0.f, 1.f, 0.f} };

	boxVertices[4] = { {-1.0f, -1.0f, -1.0f},	{0.0f, 0.0f,0}, {0.f, -1.f, 0.f} };
	boxVertices[5] = { {1.0f, -1.0f, -1.0f},	{1.0f, 0.0f,0}, {0.f, -1.f, 0.f} };
	boxVertices[6] = { {1.0f, -1.0f, 1.0f},		{1.0f, 1.0f,0}, {0.f, -1.f, 0.f} };
	boxVertices[7] = { {-1.0f, -1.0f, 1.0f},	{0.0f, 1.0f,0}, {0.f, -1.f, 0.f} };

	boxVertices[8] = { {-1.0f, -1.0f, 1.0f},	{0.0f,1.0f,0}, {-1.f, 0.f, 0.f} };
	boxVertices[9] = { {-1.0f, -1.0f, -1.0f},	{1.0f,1.0f,0}, {-1.f, 0.f, 0.f} };
	boxVertices[10] = { {-1.0f, 1.0f, -1.0f},	{1.0f,0.0f,0}, {-1.f, 0.f, 0.f} };
	boxVertices[11] = { {-1.0f, 1.0f, 1.0f},	{0.0f,0.0f,0}, {-1.f, 0.f, 0.f} };

	boxVertices[12] = { {1.0f, -1.0f, 1.0f},	{1.0f,1.0f,0}, {1.f, 0.f, 0.f} };
	boxVertices[13] = { {1.0f, -1.0f, -1.0f},	{0.0f,1.0f,0}, {1.f, 0.f, 0.f} };
	boxVertices[14] = { {1.0f, 1.0f, -1.0f},	{0.0f,0.0f,0}, {1.f, 0.f, 0.f} };
	boxVertices[15] = { {1.0f, 1.0f, 1.0f},		{1.0f,0.0f,0}, {1.f, 0.f, 0.f} };

	boxVertices[16] = { {-1.0f, -1.0f, -1.0f},	{0.0f,1.0f,0}, {0.f, 0.f, -1.0f} };
	boxVertices[17] = { {1.0f, -1.0f, -1.0f},	{1.0f,1.0f,0}, {0.f, 0.f, -1.0f} };
	boxVertices[18] = { {1.0f, 1.0f, -1.0f},	{1.0f,0.0f,0}, {0.f, 0.f, -1.0f} };
	boxVertices[19] = { {-1.0f, 1.0f, -1.0f},	{0.0f,0.0f,0}, {0.f, 0.f, -1.0f} };

	boxVertices[20] = { {-1.0f, -1.0f, 1.0f},	{1.0f,1.0f,0}, {0.f, 0.f, 1.0f} };
	boxVertices[21] = { {1.0f, -1.0f, 1.0f},	{0.0f,1.0f,0}, {0.f, 0.f, 1.0f} };
	boxVertices[22] = { {1.0f, 1.0f, 1.0f},		{0.0f,0.0f,0}, {0.f, 0.f, 1.0f} };
	boxVertices[23] = { {-1.0f, 1.0f, 1.0f},	{1.0f,0.0f,0}, {0.f, 0.f, 1.0f} };

	boxIndices[0] = 3; boxIndices[1] = 1; boxIndices[2] = 0;
	boxIndices[3] = 2; boxIndices[4] = 1; boxIndices[5] = 3;

	boxIndices[6] = 6; boxIndices[7] = 4; boxIndices[8] = 5;
	boxIndices[9] = 7; boxIndices[10] = 4; boxIndices[11] = 6;

	boxIndices[12] = 11; boxIndices[13] = 9; boxIndices[14] = 8;
	boxIndices[15] = 10; boxIndices[16] = 9; boxIndices[17] = 11;

	boxIndices[18] = 14; boxIndices[19] = 12; boxIndices[20] = 13;
	boxIndices[21] = 15; boxIndices[22] = 12; boxIndices[23] = 14;

	boxIndices[24] = 19; boxIndices[25] = 17; boxIndices[26] = 16;
	boxIndices[27] = 18; boxIndices[28] = 17; boxIndices[29] = 19;

	boxIndices[30] = 22; boxIndices[31] = 20; boxIndices[32] = 21;
	boxIndices[33] = 23; boxIndices[34] = 20; boxIndices[35] = 22;

	auto& Indices = m_pNode->m_Meshs.back().m_Indices;


	for (UINT i = 0; i < 36; ++i)
	{
		Indices.push_back(boxIndices[i]);
	}

	SimpleMath::Vector3 dtUV1, dtUV2;
	SimpleMath::Vector3 e0, e1, tan, bitan;
	float tx, ty, tz;
	float bx, by, bz;

	float dt, invert;

	for (int i = 0; i < 36; i += 3)
	{
		dtUV1 = boxVertices[Indices[i + 1]].m_UVW - boxVertices[Indices[i]].m_UVW;
		dtUV2 = boxVertices[Indices[i + 2]].m_UVW - boxVertices[Indices[i]].m_UVW;
		e0 = boxVertices[Indices[i + 1]].m_Pos - boxVertices[Indices[i]].m_Pos;
		e1 = boxVertices[Indices[i + 2]].m_Pos - boxVertices[Indices[i]].m_Pos;

		dt = ((dtUV1.x * dtUV2.y) - (dtUV1.y * dtUV2.x));

		if (dt == 0)
		{
			continue;
		}
		invert = 1 / dt;
		tan.x = invert * ((dtUV2.y * e0.x) + (-dtUV1.y * e1.x));
		tan.y = invert * ((dtUV2.y * e0.y) + (-dtUV1.y * e1.y));
		tan.z = invert * ((dtUV2.y * e0.z) + (-dtUV1.y * e1.z));

		bitan.x = invert * ((-dtUV2.x * e0.x) + (dtUV1.x * e1.x));
		bitan.y = invert * ((-dtUV2.x * e0.y) + (dtUV1.x * e1.y));
		bitan.z = invert * ((-dtUV2.x * e0.z) + (dtUV1.x * e1.z));
		tan.Normalize();
		bitan.Normalize();
		for (int j = 0; j < 3; j++)
		{
			boxVertices[Indices[i + j]].m_Tangent = tan;
		}
	}

	/*m_pNode->m_Meshs.back().CreateIndexBuffer(_device, Indices);
	m_pNode->m_Meshs.back().CreateVertexBuffer(_device, boxVertices);*/
}