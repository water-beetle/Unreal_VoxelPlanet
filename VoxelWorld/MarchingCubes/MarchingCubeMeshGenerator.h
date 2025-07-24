#pragma once

struct FVoxelMeshData;
struct FChunkInfo;

class MarchingCubeMeshGenerator
{
public:
	static FVoxelMeshData GenerateMesh(FChunkInfo& ChunkInfo);
	static float SampleDensity(const FVector& Pos, int Radius);
	static FVector InterpolateVertex(const FVector& p1, const FVector& p2, float valp1, float valp2);
};
