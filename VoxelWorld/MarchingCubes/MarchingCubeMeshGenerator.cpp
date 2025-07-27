#include "MarchingCubeMeshGenerator.h"

#include "MarchingCubeLookupTable.h"
#include "VoxelWorld/Voxel/VoxelStructs.h"

static const int EdgeCorners[12][2] =
{
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

FVoxelMeshData MarchingCubeMeshGenerator::GenerateMesh(ChunkSettingInfo& chunkSettingInfo, TArray<FVertexDensity> VertexDensityData)
{
	FVoxelMeshData VoxelMeshData;
	
	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;

	// 현재 Chunk의 중심 좌표 -> Voxel 중심을 원점으로 이동 후, Chunk 중심좌표 계산
	const FVector ChunkPos = (chunkSettingInfo.ChunkIndex + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);

	if (chunkSettingInfo.LOD <= 0)
		chunkSettingInfo.LOD = 1;
	
	const int CellsPerAxis = FMath::CeilToInt(static_cast<float>(chunkSettingInfo.CellCount) / chunkSettingInfo.LOD);
	const int EstimatedCubeCount = CellsPerAxis * CellsPerAxis * CellsPerAxis;
	constexpr int MaxVertsPerCube = 15;
	VoxelMeshData.Vertices.Reserve(EstimatedCubeCount * MaxVertsPerCube);
	VoxelMeshData.Normals.Reserve(EstimatedCubeCount * MaxVertsPerCube);
	VoxelMeshData.Triangles.Reserve(EstimatedCubeCount * MaxVertsPerCube);
	
	for (int z=0; z < chunkSettingInfo.CellCount; z += chunkSettingInfo.LOD)
	{
		for (int y=0; y < chunkSettingInfo.CellCount; y += chunkSettingInfo.LOD)
		{
			for (int x=0; x < chunkSettingInfo.CellCount; x += chunkSettingInfo.LOD)
			{
				FVector CubeCorner[8];
				SetCubeCorner(x, y, z, CubeCorner);
				float CubeCornerDensity[8];
			
				for (int i = 0; i < 8; i++)
				{
					CubeCornerDensity[i] = VertexDensityData[GetIndex(
						CubeCorner[i].X,CubeCorner[i].Y,CubeCorner[i].Z,chunkSettingInfo.CellCount)].Density;
					// 중심을 원점으로 이동 후, ChunkIndex 만큼 이동시키기
					CubeCorner[i] = CubeCorner[i] * chunkSettingInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos; 
				}

				int cubeIndex = 0;
                for (int i = 0; i < 8; i++)
                {
                    if (CubeCornerDensity[i] < 0.0f)
                        cubeIndex |= (1 << i);
                }

                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] == 0)
                    continue;

                FVector VertexList[12];

				for (int e = 0; e < 12; ++e)
				{
					if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & (1 << e))
					{
						int c0 = EdgeCorners[e][0];
						int c1 = EdgeCorners[e][1];
						VertexList[e] = InterpolateVertex(
							CubeCorner[c0], CubeCorner[c1],
							CubeCornerDensity[c0], CubeCornerDensity[c1]);
					}
				}

				for (int i = 0; MarchingCubeLooupTable::TriTable[cubeIndex][i] != -1; i += 3)
				{
					int idx0 = MarchingCubeLooupTable::TriTable[cubeIndex][i];
					int idx1 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 1];
					int idx2 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 2];

					FVector v0 = VertexList[idx0];
					FVector v1 = VertexList[idx1];
					FVector v2 = VertexList[idx2];

					// FVector Edge1 = v1 - v0;
					// FVector Edge2 = v2 - v0;
					// FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();
					//
					// VoxelMeshData.Normals.Add(Normal);
					// VoxelMeshData.Normals.Add(Normal);
					// VoxelMeshData.Normals.Add(Normal);
					
					int vertIndex = VoxelMeshData.Vertices.Num();
					VoxelMeshData.Vertices.Add(v0);
					VoxelMeshData.Vertices.Add(v1);
					VoxelMeshData.Vertices.Add(v2);

					VoxelMeshData.Triangles.Add(vertIndex + 2);
					VoxelMeshData.Triangles.Add(vertIndex + 1);
					VoxelMeshData.Triangles.Add(vertIndex);
				}
			}
		}
	}

	return VoxelMeshData;
}

FVector MarchingCubeMeshGenerator::InterpolateVertex(const FVector& p1, const FVector& p2, float valp1, float valp2)
{
	float t = (0.0f - valp1) / (valp2 - valp1);
	return FMath::Lerp(p1, p2, t);
}

void MarchingCubeMeshGenerator::SetCubeCorner(int X, int Y, int Z, FVector* V)
{
	V[4].X = V[5].X = V[0].X = V[1].X = X;
	V[7].X = V[6].X = V[3].X = V[2].X = X + 1;

	V[0].Y = V[1].Y = V[2].Y = V[3].Y = Y;
	V[4].Y = V[5].Y = V[6].Y = V[7].Y = Y + 1;

	V[0].Z = V[3].Z = V[4].Z = V[7].Z = Z;
	V[1].Z = V[2].Z = V[5].Z = V[6].Z = Z + 1;
}

int MarchingCubeMeshGenerator::GetIndex(const int x, const int y, const int z, const int CellCount)
{
	return x + y * (CellCount + 1) + z * (CellCount + 1) * (CellCount + 1);
}
