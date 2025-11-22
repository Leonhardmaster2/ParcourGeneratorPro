// Copyright ParkourGeneratorPro. All Rights Reserved.

#include "ParkourPlayArea.h"
#include "ParkourPath.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AParkourPlayArea::AParkourPlayArea()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create bounds box
	BoundsBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundsBox"));
	BoundsBox->SetupAttachment(Root);
	BoundsBox->SetBoxExtent(AreaDimensions * 0.5f);
	BoundsBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BoundsBox->SetHiddenInGame(true);
	BoundsBox->ShapeColor = FColor::Green;
	BoundsBox->SetLineThickness(2.0f);

	// Create custom bounds spline (hidden by default)
	CustomBoundsSpline = CreateDefaultSubobject<USplineComponent>(TEXT("CustomBoundsSpline"));
	CustomBoundsSpline->SetupAttachment(Root);
	CustomBoundsSpline->SetClosedLoop(true);
	CustomBoundsSpline->SetVisibility(false);

	// Initialize spline with a square by default
	CustomBoundsSpline->ClearSplinePoints();
	CustomBoundsSpline->AddSplinePoint(FVector(-2500, -2500, 0), ESplineCoordinateSpace::Local);
	CustomBoundsSpline->AddSplinePoint(FVector(2500, -2500, 0), ESplineCoordinateSpace::Local);
	CustomBoundsSpline->AddSplinePoint(FVector(2500, 2500, 0), ESplineCoordinateSpace::Local);
	CustomBoundsSpline->AddSplinePoint(FVector(-2500, 2500, 0), ESplineCoordinateSpace::Local);
}

void AParkourPlayArea::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	UpdateBoundsVisualization();
}

#if WITH_EDITOR
void AParkourPlayArea::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	UpdateBoundsVisualization();
}
#endif

void AParkourPlayArea::UpdateBoundsVisualization()
{
	// Update box visibility based on shape type
	bool bUseBox = (AreaShape != EParkourAreaShape::Custom);
	BoundsBox->SetVisibility(bShowBounds && bUseBox);
	CustomBoundsSpline->SetVisibility(bShowBounds && !bUseBox);

	// Update box size
	if (bUseBox)
	{
		BoundsBox->SetBoxExtent(AreaDimensions * 0.5f);
	}

#if WITH_EDITOR
	// Draw additional debug shapes for L and U shapes
	if (bShowBounds && GetWorld())
	{
		if (AreaShape == EParkourAreaShape::LShape)
		{
			// Draw L-shape
			FVector BaseOrigin = GetActorLocation();

			// Main arm
			DrawDebugBox(GetWorld(), BaseOrigin, AreaDimensions * 0.5f, FColor::Green, false, -1.0f, 0, 2.0f);

			// Secondary arm
			FVector SecondArmOrigin = BaseOrigin + FVector(AreaDimensions.X * 0.5f + LShapeArmWidth * 0.5f, 0, 0);
			FVector SecondArmExtent(LShapeArmWidth * 0.5f, AreaDimensions.Y * 0.5f, AreaDimensions.Z * 0.5f);
			DrawDebugBox(GetWorld(), SecondArmOrigin, SecondArmExtent, FColor::Green, false, -1.0f, 0, 2.0f);
		}
		else if (AreaShape == EParkourAreaShape::UShape)
		{
			// Draw U-shape
			FVector BaseOrigin = GetActorLocation();

			// Main corridor
			FVector MainExtent(AreaDimensions.X * 0.5f, UShapeArmWidth * 0.5f, AreaDimensions.Z * 0.5f);
			DrawDebugBox(GetWorld(), BaseOrigin, MainExtent, FColor::Green, false, -1.0f, 0, 2.0f);

			// Left arm
			FVector LeftArmOrigin = BaseOrigin + FVector(0, -AreaDimensions.Y * 0.25f, 0);
			FVector ArmExtent(UShapeArmWidth * 0.5f, AreaDimensions.Y * 0.25f, AreaDimensions.Z * 0.5f);
			DrawDebugBox(GetWorld(), LeftArmOrigin, ArmExtent, FColor::Green, false, -1.0f, 0, 2.0f);

			// Right arm
			FVector RightArmOrigin = BaseOrigin + FVector(0, AreaDimensions.Y * 0.25f, 0);
			DrawDebugBox(GetWorld(), RightArmOrigin, ArmExtent, FColor::Green, false, -1.0f, 0, 2.0f);
		}
	}
#endif
}

