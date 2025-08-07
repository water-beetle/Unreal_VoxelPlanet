#pragma once
#include "VoxelWorld/Voxel/VoxelStructs.h"

struct FVoxelMeshData;
struct ChunkSettingInfo;

class MarchingCubeMeshGenerator
{
public:
	static FVoxelMeshData GenerateMesh(ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData);
	static FVoxelMeshData GenerateCellMesh(const ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData, const FIntVector& Cell);

private:
	static FVoxelMeshData ProcessCell(const ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData, const FIntVector& Cell);

	static FVector InterpolateVertex(const FVector& p1, const FVector& p2, float valp1, float valp2);
	static void SetCubeCorner(int X, int Y, int Z, FVector* V);
	static int GetIndex(int x, int y, int z, int CellCount);
};
