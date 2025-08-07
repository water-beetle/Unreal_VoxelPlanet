// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelStructs.h"
#include "VoxelChunk.generated.h"

namespace UE::Geometry
{
	class FDynamicMesh3;
}

class UDynamicMeshComponent;
class UVoxelMeshComponent;
class AVoxelManager;

USTRUCT()
struct FChunkMeshMappings
{
	GENERATED_BODY()

	TArray<TSet<int32>> VertexToTriangles;
	TMap<FIntVector, TSet<int32>> CellToTriangles;
	TMap<FIntVector, TArray<int32>> CellToVertices;
	bool bIsLoaded = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VOXELWORLD_API UVoxelChunk : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVoxelChunk();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	void GenerateMeshComponent();

public:

	void Build(const ChunkSettingInfo& InSettings);
	void ApplyBrush(const FVector& HitLocation);
	void ApplyBrushInternal(const FVector& HitLocation); // 자신의 Chunk에만 Brush 적용

	//void LoadMappings();
	//void UnloadMappings();
	//bool IsNearCamera(float Radius = 2000.f) const;

	FIntVector GetCellFromTriangle(const UE::Geometry::FDynamicMesh3& Mesh, int32 V0, int32 V1, int32 V2) const;
	
	void UpdateMesh(const FVoxelMeshData& VoxelMeshData);


	FORCEINLINE void SetVoxelManager(AVoxelManager* VoxelManager) { OwningManager = VoxelManager; }
	FORCEINLINE AVoxelManager* GetManager() const { return OwningManager; }
		
	UPROPERTY()
	UVoxelMeshComponent* MeshComponent;
	UPROPERTY()
	TArray<FVertexDensity> VertexDensityData;

private:
	void CalculateVertexDensity();
	static float SampleDensity(const FVector& Pos, int Radius);
	static int GetIndex(int x, int y, int z, int CellCount);
	void UpdateMeshPartialCells(const TSet<FIntVector>& ModifiedCells);

	FVoxelMeshData CachedMeshData;
	FChunkMeshMappings Mappings;
	ChunkSettingInfo chunkSettingInfo;
	int ChunkSize = 0;
	int VoxelSize = 0;
	FVector ChunkPos;
	
	int BrushRadius = 5;
	
	UPROPERTY()
	AVoxelManager* OwningManager = nullptr;

	
};
