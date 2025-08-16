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



void UVoxelChunk::Build(const ChunkSettingInfo& InSettings)
{
	// 클래스 변수 값 업데이트
	chunkSettingInfo = InSettings;
	ChunkSize = chunkSettingInfo.CellSize * chunkSettingInfo.CellCount;
	VoxelSize = ChunkSize * chunkSettingInfo.ChunkCount;
	ChunkPos = (FVector(chunkSettingInfo.ChunkIndex) + 0.5f) * ChunkSize - FVector(VoxelSize * 0.5f);

	// Marching Cube에서 사용할 vertex의 desntiy 값 초기화
	CalculateVertexDensity();
	
	// Dynamic Mesh Component 생성 및 등록
	GenerateMeshComponent();

	// Marching Cube를 사용한 Mesh 업데이트
	CachedMeshData = MarchingCubeMeshGenerator::GenerateMesh(chunkSettingInfo, VertexDensityData);
	UpdateMesh(CachedMeshData);
}

void UVoxelChunk::GenerateMeshComponent()
{
	MeshComponent = NewObject<UVoxelMeshComponent>(GetOwner());
	MeshComponent->RegisterComponent();
	MeshComponent->AttachToComponent(GetOwner()->GetRootComponent(),	FAttachmentTransformRules::KeepRelativeTransform);
	MeshComponent->SetComplexAsSimpleCollisionEnabled(true, true);
	MeshComponent->bUseAsyncCooking = true;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	MeshComponent->SetMobility(EComponentMobility::Movable);
	MeshComponent->GetDynamicMesh()->Reset();
	MeshComponent->OwningChunk = this;
	
	// Chunk의 위치를 월드 좌표계에서 이동
	MeshComponent->SetRelativeLocation(ChunkPos);
}

void UVoxelChunk::UpdateMesh(const FVoxelMeshData& VoxelMeshData)
{
	// 삼각형 데이터가 없으면 메시와 충돌을 초기화한 뒤 종료
	if (VoxelMeshData.Triangles.Num() == 0)
	{
		MeshComponent->GetDynamicMesh()->Reset();
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->NotifyMeshUpdated();
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	MeshComponent->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
	{
		EditMesh.Clear(); // 필요 시 기존 데이터 초기화
		EditMesh.EnableVertexNormals(FVector3f());

		Mappings.VertexToTriangles.SetNum(VoxelMeshData.Vertices.Num());
		
		TArray<int32> VIDs;
		VIDs.Reserve(VoxelMeshData.Vertices.Num());

		// 정점 추가
		for (int i = 0; i < VoxelMeshData.Vertices.Num(); i++)
		{
			int32 ID = EditMesh.AppendVertex(VoxelMeshData.Vertices[i]);
			VIDs.Add(ID);
		}

		// 삼각형 추가
		for (int i = 0; i < VoxelMeshData.Triangles.Num(); i += 3)
		{
			int32 T0 = VIDs[VoxelMeshData.Triangles[i]];
			int32 T1 = VIDs[VoxelMeshData.Triangles[i + 1]];
			int32 T2 = VIDs[VoxelMeshData.Triangles[i + 2]];

			int32 TriID = EditMesh.AppendTriangle(T0, T1, T2);

			Mappings.VertexToTriangles[T0].Add(TriID);
			Mappings.VertexToTriangles[T1].Add(TriID);
			Mappings.VertexToTriangles[T2].Add(TriID);

			FIntVector Cell = GetCellFromTriangle(EditMesh, T0, T1, T2);
			Mappings.CellToTriangles.FindOrAdd(Cell).Add(TriID);
			Mappings.CellToVertices.FindOrAdd(Cell).AddUnique(T0);
			Mappings.CellToVertices[Cell].AddUnique(T1);
			Mappings.CellToVertices[Cell].AddUnique(T2);
		}

		// 노멀 재계산
		FMeshNormals::QuickComputeVertexNormals(EditMesh);
	});
	MeshComponent->NotifyMeshUpdated();
}

