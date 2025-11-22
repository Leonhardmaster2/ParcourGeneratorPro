// Copyright ParkourGeneratorPro. All Rights Reserved.

#include "ParkourPath.h"
#include "ProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AParkourPath::AParkourPath()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create spline component
	PathSpline = CreateDefaultSubobject<USplineComponent>(TEXT("PathSpline"));
	PathSpline->SetupAttachment(Root);
	PathSpline->SetClosedLoop(false);

	// Set up default spline with some initial points
	PathSpline->ClearSplinePoints();
	PathSpline->AddSplinePoint(FVector(0, 0, 0), ESplineCoordinateSpace::Local);
	PathSpline->AddSplinePoint(FVector(500, 0, 100), ESplineCoordinateSpace::Local);
	PathSpline->AddSplinePoint(FVector(1000, 300, 200), ESplineCoordinateSpace::Local);
	PathSpline->AddSplinePoint(FVector(1500, 300, 400), ESplineCoordinateSpace::Local);
	PathSpline->AddSplinePoint(FVector(2000, 0, 300), ESplineCoordinateSpace::Local);

	// Visual settings for the spline in editor
	PathSpline->SetDrawDebug(true);
	PathSpline->SetUnselectedSplineSegmentColor(FLinearColor(0.0f, 0.8f, 0.2f));
	PathSpline->SetSelectedSplineSegmentColor(FLinearColor(0.0f, 1.0f, 0.0f));
	PathSpline->ScaleVisualizationWidth = 10.0f;
}

void AParkourPath::BeginPlay()
{
	Super::BeginPlay();
}

void AParkourPath::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bShowPlatformPreview)
	{
		UpdatePreview();
	}
}

#if WITH_EDITOR
void AParkourPath::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (bShowPlatformPreview)
	{
		UpdatePreview();
	}
}
#endif

bool AParkourPath::IsJumpValid(const FVector& FromPoint, const FVector& ToPoint) const
{
	const FVector Diff = ToPoint - FromPoint;
	const float HorizontalDist = FVector(Diff.X, Diff.Y, 0).Size();
	const float VerticalDist = Diff.Z;

	// Check horizontal distance
	if (HorizontalDist > GetSafeJumpDistance())
	{
		return false;
	}

	// Check vertical distance (upward jump)
	if (VerticalDist > 0 && VerticalDist > GetSafeJumpHeight())
	{
		return false;
	}

	// Falling is generally okay within reasonable limits
	// But we still want to ensure the horizontal distance is valid
	// For very large drops, we might want to add intermediate platforms
	if (VerticalDist < -GetSafeJumpHeight() * 3.0f)
	{
		return false;
	}

	return true;
}

