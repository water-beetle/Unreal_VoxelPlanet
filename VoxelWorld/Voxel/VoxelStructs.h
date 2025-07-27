#pragma once
#include "VoxelStructs.generated.h"

struct FVoxelMeshData
{
	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FColor> Colors;
	TArray<int> Triangles;
};

struct ChunkSettingInfo
{
	FVector ChunkIndex;
	int CellSize;
	int CellCount;
	int ChunkCount;
	int LOD;
};

USTRUCT(Blueprintable)
struct FVertexDensity
{
	GENERATED_BODY()
	FVertexDensity()
		: Density(0.0f), Id(0) {};
	FVertexDensity(const float Density, const int Id)
	{
		this->Density = Density;
		this->Id = Id;
	};
    
	UPROPERTY(BlueprintReadOnly)
	float Density;
	UPROPERTY(BlueprintReadOnly)
	int Id;
};
	