bool AParkourPlayArea::IsPointInPlayArea(const FVector& Point) const
{
	FVector LocalPoint = GetActorTransform().InverseTransformPosition(Point);

	switch (AreaShape)
	{
	case EParkourAreaShape::Box:
	{
		FVector HalfExtent = AreaDimensions * 0.5f;
		return FMath::Abs(LocalPoint.X) <= HalfExtent.X &&
			FMath::Abs(LocalPoint.Y) <= HalfExtent.Y &&
			LocalPoint.Z >= -10.0f && LocalPoint.Z <= AreaDimensions.Z;
	}

	case EParkourAreaShape::LShape:
	{
		FVector HalfExtent = AreaDimensions * 0.5f;
		// Check main box
		bool bInMain = FMath::Abs(LocalPoint.X) <= HalfExtent.X &&
			FMath::Abs(LocalPoint.Y) <= HalfExtent.Y;
		// Check secondary arm
		bool bInSecondary = LocalPoint.X >= HalfExtent.X &&
			LocalPoint.X <= HalfExtent.X + LShapeArmWidth &&
			FMath::Abs(LocalPoint.Y) <= HalfExtent.Y;
		return (bInMain || bInSecondary) && LocalPoint.Z >= -10.0f && LocalPoint.Z <= AreaDimensions.Z;
	}

	case EParkourAreaShape::UShape:
	{
		FVector HalfExtent = AreaDimensions * 0.5f;
		// Check main corridor
		bool bInMain = FMath::Abs(LocalPoint.X) <= HalfExtent.X &&
			FMath::Abs(LocalPoint.Y) <= UShapeArmWidth * 0.5f;
		// Check left arm
		bool bInLeft = FMath::Abs(LocalPoint.X) <= UShapeArmWidth * 0.5f &&
			LocalPoint.Y <= -UShapeArmWidth * 0.5f &&
			LocalPoint.Y >= -HalfExtent.Y;
		// Check right arm
		bool bInRight = FMath::Abs(LocalPoint.X) <= UShapeArmWidth * 0.5f &&
			LocalPoint.Y >= UShapeArmWidth * 0.5f &&
			LocalPoint.Y <= HalfExtent.Y;
		return (bInMain || bInLeft || bInRight) && LocalPoint.Z >= -10.0f && LocalPoint.Z <= AreaDimensions.Z;
	}

	case EParkourAreaShape::Custom:
		// For custom spline, do a simple 2D point-in-polygon test
		// This is a simplified version - could be improved
		return true; // Placeholder
	}

	return false;
}

