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

public:
	void GenerateMesh(FChunkInfo& ChunkInfo);

	UPROPERTY()
	UDynamicMeshComponent* MeshComponent;

	int UnitNum = 20;
	int UnitSize = 100;
	
};
