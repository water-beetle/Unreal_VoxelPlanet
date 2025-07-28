// Fill out your copyright notice in the Description page of Project Settings.


#include "SpaceCharacter.h"


// Sets default values
ASpaceCharacter::ASpaceCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASpaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	CachedPC = GetWorld()->GetFirstPlayerController();
}

// Called every frame
void ASpaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CachedPC)
		return;

	FHitResult HitResult;
	if (CachedPC->GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, HitResult))
	{
		FVector HitLocation = HitResult.ImpactPoint;
		// 디버그 확인
		DrawDebugSphere(GetWorld(), HitLocation, 10.f, 12, FColor::Red, false, -1.f, 0, 1.f);
	}
}

