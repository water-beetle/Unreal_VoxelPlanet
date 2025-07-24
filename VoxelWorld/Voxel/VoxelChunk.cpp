// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelChunk.h"
#include "Components/DynamicMeshComponent.h"
#include "DynamicMesh/MeshNormals.h"
#include "VoxelWorld/MarchingCubes/MarchingCubeMeshGenerator.h"

using namespace UE::Geometry;

// Sets default values for this component's properties
UVoxelChunk::UVoxelChunk()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
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

void UVoxelChunk::GenerateMesh(FChunkInfo& ChunkInfo)
{
	FVoxelMeshData VoxelMeshData = MarchingCubeMeshGenerator::GenerateMesh(ChunkInfo);

	MeshComponent = NewObject<UDynamicMeshComponent>(GetOwner());
	MeshComponent->RegisterComponent();
	MeshComponent->SetComplexAsSimpleCollisionEnabled(true, true);
	MeshComponent->bUseAsyncCooking = true;
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetGenerateOverlapEvents(true);
	MeshComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	
	FDynamicMesh3 Mesh;
	Mesh.EnableVertexNormals(FVector3f());
	

	TArray<int32> VIDs;
	for (int i = 0; i < VoxelMeshData.Vertices.Num(); i++)
	{
		int32 ID = Mesh.AppendVertex(VoxelMeshData.Vertices[i]);
		VIDs.Add(ID);
		Mesh.SetVertexNormal(ID, FVector3f(VoxelMeshData.Normals[i]));
	}

	for (int i = 0; i < VoxelMeshData.Triangles.Num(); i += 3)
	{
		const int T0 = VIDs[VoxelMeshData.Triangles[i]];
		const int T1 = VIDs[VoxelMeshData.Triangles[i + 1]];
		const int T2 = VIDs[VoxelMeshData.Triangles[i + 2]];
		Mesh.AppendTriangle(T0, T1, T2);
	}

	// 메시 적용
	MeshComponent->GetDynamicMesh()->Reset();
	MeshComponent->GetDynamicMesh()->SetMesh(MoveTemp(Mesh));
	MeshComponent->NotifyMeshUpdated();
	
}