TArray<FTransform> AParkourPath::CalculatePlatformPositions()
{
	TArray<FTransform> Platforms;

	if (!PathSpline || PathSpline->GetNumberOfSplinePoints() < 2)
	{
		return Platforms;
	}

	// Initialize random stream
	if (RandomSeed != 0)
	{
		RandomStream.Initialize(RandomSeed);
	}
	else
	{
		RandomStream.GenerateNewSeed();
	}

	const float SplineLength = PathSpline->GetSplineLength();
	const float SafeJumpDist = GetSafeJumpDistance();

	// Calculate approximate number of platforms needed
	int32 EstimatedPlatforms = FMath::CeilToInt(SplineLength / SafeJumpDist) + 1;
	EstimatedPlatforms = FMath::Clamp(EstimatedPlatforms, MinPlatforms, MaxPlatforms);

	// Start with first platform at spline start
	float CurrentDistance = 0.0f;
	FVector LastPosition = PathSpline->GetLocationAtDistanceAlongSpline(0, ESplineCoordinateSpace::World);

	// Add first platform
	FTransform FirstTransform;
	FirstTransform.SetLocation(LastPosition);
	FVector FirstDir = PathSpline->GetDirectionAtDistanceAlongSpline(0, ESplineCoordinateSpace::World);
	FirstTransform.SetRotation(FirstDir.ToOrientationQuat());
	Platforms.Add(FirstTransform);

	// Generate platforms along the path
	while (CurrentDistance < SplineLength)
	{
		// Determine next jump distance with variation
		float NextJumpDist = SafeJumpDist;
		if (bAllowPlatformVariation)
		{
			float Variation = RandomStream.FRandRange(-PlatformSizeVariation, PlatformSizeVariation);
			NextJumpDist *= (1.0f + Variation);
			NextJumpDist = FMath::Max(NextJumpDist, SafeJumpDist * 0.5f); // Don't go below 50% of safe distance
		}

		// Try to place next platform
		float TestDistance = CurrentDistance + NextJumpDist;

		if (TestDistance >= SplineLength)
		{
			// Place final platform at end of spline
			TestDistance = SplineLength;
		}

		FVector NextPosition = PathSpline->GetLocationAtDistanceAlongSpline(TestDistance, ESplineCoordinateSpace::World);
		FVector NextDirection = PathSpline->GetDirectionAtDistanceAlongSpline(TestDistance, ESplineCoordinateSpace::World);

		// Check if jump is valid
		if (IsJumpValid(LastPosition, NextPosition))
		{
			// Valid jump - add platform
			FTransform NewTransform;
			NewTransform.SetLocation(NextPosition);
			NewTransform.SetRotation(NextDirection.ToOrientationQuat());

			// Add some platform size variation
			if (bAllowPlatformVariation)
			{
				float ScaleVariation = 1.0f + RandomStream.FRandRange(-PlatformSizeVariation * 0.5f, PlatformSizeVariation * 0.5f);
				NewTransform.SetScale3D(FVector(ScaleVariation, ScaleVariation, 1.0f));
			}

			Platforms.Add(NewTransform);
			LastPosition = NextPosition;
			CurrentDistance = TestDistance;
		}
		else
		{
			// Invalid jump - need intermediate platforms
			FTransform FromTransform;
			FromTransform.SetLocation(LastPosition);

			FTransform ToTransform;
			ToTransform.SetLocation(NextPosition);
			ToTransform.SetRotation(NextDirection.ToOrientationQuat());

			AddIntermediatePlatforms(Platforms, FromTransform, ToTransform);

			// Update last position to the newly added platform
			if (Platforms.Num() > 0)
			{
				LastPosition = Platforms.Last().GetLocation();
			}
			CurrentDistance = TestDistance;
		}

		// Safety check to prevent infinite loops
		if (Platforms.Num() >= MaxPlatforms)
		{
			break;
		}

		// If we've reached the end, break
		if (TestDistance >= SplineLength)
		{
			break;
		}
	}

	return Platforms;
}

void AParkourPath::AddIntermediatePlatforms(TArray<FTransform>& Platforms, const FTransform& From, const FTransform& To)
{
	const FVector FromPos = From.GetLocation();
	const FVector ToPos = To.GetLocation();
	const FVector Diff = ToPos - FromPos;
	const float TotalDistance = Diff.Size();
	const float SafeJumpDist = GetSafeJumpDistance();
	const float SafeJumpH = GetSafeJumpHeight();

	// Calculate number of intermediate platforms needed
	int32 NumIntermediates = 1;

	// Check horizontal requirement
	float HorizontalDist = FVector(Diff.X, Diff.Y, 0).Size();
	if (HorizontalDist > SafeJumpDist)
	{
		NumIntermediates = FMath::Max(NumIntermediates, FMath::CeilToInt(HorizontalDist / SafeJumpDist));
	}

	// Check vertical requirement (for upward jumps)
	float VerticalDist = FMath::Abs(Diff.Z);
	if (Diff.Z > SafeJumpH)
	{
		int32 VerticalIntermediates = FMath::CeilToInt(Diff.Z / SafeJumpH);
		NumIntermediates = FMath::Max(NumIntermediates, VerticalIntermediates);
	}

	// Add intermediate platforms
	for (int32 i = 1; i <= NumIntermediates; i++)
	{
		float Alpha = (float)i / (float)(NumIntermediates + 1);
		FVector IntermediatePos = FMath::Lerp(FromPos, ToPos, Alpha);

		// Add some horizontal offset variation for more interesting layout
		if (bAllowPlatformVariation && NumIntermediates > 1)
		{
			FVector RightVec = FVector::CrossProduct(Diff.GetSafeNormal(), FVector::UpVector);
			float OffsetAmount = RandomStream.FRandRange(-PlatformWidth * 0.5f, PlatformWidth * 0.5f);
			IntermediatePos += RightVec * OffsetAmount;
		}

		FTransform IntermediateTransform;
		IntermediateTransform.SetLocation(IntermediatePos);
		IntermediateTransform.SetRotation(To.GetRotation());

		Platforms.Add(IntermediateTransform);
	}

	// Add final platform
	Platforms.Add(To);
}

