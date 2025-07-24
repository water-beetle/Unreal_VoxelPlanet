// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "VoxelManager.generated.h"

UCLASS()
class VOXELWORLD_API AVoxelManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVoxelManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere)
	int CellSize;
	UPROPERTY(EditAnywhere)
	int CellCount;
	UPROPERTY(EditAnywhere)
	int ChunkCount;

	int LOD;
	
};
