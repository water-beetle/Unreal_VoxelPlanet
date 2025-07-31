// Fill out your copyright notice in the Description page of Project Settings.

#include "VoxelChunk.h"

#include "VoxelManager.h"
#include "VoxelMeshComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/MeshNormals.h"
#include "VoxelWorld/MarchingCubes/MarchingCubeMeshGenerator.h"

using namespace UE::Geometry;

// Sets default values for this component's properties
UVoxelChunk::UVoxelChunk()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}


// Called when the game starts
void UVoxelChunk::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UVoxelChunk::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}



void UVoxelChunk::Build(ChunkSettingInfo& _chunkSettingInfo)
{
	// 클래스 변수 값 업데이트
	chunkSettingInfo = _chunkSettingInfo;

	// Marching Cube에서 사용할 vertex의 desntiy 값 초기화
	CalculateVertexDensity();
	
	// Dynamic Mesh Component 생성 및 등록
	GenerateMeshComponent();

	// Marching Cube를 사용한 Mesh 업데이트
	UpdateMesh();
}

void UVoxelChunk::GenerateMeshComponent()
{
	MeshComponent = NewObject<UVoxelMeshComponent>(GetOwner());
	MeshComponent->RegisterComponent();
	MeshComponent->AttachToComponent(GetOwner()->GetRootComponent(),	FAttachmentTransformRules::KeepRelativeTransform);
	MeshComponent->SetComplexAsSimpleCollisionEnabled(true, true);
	MeshComponent->bUseAsyncCooking = true;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->GetDynamicMesh()->Reset();
	MeshComponent->OwningChunk = this;

	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;
	const FVector ChunkPos = (FVector(chunkSettingInfo.ChunkIndex) + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);

	// Chunk의 위치를 월드 좌표계에서 이동
	MeshComponent->SetRelativeLocation(ChunkPos);
}

void UVoxelChunk::UpdateMesh()
{
	if (VertexDensityData.Num() != (chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1))
		return;
	
	FVoxelMeshData VoxelMeshData = MarchingCubeMeshGenerator::GenerateMesh(chunkSettingInfo, VertexDensityData);
	
	FDynamicMesh3 Mesh;
	Mesh.EnableVertexNormals(FVector3f());
	TArray<int32> VIDs;
	
	for (int i = 0; i < VoxelMeshData.Vertices.Num(); i++)
	{
		int32 ID = Mesh.AppendVertex(VoxelMeshData.Vertices[i]);
		VIDs.Add(ID);
		//Mesh.SetVertexNormal(ID, FVector3f(VoxelMeshData.Normals[i]));
	}

	for (int i = 0; i < VoxelMeshData.Triangles.Num(); i += 3)
	{
		const int T0 = VIDs[VoxelMeshData.Triangles[i]];
		const int T1 = VIDs[VoxelMeshData.Triangles[i + 1]];
		const int T2 = VIDs[VoxelMeshData.Triangles[i + 2]];
		Mesh.AppendTriangle(T0, T1, T2);
	}

	FMeshNormals::QuickComputeVertexNormals(Mesh);
	// 메시 적용
	MeshComponent->GetDynamicMesh()->SetMesh(MoveTemp(Mesh));
	MeshComponent->NotifyMeshUpdated();
}