FVector AParkourPlayArea::GetRandomPointInArea(FRandomStream& RandomStream) const
{
	FVector Result = GetActorLocation();
	FVector HalfExtent = AreaDimensions * 0.5f;

	switch (AreaShape)
	{
	case EParkourAreaShape::Box:
		Result.X += RandomStream.FRandRange(-HalfExtent.X, HalfExtent.X);
		Result.Y += RandomStream.FRandRange(-HalfExtent.Y, HalfExtent.Y);
		Result.Z += RandomStream.FRandRange(0, AreaDimensions.Z * 0.2f); // Mostly at ground level
		break;

	case EParkourAreaShape::LShape:
	{
		// Randomly choose main or secondary arm
		if (RandomStream.FRand() < 0.6f) // 60% chance main arm
		{
			Result.X += RandomStream.FRandRange(-HalfExtent.X, HalfExtent.X);
			Result.Y += RandomStream.FRandRange(-HalfExtent.Y, HalfExtent.Y);
		}
		else
		{
			Result.X += HalfExtent.X + RandomStream.FRandRange(0, LShapeArmWidth);
			Result.Y += RandomStream.FRandRange(-HalfExtent.Y, HalfExtent.Y);
		}
		Result.Z += RandomStream.FRandRange(0, AreaDimensions.Z * 0.2f);
		break;
	}

	case EParkourAreaShape::UShape:
	{
		float Choice = RandomStream.FRand();
		if (Choice < 0.4f) // Main corridor
		{
			Result.X += RandomStream.FRandRange(-HalfExtent.X, HalfExtent.X);
			Result.Y += RandomStream.FRandRange(-UShapeArmWidth * 0.5f, UShapeArmWidth * 0.5f);
		}
		else if (Choice < 0.7f) // Left arm
		{
			Result.X += RandomStream.FRandRange(-UShapeArmWidth * 0.5f, UShapeArmWidth * 0.5f);
			Result.Y += RandomStream.FRandRange(-HalfExtent.Y, -UShapeArmWidth * 0.5f);
		}
		else // Right arm
		{
			Result.X += RandomStream.FRandRange(-UShapeArmWidth * 0.5f, UShapeArmWidth * 0.5f);
			Result.Y += RandomStream.FRandRange(UShapeArmWidth * 0.5f, HalfExtent.Y);
		}
		Result.Z += RandomStream.FRandRange(0, AreaDimensions.Z * 0.2f);
		break;
	}

	default:
		Result.X += RandomStream.FRandRange(-HalfExtent.X, HalfExtent.X);
		Result.Y += RandomStream.FRandRange(-HalfExtent.Y, HalfExtent.Y);
		break;
	}

	return Result;
}

FBox AParkourPlayArea::GetAreaBounds() const
{
	FVector Origin = GetActorLocation();
	FVector HalfExtent = AreaDimensions * 0.5f;

	FVector Min = Origin - FVector(HalfExtent.X, HalfExtent.Y, 10.0f);
	FVector Max = Origin + FVector(HalfExtent.X + (AreaShape == EParkourAreaShape::LShape ? LShapeArmWidth : 0),
		HalfExtent.Y, AreaDimensions.Z);

	return FBox(Min, Max);
}

bool AParkourPlayArea::IsPlacementValid(const FVector& Location, const FVector& Size) const
{
	// Check if placement overlaps with parkour path
	if (ParkourPath && ParkourPath->PathSpline)
	{
		USplineComponent* Spline = ParkourPath->PathSpline;
		float SplineLength = Spline->GetSplineLength();

		// Sample spline and check distance
		for (float Dist = 0; Dist < SplineLength; Dist += 100.0f)
		{
			FVector SplinePoint = Spline->GetLocationAtDistanceAlongSpline(Dist, ESplineCoordinateSpace::World);

			// Check horizontal distance
			FVector HorizontalDiff = Location - SplinePoint;
			HorizontalDiff.Z = 0;

			if (HorizontalDiff.Size() < PathClearance + FMath::Max(Size.X, Size.Y) * 0.5f)
			{
				return false;
			}
		}

		// Also check generated platforms
		for (const FTransform& PlatformTransform : ParkourPath->GeneratedPlatformTransforms)
		{
			FVector PlatformLoc = PlatformTransform.GetLocation();
			FVector HorizontalDiff = Location - PlatformLoc;
			HorizontalDiff.Z = 0;

			if (HorizontalDiff.Size() < PathClearance + FMath::Max(Size.X, Size.Y) * 0.5f)
			{
				return false;
			}
		}
	}

	return true;
}

