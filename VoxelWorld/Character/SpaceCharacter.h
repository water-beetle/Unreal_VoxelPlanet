// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SpaceCharacter.generated.h"

UCLASS()
class VOXELWORLD_API ASpaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASpaceCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	APlayerController* CachedPC;
};
