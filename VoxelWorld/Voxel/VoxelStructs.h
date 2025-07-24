#pragma once

struct FVoxelMeshData
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<int> Triangles;
};

struct FChunkInfo
{
	FVector ChunkIndex;
	int CellSize;
	int CellCount;
	int ChunkCount;
	int LOD;
};
	

