#include "MarchingCubeMeshGenerator.h"

#include "MarchingCubeLookupTable.h"
#include "VoxelWorld/Voxel/VoxelStructs.h"

FVoxelMeshData MarchingCubeMeshGenerator::GenerateMesh(ChunkSettingInfo& chunkSettingInfo, TArray<FVertexDensity> VertexDensityData)
{
	FVoxelMeshData VoxelMeshData;
	
	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;

	// 현재 Chunk의 중심 좌표 -> Voxel 중심을 원점으로 이동 후, Chunk 중심좌표 계산
	const FVector ChunkPos = (chunkSettingInfo.ChunkIndex + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);

	if (chunkSettingInfo.LOD <= 0)
		chunkSettingInfo.LOD = 1;
	
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

                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 1)
                    VertexList[0] = InterpolateVertex(CubeCorner[0], CubeCorner[1], CubeCornerDensity[0], CubeCornerDensity[1]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 2)
                    VertexList[1] = InterpolateVertex(CubeCorner[1], CubeCorner[2], CubeCornerDensity[1], CubeCornerDensity[2]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 4)
                    VertexList[2] = InterpolateVertex(CubeCorner[2], CubeCorner[3], CubeCornerDensity[2], CubeCornerDensity[3]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 8)
                    VertexList[3] = InterpolateVertex(CubeCorner[3], CubeCorner[0], CubeCornerDensity[3], CubeCornerDensity[0]);

                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 16)
                    VertexList[4] = InterpolateVertex(CubeCorner[4], CubeCorner[5], CubeCornerDensity[4], CubeCornerDensity[5]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 32)
                    VertexList[5] = InterpolateVertex(CubeCorner[5], CubeCorner[6], CubeCornerDensity[5], CubeCornerDensity[6]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 64)
                    VertexList[6] = InterpolateVertex(CubeCorner[6], CubeCorner[7], CubeCornerDensity[6], CubeCornerDensity[7]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 128)
                    VertexList[7] = InterpolateVertex(CubeCorner[7], CubeCorner[4], CubeCornerDensity[7], CubeCornerDensity[4]);

                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 256)
                    VertexList[8] = InterpolateVertex(CubeCorner[0], CubeCorner[4], CubeCornerDensity[0], CubeCornerDensity[4]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 512)
                    VertexList[9] = InterpolateVertex(CubeCorner[1], CubeCorner[5], CubeCornerDensity[1], CubeCornerDensity[5]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 1024)
                    VertexList[10] = InterpolateVertex(CubeCorner[2], CubeCorner[6], CubeCornerDensity[2], CubeCornerDensity[6]);
                if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & 2048)
                	VertexList[11] = InterpolateVertex(CubeCorner[3], CubeCorner[7], CubeCornerDensity[3], CubeCornerDensity[7]);

				for (int i = 0; MarchingCubeLooupTable::TriTable[cubeIndex][i] != -1; i += 3)
				{
					int idx0 = MarchingCubeLooupTable::TriTable[cubeIndex][i];
					int idx1 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 1];
					int idx2 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 2];

					FVector v0 = VertexList[idx0];
					FVector v1 = VertexList[idx1];
					FVector v2 = VertexList[idx2];

					FVector Edge1 = v1 - v0;
					FVector Edge2 = v2 - v0;
					FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

					VoxelMeshData.Normals.Add(Normal);
					VoxelMeshData.Normals.Add(Normal);
					VoxelMeshData.Normals.Add(Normal);
					
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
	return x + y * CellCount + z * CellCount * CellCount;
}