UProceduralMeshComponent* AParkourPlayArea::CreateBuildingMesh(const FVector& Location, const FVector& Size, int32 Index)
{
	FString MeshName = FString::Printf(TEXT("Building_%d"), Index);
	UProceduralMeshComponent* BuildingMesh = NewObject<UProceduralMeshComponent>(this, *MeshName);
	BuildingMesh->SetupAttachment(RootComponent);
	BuildingMesh->RegisterComponent();

	// Half sizes
	float HX = Size.X * 0.5f;
	float HY = Size.Y * 0.5f;
	float HZ = Size.Z * 0.5f;

	// Create box vertices
	TArray<FVector> Vertices;
	// Top face
	Vertices.Add(FVector(-HX, -HY, Size.Z));
	Vertices.Add(FVector(HX, -HY, Size.Z));
	Vertices.Add(FVector(HX, HY, Size.Z));
	Vertices.Add(FVector(-HX, HY, Size.Z));
	// Bottom face
	Vertices.Add(FVector(-HX, -HY, 0));
	Vertices.Add(FVector(HX, -HY, 0));
	Vertices.Add(FVector(HX, HY, 0));
	Vertices.Add(FVector(-HX, HY, 0));

	// Triangles
	TArray<int32> Triangles;
	// Top
	Triangles.Append({0, 1, 2, 0, 2, 3});
	// Bottom
	Triangles.Append({4, 6, 5, 4, 7, 6});
	// Front
	Triangles.Append({0, 3, 7, 0, 7, 4});
	// Back
	Triangles.Append({1, 5, 6, 1, 6, 2});
	// Left
	Triangles.Append({0, 4, 5, 0, 5, 1});
	// Right
	Triangles.Append({3, 2, 6, 3, 6, 7});

	// Normals
	TArray<FVector> Normals;
	Normals.SetNum(8);
	for (int32 i = 0; i < 8; i++)
	{
		Normals[i] = (Vertices[i] - FVector(0, 0, Size.Z * 0.5f)).GetSafeNormal();
	}

	// UVs
	TArray<FVector2D> UVs;
	UVs.SetNum(8);
	for (int32 i = 0; i < 8; i++)
	{
		UVs[i] = FVector2D((i % 2), (i / 2) % 2);
	}

	// Vertex colors - darker gray for buildings
	TArray<FColor> VertexColors;
	VertexColors.SetNum(8);
	FColor BuildingColor = FColor(120, 120, 130, 255);
	for (int32 i = 0; i < 8; i++)
	{
		VertexColors[i] = BuildingColor;
	}

	BuildingMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);

	// Set position
	BuildingMesh->SetWorldLocation(Location);

	// Enable collision
	BuildingMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	BuildingMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	return BuildingMesh;
}

void AParkourPlayArea::CreateFloorMesh()
{
	if (GeneratedFloorMesh)
	{
		GeneratedFloorMesh->DestroyComponent();
		GeneratedFloorMesh = nullptr;
	}

	GeneratedFloorMesh = NewObject<UProceduralMeshComponent>(this, TEXT("Floor"));
	GeneratedFloorMesh->SetupAttachment(RootComponent);
	GeneratedFloorMesh->RegisterComponent();

	FVector HalfExtent = AreaDimensions * 0.5f;

	// Floor vertices
	TArray<FVector> Vertices;
	Vertices.Add(FVector(-HalfExtent.X, -HalfExtent.Y, -FloorOffset));
	Vertices.Add(FVector(HalfExtent.X, -HalfExtent.Y, -FloorOffset));
	Vertices.Add(FVector(HalfExtent.X, HalfExtent.Y, -FloorOffset));
	Vertices.Add(FVector(-HalfExtent.X, HalfExtent.Y, -FloorOffset));

	TArray<int32> Triangles = {0, 1, 2, 0, 2, 3};

	TArray<FVector> Normals = {FVector::UpVector, FVector::UpVector, FVector::UpVector, FVector::UpVector};

	TArray<FVector2D> UVs = {FVector2D(0, 0), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1)};

	TArray<FColor> VertexColors;
	FColor FloorColor(100, 100, 100, 255);
	VertexColors.SetNum(4);
	for (int32 i = 0; i < 4; i++)
	{
		VertexColors[i] = FloorColor;
	}

	GeneratedFloorMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);
	GeneratedFloorMesh->SetWorldLocation(GetActorLocation());

	GeneratedFloorMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GeneratedFloorMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
}

