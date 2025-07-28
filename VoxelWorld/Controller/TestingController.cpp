// Fill out your copyright notice in the Description page of Project Settings.


#include "TestingController.h"

#include "VoxelWorld/Voxel/VoxelChunk.h"
#include "VoxelWorld/Voxel/VoxelMeshComponent.h"

void ATestingController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}

void ATestingController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	FHitResult HitResult;
	if (GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, HitResult))
	{
		FVector HitLocation = HitResult.ImpactPoint;
		
		const float CurrentTime = GetWorld()->GetTimeSeconds();
		if (CurrentTime > LastEditTime + (1.0f / EditsPerSecond))
		{
			LastEditTime = CurrentTime;
			
			if (bIsEditing)
			{
				if (UVoxelMeshComponent* MeshComp = Cast<UVoxelMeshComponent>(HitResult.GetComponent()))
				{
					if (UVoxelChunk* Chunk = MeshComp->OwningChunk)
					{
						Chunk->ApplyBrush(HitLocation);
					}
				}
			}
		}
		DrawDebugSphere(GetWorld(), HitLocation, 10.f, 12, FColor::Green, false, 0.05f);
	}
}

void ATestingController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATestingController::StartEditing);
	InputComponent->BindKey(EKeys::LeftMouseButton, IE_Released, this, &ATestingController::StopEditing);
}

void ATestingController::StartEditing()
{
	bIsEditing = true;
}

void ATestingController::StopEditing()
{
	bIsEditing = false;
}

void ATestingController::ApplyBrush(const FVector& Location)
{
}