UProceduralMeshComponent* AParkourPath::CreatePlatformMesh(const FTransform& PlatformTransform, int32 Index)
{
	// Create procedural mesh component
	FString MeshName = FString::Printf(TEXT("Platform_%d"), Index);
	UProceduralMeshComponent* PlatformMesh = NewObject<UProceduralMeshComponent>(this, *MeshName);
	PlatformMesh->SetupAttachment(RootComponent);
	PlatformMesh->RegisterComponent();

	// Get platform dimensions with any scale variation
	float Width = PlatformWidth * PlatformTransform.GetScale3D().X;
	float Depth = PlatformDepth * PlatformTransform.GetScale3D().Y;
	float Thickness = PlatformThickness;

	// Half dimensions for vertex positions
	float HW = Width * 0.5f;
	float HD = Depth * 0.5f;
	float HT = Thickness * 0.5f;

	// Create box vertices (8 vertices for a box)
	TArray<FVector> Vertices;
	// Top face
	Vertices.Add(FVector(-HD, -HW, HT));  // 0: top front left
	Vertices.Add(FVector(HD, -HW, HT));   // 1: top back left
	Vertices.Add(FVector(HD, HW, HT));    // 2: top back right
	Vertices.Add(FVector(-HD, HW, HT));   // 3: top front right
	// Bottom face
	Vertices.Add(FVector(-HD, -HW, -HT)); // 4: bottom front left
	Vertices.Add(FVector(HD, -HW, -HT));  // 5: bottom back left
	Vertices.Add(FVector(HD, HW, -HT));   // 6: bottom back right
	Vertices.Add(FVector(-HD, HW, -HT));  // 7: bottom front right

	// Create triangles (12 triangles for 6 faces)
	TArray<int32> Triangles;
	// Top face
	Triangles.Append({0, 1, 2, 0, 2, 3});
	// Bottom face
	Triangles.Append({4, 6, 5, 4, 7, 6});
	// Front face
	Triangles.Append({0, 3, 7, 0, 7, 4});
	// Back face
	Triangles.Append({1, 5, 6, 1, 6, 2});
	// Left face
	Triangles.Append({0, 4, 5, 0, 5, 1});
	// Right face
	Triangles.Append({3, 2, 6, 3, 6, 7});

	// Create normals
	TArray<FVector> Normals;
	Normals.SetNum(Vertices.Num());
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		Normals[i] = Vertices[i].GetSafeNormal();
	}

	// Create UVs
	TArray<FVector2D> UVs;
	UVs.SetNum(Vertices.Num());
	UVs[0] = FVector2D(0, 0);
	UVs[1] = FVector2D(1, 0);
	UVs[2] = FVector2D(1, 1);
	UVs[3] = FVector2D(0, 1);
	UVs[4] = FVector2D(0, 0);
	UVs[5] = FVector2D(1, 0);
	UVs[6] = FVector2D(1, 1);
	UVs[7] = FVector2D(0, 1);

	// Create vertex colors (light gray for blockout)
	TArray<FColor> VertexColors;
	VertexColors.SetNum(Vertices.Num());
	for (int32 i = 0; i < Vertices.Num(); i++)
	{
		VertexColors[i] = FColor(200, 200, 200, 255);
	}

	// Create the mesh section
	PlatformMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>(), true);

	// Set world transform
	FVector Location = PlatformTransform.GetLocation();
	Location.Z -= HT; // Offset so top of platform is at the path height
	PlatformMesh->SetWorldLocation(Location);
	PlatformMesh->SetWorldRotation(PlatformTransform.GetRotation().Rotator());

	// Enable collision
	PlatformMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

	return PlatformMesh;
}

