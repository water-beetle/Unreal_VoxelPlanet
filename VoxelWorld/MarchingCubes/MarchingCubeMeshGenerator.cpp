#include "MarchingCubeMeshGenerator.h"

#include "MarchingCubeLookupTable.h"
#include "VoxelWorld/Voxel/VoxelStructs.h"

FVoxelMeshData MarchingCubeMeshGenerator::GenerateMesh(FChunkInfo& ChunkInfo)
{
	FVoxelMeshData VoxelMeshData;
	
	const float ChunkSize = ChunkInfo.CellSize * ChunkInfo.CellCount;
	const float VoxelSize = ChunkSize * ChunkInfo.ChunkCount;

	// 현재 Chunk의 중심 좌표 -> Voxel 중심을 원점으로 이동 후, Chunk 중심좌표 계산
	const FVector ChunkPos = (ChunkInfo.ChunkIndex + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);

	if (ChunkInfo.LOD <= 0)
		ChunkInfo.LOD = 1;
	
	for (int x=0; x < ChunkInfo.CellCount; x += ChunkInfo.LOD)
	{
		for (int y=0; y < ChunkInfo.CellCount; y += ChunkInfo.LOD)
		{
			for (int z=0; z < ChunkInfo.CellCount; z += ChunkInfo.LOD)
			{
				FVector CubeCorner[8];
				float CubeCornerDensity[8];

				for (int i = 0; i < 8; i++)
				{
					int ix = x + ((i & 1) ? 1 : 0);
					int iy = y + ((i & 2) ? 1 : 0);
					int iz = z + ((i & 4) ? 1 : 0);

					// 중심을 원점으로 이동 후, ChunkIndex 만큼 이동시키기
					FVector WorldPos = FVector(ix, iy, iz) * ChunkInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos; 
					CubeCorner[i] = WorldPos;
					CubeCornerDensity[i] = SampleDensity(WorldPos, VoxelSize * 0.5f);
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

					VoxelMeshData.Triangles.Add(vertIndex);
					VoxelMeshData.Triangles.Add(vertIndex + 1);
					VoxelMeshData.Triangles.Add(vertIndex + 2);
				}
			}
		}
	}

	return VoxelMeshData;
}

float MarchingCubeMeshGenerator::SampleDensity(const FVector& Pos, int Radius)
{	
	float Distance = Pos.Size();
	float Density = Radius - Distance;
	
	//float Noise = FMath::PerlinNoise3D(Position * NoiseScale) * 100.0f;
	return Density;
}

FVector MarchingCubeMeshGenerator::InterpolateVertex(const FVector& p1, const FVector& p2, float valp1, float valp2)
{
	float t = (0.0f - valp1) / (valp2 - valp1);
	return FMath::Lerp(p1, p2, t);
}