// 삼각형이 Chunk내 어떤 셀에 들어 있는지 계산하는 함수
FIntVector UVoxelChunk::GetCellFromTriangle(const FDynamicMesh3& Mesh, int32 V0, int32 V1, int32 V2) const
{
	FVector3f P0 = static_cast<FVector3f>(Mesh.GetVertex(V0));
	FVector3f P1 = static_cast<FVector3f>(Mesh.GetVertex(V1));
	FVector3f P2 = static_cast<FVector3f>(Mesh.GetVertex(V2));
	FVector3f Center = (P0 + P1 + P2) / 3.0f;

	// cell 좌측 하단(최소점) 기준이 원점이 되도록 좌표계 변환 후 Index 계산
	int cx = FMath::FloorToInt((Center.X + (chunkSettingInfo.CellSize * chunkSettingInfo.CellCount * 0.5f)) / chunkSettingInfo.CellSize);
	int cy = FMath::FloorToInt((Center.Y + (chunkSettingInfo.CellSize * chunkSettingInfo.CellCount * 0.5f)) / chunkSettingInfo.CellSize);
	int cz = FMath::FloorToInt((Center.Z + (chunkSettingInfo.CellSize * chunkSettingInfo.CellCount * 0.5f)) / chunkSettingInfo.CellSize);
	return FIntVector(cx, cy, cz);
}

void UVoxelChunk::ApplyBrush(const FVector& HitLocation)
{
	ApplyBrushInternal(HitLocation);

	if (!OwningManager)
		return;
	
	const FVector LocalPos = HitLocation - GetOwner()->GetActorLocation();

	FVector ChunkMin = ChunkPos - FVector(ChunkSize) * 0.5f;
	FVector ChunkMax = ChunkPos + FVector(ChunkSize) * 0.5f;

	TArray<FIntVector> Offsets;

	const float EffectiveRadius = BrushRadius + chunkSettingInfo.CellSize;

	const bool NearMinX = LocalPos.X - ChunkMin.X <= EffectiveRadius;
	const bool NearMaxX = ChunkMax.X - LocalPos.X <= EffectiveRadius;
	const bool NearMinY = LocalPos.Y - ChunkMin.Y <= EffectiveRadius;
	const bool NearMaxY = ChunkMax.Y - LocalPos.Y <= EffectiveRadius;
	const bool NearMinZ = LocalPos.Z - ChunkMin.Z <= EffectiveRadius;
	const bool NearMaxZ = ChunkMax.Z - LocalPos.Z <= EffectiveRadius;

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
	TSet<FIntVector> ModifiedCells;

	// 모든 정점 업데이트 (<= CellCount)
	for (int z = 0; z <= chunkSettingInfo.CellCount; z++)
	{
		for (int y = 0; y <= chunkSettingInfo.CellCount; y++)
		{
			for (int x = 0; x <= chunkSettingInfo.CellCount; x++)
			{
				FVector Pos = FVector(x, y, z) * chunkSettingInfo.CellSize - FVector(ChunkSize) * 0.5f + ChunkPos;
				FVector BrushPos = HitLocation - GetOwner()->GetActorLocation();
				float BrushDensity = FVector::Dist(Pos, BrushPos) - BrushRadius;

				int Index = GetIndex(x, y, z, chunkSettingInfo.CellCount);
				if (BrushDensity < VertexDensityData[Index].Density)
				{
					VertexDensityData[Index].Density = BrushDensity;

					// 정점(x,y,z)을 공유하는 8개 셀 모두 후보에 추가
					for (int dx = -1; dx <= 0; ++dx)
						for (int dy = -1; dy <= 0; ++dy)
							for (int dz = -1; dz <= 0; ++dz)
							{
								const int cx = x + dx;
								const int cy = y + dy;
								const int cz = z + dz;

								// 셀 유효 범위: [0, CellCount-1]
								if (0 <= cx && cx < chunkSettingInfo.CellCount &&
									0 <= cy && cy < chunkSettingInfo.CellCount &&
									0 <= cz && cz < chunkSettingInfo.CellCount)
								{
									ModifiedCells.Add(FIntVector(cx, cy, cz));
								}
							}
				}
			}
		}
	}

	UpdateMeshPartialCells(ModifiedCells);

	// if (!IsNearCamera())
	// 	UnloadMappings();
}