void AParkourPath::GenerateParkour()
{
	// Clear any existing platforms
	ClearGeneratedParkour();

	// Calculate platform positions
	GeneratedPlatformTransforms = CalculatePlatformPositions();

	// Create platform meshes
	for (int32 i = 0; i < GeneratedPlatformTransforms.Num(); i++)
	{
		UProceduralMeshComponent* PlatformMesh = CreatePlatformMesh(GeneratedPlatformTransforms[i], i);
		if (PlatformMesh)
		{
			GeneratedPlatformMeshes.Add(PlatformMesh);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Generated %d parkour platforms"), GeneratedPlatformTransforms.Num());
}

void AParkourPath::ClearGeneratedParkour()
{
	// Destroy all generated platform meshes
	for (UProceduralMeshComponent* Mesh : GeneratedPlatformMeshes)
	{
		if (Mesh)
		{
			Mesh->DestroyComponent();
		}
	}
	GeneratedPlatformMeshes.Empty();
	GeneratedPlatformTransforms.Empty();
}

void AParkourPath::UpdatePreview()
{
#if WITH_EDITOR
	// Draw debug visualization of platform positions
	if (!GetWorld())
	{
		return;
	}

	// Calculate positions but don't create meshes
	TArray<FTransform> PreviewTransforms = CalculatePlatformPositions();

	// Draw debug boxes and jump arcs
	for (int32 i = 0; i < PreviewTransforms.Num(); i++)
	{
		const FTransform& CurrentTransform = PreviewTransforms[i];
		FVector Location = CurrentTransform.GetLocation();

		// Draw platform preview box
		float Width = PlatformWidth * CurrentTransform.GetScale3D().X;
		float Depth = PlatformDepth * CurrentTransform.GetScale3D().Y;

		DrawDebugBox(
			GetWorld(),
			Location - FVector(0, 0, PlatformThickness * 0.5f),
			FVector(Depth * 0.5f, Width * 0.5f, PlatformThickness * 0.5f),
			CurrentTransform.GetRotation(),
			FColor::Cyan,
			false,
			-1.0f,
			0,
			2.0f
		);

		// Draw jump arcs to next platform
		if (bShowJumpArcs && i < PreviewTransforms.Num() - 1)
		{
			const FTransform& NextTransform = PreviewTransforms[i + 1];
			FVector NextLocation = NextTransform.GetLocation();

			// Draw arc
			FVector MidPoint = (Location + NextLocation) * 0.5f;
			MidPoint.Z += GetSafeJumpHeight() * 0.5f; // Arc peak

			// Draw simple line for now (could be improved to parabola)
			DrawDebugLine(GetWorld(), Location, MidPoint, FColor::Yellow, false, -1.0f, 0, 1.0f);
			DrawDebugLine(GetWorld(), MidPoint, NextLocation, FColor::Yellow, false, -1.0f, 0, 1.0f);
		}

		// Draw platform number
		DrawDebugString(GetWorld(), Location + FVector(0, 0, 50), FString::Printf(TEXT("%d"), i), nullptr, FColor::White, -1.0f);
	}
#endif
}
