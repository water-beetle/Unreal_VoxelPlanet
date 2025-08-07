#include "MarchingCubeMeshGenerator.h"
#include "MarchingCubeLookupTable.h"
#include "VoxelWorld/Voxel/VoxelStructs.h"

static const int EdgeCorners[12][2] =
{
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

FVoxelMeshData MarchingCubeMeshGenerator::GenerateMesh(ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData)
{
    FVoxelMeshData VoxelMeshData;

    if (Info.LOD <= 0)
        Info.LOD = 1;

    for (int z = 0; z < Info.CellCount; z += Info.LOD)
    {
        for (int y = 0; y < Info.CellCount; y += Info.LOD)
        {
            for (int x = 0; x < Info.CellCount; x += Info.LOD)
            {
                FVoxelMeshData CellMesh = ProcessCell(Info, VertexDensityData, FIntVector(x, y, z));

                if (CellMesh.Triangles.Num() > 0)
                {
                    int baseIndex = VoxelMeshData.Vertices.Num();
                    VoxelMeshData.Vertices.Append(CellMesh.Vertices);

                    for (int triIdx : CellMesh.Triangles)
                        VoxelMeshData.Triangles.Add(baseIndex + triIdx);
                }
            }
        }
    }

    return VoxelMeshData;
}

FVoxelMeshData MarchingCubeMeshGenerator::GenerateCellMesh(const ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData, const FIntVector& Cell)
{
    return ProcessCell(Info, VertexDensityData, Cell);
}

FVoxelMeshData MarchingCubeMeshGenerator::ProcessCell(const ChunkSettingInfo& Info, const TArray<FVertexDensity>& VertexDensityData, const FIntVector& Cell)
{
    FVoxelMeshData VoxelMeshData;

    const float ChunkSize = Info.CellSize * Info.CellCount;

    FVector CubeCorner[8];
    SetCubeCorner(Cell.X, Cell.Y, Cell.Z, CubeCorner);
    float CubeCornerDensity[8];

    for (int i = 0; i < 8; i++)
    {
        CubeCornerDensity[i] = VertexDensityData[GetIndex(
            CubeCorner[i].X, CubeCorner[i].Y, CubeCorner[i].Z, Info.CellCount)].Density;

        CubeCorner[i] = CubeCorner[i] * Info.CellSize - FVector(ChunkSize) * 0.5f;
    }

    int cubeIndex = 0;
    for (int i = 0; i < 8; i++)
    {
        if (CubeCornerDensity[i] < 0.0f)
            cubeIndex |= (1 << i);
    }

    if (MarchingCubeLooupTable::EdgeTable[cubeIndex] == 0)
        return VoxelMeshData;

    FVector VertexList[12];
    for (int e = 0; e < 12; ++e)
    {
        if (MarchingCubeLooupTable::EdgeTable[cubeIndex] & (1 << e))
        {
            int c0 = EdgeCorners[e][0];
            int c1 = EdgeCorners[e][1];
            VertexList[e] = InterpolateVertex(
                CubeCorner[c0], CubeCorner[c1],
                CubeCornerDensity[c0], CubeCornerDensity[c1]);
        }
    }

    for (int i = 0; MarchingCubeLooupTable::TriTable[cubeIndex][i] != -1; i += 3)
    {
        int idx0 = MarchingCubeLooupTable::TriTable[cubeIndex][i];
        int idx1 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 1];
        int idx2 = MarchingCubeLooupTable::TriTable[cubeIndex][i + 2];

        int vertIndex = VoxelMeshData.Vertices.Num();
        VoxelMeshData.Vertices.Add(VertexList[idx0]);
        VoxelMeshData.Vertices.Add(VertexList[idx1]);
        VoxelMeshData.Vertices.Add(VertexList[idx2]);

        VoxelMeshData.Triangles.Add(vertIndex + 2);
        VoxelMeshData.Triangles.Add(vertIndex + 1);
        VoxelMeshData.Triangles.Add(vertIndex);
    }

    return VoxelMeshData;
}

FVector MarchingCubeMeshGenerator::InterpolateVertex(const FVector& p1, const FVector& p2, float valp1, float valp2)
{
    float t = (0.0f - valp1) / (valp2 - valp1);
    return FMath::Lerp(p1, p2, t);
}

void MarchingCubeMeshGenerator::SetCubeCorner(int X, int Y, int Z, FVector* V)
{
    V[4].X = V[5].X = V[0].X = V[1].X = X;
    V[7].X = V[6].X = V[3].X = V[2].X = X + 1;

    V[0].Y = V[1].Y = V[2].Y = V[3].Y = Y;
    V[4].Y = V[5].Y = V[6].Y = V[7].Y = Y + 1;

    V[0].Z = V[3].Z = V[4].Z = V[7].Z = Z;
    V[1].Z = V[2].Z = V[5].Z = V[6].Z = Z + 1;
}

int MarchingCubeMeshGenerator::GetIndex(int x, int y, int z, int CellCount)
{
    return x + y * (CellCount + 1) + z * (CellCount + 1) * (CellCount + 1);
}