void AParkourPlayArea::GenerateUrbanBuildings(FRandomStream& RandomStream)
{
	FBox Bounds = GetAreaBounds();
	FVector AreaSize = Bounds.GetSize();

	// Calculate grid for building placement
	float GridCellSize = FMath::Max(MaxBuildingSize.X, MaxBuildingSize.Y) * 1.5f;
	int32 GridX = FMath::CeilToInt(AreaSize.X / GridCellSize);
	int32 GridY = FMath::CeilToInt(AreaSize.Y / GridCellSize);

	int32 BuildingIndex = 0;

	for (int32 X = 0; X < GridX; X++)
	{
		for (int32 Y = 0; Y < GridY; Y++)
		{
			// Apply density check
			if (RandomStream.FRand() > BuildingDensity)
			{
				continue;
			}

			// Calculate cell position
			FVector CellOrigin = Bounds.Min + FVector(X * GridCellSize, Y * GridCellSize, 0);

			// Randomize position within cell
			FVector BuildingPos = CellOrigin + FVector(
				RandomStream.FRandRange(0, GridCellSize * 0.5f),
				RandomStream.FRandRange(0, GridCellSize * 0.5f),
				0
			);

			// Check if in play area
			if (!IsPointInPlayArea(BuildingPos))
			{
				continue;
			}

			// Randomize building size
			FVector BuildingSize;
			BuildingSize.X = RandomStream.FRandRange(MinBuildingSize.X, MaxBuildingSize.X);
			BuildingSize.Y = RandomStream.FRandRange(MinBuildingSize.Y, MaxBuildingSize.Y);
			BuildingSize.Z = RandomStream.FRandRange(MinBuildingSize.Z, MaxBuildingSize.Z);

			// Check if placement is valid
			if (!IsPlacementValid(BuildingPos, BuildingSize))
			{
				continue;
			}

			// Create building
			UProceduralMeshComponent* Building = CreateBuildingMesh(BuildingPos, BuildingSize, BuildingIndex);
			if (Building)
			{
				GeneratedBuildingMeshes.Add(Building);
				BuildingIndex++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d urban buildings"), BuildingIndex);
}

void AParkourPlayArea::GenerateIndustrialStructures(FRandomStream& RandomStream)
{
	// Similar to urban but with more horizontal structures
	FBox Bounds = GetAreaBounds();
	FVector AreaSize = Bounds.GetSize();

	float GridCellSize = FMath::Max(MaxBuildingSize.X, MaxBuildingSize.Y) * 2.0f;
	int32 GridX = FMath::CeilToInt(AreaSize.X / GridCellSize);
	int32 GridY = FMath::CeilToInt(AreaSize.Y / GridCellSize);

	int32 StructureIndex = 0;

	for (int32 X = 0; X < GridX; X++)
	{
		for (int32 Y = 0; Y < GridY; Y++)
		{
			if (RandomStream.FRand() > BuildingDensity * 0.8f) // Slightly less dense
			{
				continue;
			}

			FVector CellOrigin = Bounds.Min + FVector(X * GridCellSize, Y * GridCellSize, 0);
			FVector StructurePos = CellOrigin + FVector(
				RandomStream.FRandRange(0, GridCellSize * 0.3f),
				RandomStream.FRandRange(0, GridCellSize * 0.3f),
				0
			);

			if (!IsPointInPlayArea(StructurePos))
			{
				continue;
			}

			// Industrial structures are wider and shorter
			FVector StructureSize;
			StructureSize.X = RandomStream.FRandRange(MinBuildingSize.X * 1.5f, MaxBuildingSize.X * 1.5f);
			StructureSize.Y = RandomStream.FRandRange(MinBuildingSize.Y * 1.5f, MaxBuildingSize.Y * 1.5f);
			StructureSize.Z = RandomStream.FRandRange(MinBuildingSize.Z * 0.5f, MaxBuildingSize.Z * 0.6f);

			if (!IsPlacementValid(StructurePos, StructureSize))
			{
				continue;
			}

			UProceduralMeshComponent* Structure = CreateBuildingMesh(StructurePos, StructureSize, StructureIndex);
			if (Structure)
			{
				GeneratedBuildingMeshes.Add(Structure);
				StructureIndex++;
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d industrial structures"), StructureIndex);
}

void AParkourPlayArea::GenerateAbstractBlockout(FRandomStream& RandomStream)
{
	// Generate various sized blocks for abstract feeling
	FBox Bounds = GetAreaBounds();

	int32 NumBlocks = FMath::CeilToInt(BuildingDensity * 30);
	int32 BlockIndex = 0;

	for (int32 i = 0; i < NumBlocks; i++)
	{
		FVector BlockPos = GetRandomPointInArea(RandomStream);

		if (!IsPointInPlayArea(BlockPos))
		{
			continue;
		}

		// Abstract blocks have more size variety
		FVector BlockSize;
		BlockSize.X = RandomStream.FRandRange(MinBuildingSize.X * 0.5f, MaxBuildingSize.X);
		BlockSize.Y = RandomStream.FRandRange(MinBuildingSize.Y * 0.5f, MaxBuildingSize.Y);
		BlockSize.Z = RandomStream.FRandRange(MinBuildingSize.Z * 0.3f, MaxBuildingSize.Z * 1.2f);

		if (!IsPlacementValid(BlockPos, BlockSize))
		{
			continue;
		}

		UProceduralMeshComponent* Block = CreateBuildingMesh(BlockPos, BlockSize, BlockIndex);
		if (Block)
		{
			GeneratedBuildingMeshes.Add(Block);
			BlockIndex++;
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d abstract blocks"), BlockIndex);
}

void AParkourPlayArea::GenerateEnvironment()
{
	ClearGeneratedEnvironment();

	FRandomStream RandomStream;
	if (EnvironmentSeed != 0)
	{
		RandomStream.Initialize(EnvironmentSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}

	// Generate floor if requested
	if (bGenerateFloor)
	{
		CreateFloorMesh();
	}

	// Generate environment based on style
	switch (EnvironmentStyle)
	{
	case EParkourEnvironmentStyle::Urban:
		GenerateUrbanBuildings(RandomStream);
		break;

	case EParkourEnvironmentStyle::Industrial:
		GenerateIndustrialStructures(RandomStream);
		break;

	case EParkourEnvironmentStyle::Abstract:
		GenerateAbstractBlockout(RandomStream);
		break;

	case EParkourEnvironmentStyle::None:
	default:
		break;
	}
}

void AParkourPlayArea::ClearGeneratedEnvironment()
{
	for (UProceduralMeshComponent* Mesh : GeneratedBuildingMeshes)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	GeneratedBuildingMeshes.Empty();

	if (GeneratedFloorMesh)
	{
		GeneratedFloorMesh->DestroyComponent();
		GeneratedFloorMesh = nullptr;
	}
}

void AParkourPlayArea::GenerateAll()
{
	// First generate parkour path platforms
	if (ParkourPath)
	{
		ParkourPath->GenerateParkour();
	}

	// Then generate environment (which avoids the path)
	GenerateEnvironment();
}

void AParkourPlayArea::ClearAll()
{
	if (ParkourPath)
	{
		ParkourPath->ClearGeneratedParkour();
	}

	ClearGeneratedEnvironment();
}