void UVoxelChunk::ApplyBrush(const FVector& HitLocation)
{
	ApplyBrushInternal(HitLocation);

	if (!OwningManager)
		return;

	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;
	const FVector ChunkPos = (FVector(chunkSettingInfo.ChunkIndex) + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);
	const FVector LocalPos = HitLocation - GetOwner()->GetActorLocation();

	FVector ChunkMin = ChunkPos - FVector(ChunkSize) * 0.5f;
	FVector ChunkMax = ChunkPos + FVector(ChunkSize) * 0.5f;

	TArray<FIntVector> Offsets;

	const bool NearMinX = LocalPos.X - ChunkMin.X < BrushRadius;
	const bool NearMaxX = ChunkMax.X - LocalPos.X < BrushRadius;
	const bool NearMinY = LocalPos.Y - ChunkMin.Y < BrushRadius;
	const bool NearMaxY = ChunkMax.Y - LocalPos.Y < BrushRadius;
	const bool NearMinZ = LocalPos.Z - ChunkMin.Z < BrushRadius;
	const bool NearMaxZ = ChunkMax.Z - LocalPos.Z < BrushRadius;

	for (int32 dx = -1; dx <= 1; ++dx)
	{
		for (int32 dy = -1; dy <= 1; ++dy)
		{
			for (int32 dz = -1; dz <= 1; ++dz)
			{
				if (dx == 0 && dy == 0 && dz == 0)
					continue;

				bool bValid = true;
				if (dx == -1) bValid &= NearMinX;
				if (dx == 1)  bValid &= NearMaxX;
				if (dy == -1) bValid &= NearMinY;
				if (dy == 1)  bValid &= NearMaxY;
				if (dz == -1) bValid &= NearMinZ;
				if (dz == 1)  bValid &= NearMaxZ;

				if (bValid)
				{
					Offsets.Add(FIntVector(dx, dy, dz));
				}
			}
		}
	}

	for (const FIntVector& Offset : Offsets)
	{
		FIntVector NeighborIndex = FIntVector(chunkSettingInfo.ChunkIndex.X + Offset.X,
											 chunkSettingInfo.ChunkIndex.Y + Offset.Y,
											 chunkSettingInfo.ChunkIndex.Z + Offset.Z);
		if (UVoxelChunk* Neighbor = OwningManager->GetChunk(NeighborIndex))
		{
			Neighbor->ApplyBrushInternal(HitLocation);
		}
	}
}

void UVoxelChunk::ApplyBrushInternal(const FVector& HitLocation)
{
	// Sculpt

	if (chunkSettingInfo.CellCount == 0)
		return;
	
	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;
	const FVector ChunkPos = (FVector(chunkSettingInfo.ChunkIndex) + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);
	
	for (int z=0; z <= chunkSettingInfo.CellCount; z += chunkSettingInfo.LOD)
	{
		for (int y=0; y <= chunkSettingInfo.CellCount; y += chunkSettingInfo.LOD)
		{
			for (int x=0; x <= chunkSettingInfo.CellCount; x += chunkSettingInfo.LOD)
			{
				FVector Pos = FVector(x, y, z) * chunkSettingInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos;
				FVector BrushPos = HitLocation - GetOwner()->GetActorLocation();

				float BrushDensity = FVector::Dist(Pos, BrushPos) - BrushRadius;
				
				VertexDensityData[GetIndex(x,y,z, chunkSettingInfo.CellCount)].Density =
					FMath::Min(VertexDensityData[GetIndex(x,y,z, chunkSettingInfo.CellCount)].Density,
						  BrushDensity);
			}
		}
	}
	
	UpdateMesh();
}

void UVoxelChunk::CalculateVertexDensity()
{
	VertexDensityData.SetNum((chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1));

	const float ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	const float VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;
	const FVector ChunkPos = (FVector(chunkSettingInfo.ChunkIndex) + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);
	
	for (int z=0; z < chunkSettingInfo.CellCount + 1; z += 1)
	{
		for (int y=0; y < chunkSettingInfo.CellCount + 1; y += 1)
		{
			for (int x=0; x < chunkSettingInfo.CellCount + 1; x += 1)
			{
				FVector Pos = FVector(x, y, z) * chunkSettingInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos;
				VertexDensityData[GetIndex(x,y,z,chunkSettingInfo.CellCount)].Density = SampleDensity(Pos, VoxelSize * 0.3f);
			}
		}
	}
	const int CellDim = chunkSettingInfo.CellCount + 1;

	// 병렬처리가 더 시간 오래걸림...
	// ParallelFor(CellDim, [&](int32 z)
	// {
	// 	for (int y = 0; y < CellDim; y++)
	// 	{
	// 		for (int x = 0; x < CellDim; x++)
	// 		{
	// 			FVector Pos = FVector(x, y, z) * chunkSettingInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos;
	// 			VertexDensityData[GetIndex(x, y, z, chunkSettingInfo.CellCount)].Density = SampleDensity(Pos, VoxelSize * 0.3f);
	// 		}
	// 	}
	// });
}

float UVoxelChunk::SampleDensity(const FVector& Pos, int Radius)
{
	float Distance = Pos.Size();
	float Density = Radius - Distance;
	
	//float Noise = FMath::PerlinNoise3D(Position * NoiseScale) * 100.0f;
	return Density;
}

int UVoxelChunk::GetIndex(const int x, const int y, const int z, const int CellCount)
{
	return x + y * (CellCount + 1) + z * (CellCount + 1) * (CellCount + 1);
}

