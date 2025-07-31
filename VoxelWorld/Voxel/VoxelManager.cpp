// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelManager.h"

#include "VoxelChunk.h"
#include "VoxelStructs.h"


// Sets default values
AVoxelManager::AVoxelManager()
	: CellSize(100), CellCount(10), ChunkCount(10), LOD(1)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AVoxelManager::BeginPlay()
{
	Super::BeginPlay();

	const double StartTime = FPlatformTime::Seconds();
	
	for (int32 x = 0; x < ChunkCount; ++x)
	{
		for (int32 y = 0; y < ChunkCount; ++y)
		{
			for (int32 z = 0; z < ChunkCount; ++z)
			{
				UVoxelChunk* Chunk = NewObject<UVoxelChunk>(this);
				Chunk->RegisterComponent();
				Chunk->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
				
				ChunkSettingInfo ChunkInfo{FIntVector (x,y,z), CellSize, CellCount, ChunkCount, 1};
				Chunk->Build(ChunkInfo);
				Chunk->SetVoxelManager(this);
				RegisterChunk(FIntVector(x,y,z), Chunk);
			}
		}
	}
	
	double ElapsedTime = (FPlatformTime::Seconds() - StartTime) * 1000.0;  // 밀리초 단위
	UE_LOG(LogTemp, Warning, TEXT("Time : %.2f ms"), ElapsedTime);
}

// Called every frame
void AVoxelManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AVoxelManager::RegisterChunk(const FIntVector& Index, UVoxelChunk* Chunk)
{
	ChunkMap.Add(Index, Chunk);
}

UVoxelChunk* AVoxelManager::GetChunk(const FIntVector& Index)
{
	if (UVoxelChunk** Ptr = ChunkMap.Find(Index))
	{
		return *Ptr;
	}
	return nullptr;
}

