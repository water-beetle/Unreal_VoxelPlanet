// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "VoxelChunk.h"
#include "Components/DynamicMeshComponent.h"
#include "VoxelMeshComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VOXELWORLD_API UVoxelMeshComponent : public UDynamicMeshComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UVoxelMeshComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY()
	UVoxelChunk* OwningChunk = nullptr;
};
