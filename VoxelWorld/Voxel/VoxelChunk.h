// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelStructs.h"
#include "VoxelChunk.generated.h"

class UDynamicMeshComponent;

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

	void Sculpt();
	
	UPROPERTY()
	UDynamicMeshComponent* MeshComponent;
	UPROPERTY()
	TArray<FVertexDensity> VertexDensityData;

private:
	void CalculateVertexDensity();
	static float SampleDensity(const FVector& Pos, int Radius);
	static int GetIndex(int x, int y, int z, int CellCount);
	
	ChunkSettingInfo chunkSettingInfo;

	
};
