// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TestingController.generated.h"

/**
 * 
 */
UCLASS()
class VOXELWORLD_API ATestingController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

public:
	void StartEditing();
	void StopEditing();

	void ApplyBrush(const FVector& Location);
	
	UPROPERTY(EditAnywhere)
	float EditsPerSecond = 10.f;
	UPROPERTY(EditAnywhere)
	bool bIsEditing = false;

	float LastEditTime = 0.f;
};
