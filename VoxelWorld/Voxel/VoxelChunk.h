// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelStructs.h"
#include "VoxelChunk.generated.h"

class UDynamicMeshComponent;
class UVoxelMeshComponent;
class AVoxelManager;

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

	void Build(ChunkSettingInfo& _chunkSettingInfo);
	void UpdateMesh();

	void ApplyBrush(const FVector& HitLocation);

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
	void ApplyBrushInternal(const FVector& HitLocation); // 자신의 Chunk에만 Brush 적용
	
	ChunkSettingInfo chunkSettingInfo;
	int BrushRadius = 200;
	
	UPROPERTY()
	AVoxelManager* OwningManager = nullptr;

	
};