void UVoxelChunk::UpdateMeshPartialCells(const TSet<FIntVector>& ModifiedCells)
{
    if (!MeshComponent) return;

    MeshComponent->GetDynamicMesh()->EditMesh([&](FDynamicMesh3& EditMesh)
    {
        // 노멀 어트리뷰트 보장
        EditMesh.EnableVertexNormals(FVector3f());

        auto EnsureV2TSize = [&](int32 VID)
        {
            if (VID >= Mappings.VertexToTriangles.Num())
                Mappings.VertexToTriangles.SetNum(VID + 1);
        };

        auto FindOrAddVertex = [&](const FVector& Pos, const FIntVector& Cell) -> int32
        {
            // 주변 27셀 탐색 (자기 자신 포함)으로 버텍스 재사용
            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    for (int dz = -1; dz <= 1; dz++)
                    {
                        const FIntVector NeighborCell = Cell + FIntVector(dx, dy, dz);
                        if (Mappings.CellToVertices.Contains(NeighborCell))
                        {
                            for (int32 VID : Mappings.CellToVertices[NeighborCell])
                            {
                                if (EditMesh.GetVertex(VID).Equals((FVector3d)Pos, 0.0001))
                                {
                                    EnsureV2TSize(VID);
                                    return VID; // 이미 존재 → 재사용
                                }
                            }
                        }
                    }
                }
            }

            // 없으면 새로 생성
            const int32 VID = EditMesh.AppendVertex(FVector3d(Pos));
            EnsureV2TSize(VID);
            // 재계산 시 이전 노멀 잔존 방지용 초기화(선택)
            EditMesh.SetVertexNormal(VID, FVector3f::ZeroVector);
            return VID;
        };

        for (const FIntVector& Cell : ModifiedCells)
        {
            // 1) 범위 체크
            if (Cell.X < 0 || Cell.Y < 0 || Cell.Z < 0 ||
                Cell.X >= chunkSettingInfo.CellCount ||
                Cell.Y >= chunkSettingInfo.CellCount ||
                Cell.Z >= chunkSettingInfo.CellCount)
            {
                continue;
            }

            // 2) 기존 삼각형 제거
            if (Mappings.CellToTriangles.Contains(Cell))
            {
                for (int32 TriID : Mappings.CellToTriangles[Cell])
                {
                    if (EditMesh.IsTriangle(TriID))
                        EditMesh.RemoveTriangle(TriID, /*bRemoveIsolatedVerts*/false, /*bPreserveManifold*/false);
                }
                Mappings.CellToTriangles[Cell].Reset();
            }

            // 3) 기존 버텍스 매핑 초기화 (실제 버텍스는 재사용 가능)
            if (Mappings.CellToVertices.Contains(Cell))
            {
                Mappings.CellToVertices[Cell].Reset();
            }

            // 4) 새로운 셀 메쉬 생성
            FVoxelMeshData CellMesh = MarchingCubeMeshGenerator::GenerateCellMesh(
                chunkSettingInfo, VertexDensityData, Cell);

            if (CellMesh.Vertices.Num() == 0 || CellMesh.Triangles.Num() == 0)
                continue;

            // 5) 버텍스 추가/재사용
            TArray<int32> VIDs;
            VIDs.Reserve(CellMesh.Vertices.Num());
            for (const FVector& V : CellMesh.Vertices)
            {
                const int32 VID = FindOrAddVertex(V, Cell);
                VIDs.Add(VID);
                Mappings.CellToVertices.FindOrAdd(Cell).Add(VID);
            }

            // 6) 삼각형 추가 + 매핑 갱신(안전한 인덱스 보장)
            for (int i = 0; i < CellMesh.Triangles.Num(); i += 3)
            {
                const int32 Idx0 = VIDs[CellMesh.Triangles[i]];
                const int32 Idx1 = VIDs[CellMesh.Triangles[i + 1]];
                const int32 Idx2 = VIDs[CellMesh.Triangles[i + 2]];

                const int32 TriID = EditMesh.AppendTriangle(Idx0, Idx1, Idx2);

                EnsureV2TSize(Idx0); Mappings.VertexToTriangles[Idx0].Add(TriID);
                EnsureV2TSize(Idx1); Mappings.VertexToTriangles[Idx1].Add(TriID);
                EnsureV2TSize(Idx2); Mappings.VertexToTriangles[Idx2].Add(TriID);

                Mappings.CellToTriangles.FindOrAdd(Cell).Add(TriID);
            }
        }
    	
        FMeshNormals::QuickComputeVertexNormals(EditMesh);
    });

    MeshComponent->NotifyMeshUpdated();
}


void UVoxelChunk::CalculateVertexDensity()
{
	VertexDensityData.SetNum((chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1) * (chunkSettingInfo.CellCount+1));
	
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

