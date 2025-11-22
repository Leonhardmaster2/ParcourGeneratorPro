// Copyright ParkourGeneratorPro. All Rights Reserved.

#include "ParkourGeneratorSubsystem.h"
#include "ParkourPath.h"
#include "ParkourPlayArea.h"
#include "ParkourGeneratorSettings.h"
#include "EngineUtils.h"
#include "Editor.h"

UWorld* UParkourGeneratorSubsystem::GetEditorWorld() const
{
	if (GEditor)
	{
		return GEditor->GetEditorWorldContext().World();
	}
	return nullptr;
}

AParkourPath* UParkourGeneratorSubsystem::SpawnParkourPath(const FVector& Location)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParkourGenerator: No editor world available"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AParkourPath* NewPath = World->SpawnActor<AParkourPath>(AParkourPath::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

	if (NewPath)
	{
		// Apply default settings
		const UParkourGeneratorSettings* Settings = UParkourGeneratorSettings::Get();
		if (Settings)
		{
			NewPath->MaxJumpHeight = Settings->DefaultMaxJumpHeight;
			NewPath->MaxJumpDistance = Settings->DefaultMaxJumpDistance;
			NewPath->JumpSafetyMargin = Settings->DefaultJumpSafetyMargin;
			NewPath->PlatformWidth = Settings->DefaultPlatformWidth;
			NewPath->PlatformDepth = Settings->DefaultPlatformDepth;
			NewPath->PlatformThickness = Settings->DefaultPlatformThickness;
		}

		// Select the new actor
		if (GEditor)
		{
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(NewPath, true, true);
		}

		UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Spawned new Parkour Path at %s"), *Location.ToString());
	}

	return NewPath;
}

AParkourPlayArea* UParkourGeneratorSubsystem::SpawnPlayArea(const FVector& Location, const FVector& Dimensions)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("ParkourGenerator: No editor world available"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AParkourPlayArea* NewArea = World->SpawnActor<AParkourPlayArea>(AParkourPlayArea::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);

	if (NewArea)
	{
		NewArea->AreaDimensions = Dimensions;

		// Apply default settings
		const UParkourGeneratorSettings* Settings = UParkourGeneratorSettings::Get();
		if (Settings)
		{
			NewArea->BuildingDensity = Settings->DefaultBuildingDensity;
			NewArea->PathClearance = Settings->DefaultPathClearance;
		}

		// Select the new actor
		if (GEditor)
		{
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(NewArea, true, true);
		}

		UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Spawned new Play Area at %s with dimensions %s"), *Location.ToString(), *Dimensions.ToString());
	}

	return NewArea;
}

void UParkourGeneratorSubsystem::SpawnCompleteParkourSetup(const FVector& Location, AParkourPath*& OutPath, AParkourPlayArea*& OutPlayArea)
{
	// Spawn play area first
	OutPlayArea = SpawnPlayArea(Location, FVector(5000, 5000, 2000));

	// Spawn path at center
	OutPath = SpawnParkourPath(Location);

	// Link them together
	if (OutPlayArea && OutPath)
	{
		OutPlayArea->ParkourPath = OutPath;

		// Select both
		if (GEditor)
		{
			GEditor->SelectNone(true, true);
			GEditor->SelectActor(OutPath, true, true);
			GEditor->SelectActor(OutPlayArea, true, false);
		}

		UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Spawned complete parkour setup"));
	}
}

TArray<AParkourPath*> UParkourGeneratorSubsystem::GetAllParkourPaths() const
{
	TArray<AParkourPath*> Paths;

	UWorld* World = GetEditorWorld();
	if (World)
	{
		for (TActorIterator<AParkourPath> It(World); It; ++It)
		{
			Paths.Add(*It);
		}
	}

	return Paths;
}

TArray<AParkourPlayArea*> UParkourGeneratorSubsystem::GetAllPlayAreas() const
{
	TArray<AParkourPlayArea*> Areas;

	UWorld* World = GetEditorWorld();
	if (World)
	{
		for (TActorIterator<AParkourPlayArea> It(World); It; ++It)
		{
			Areas.Add(*It);
		}
	}

	return Areas;
}

void UParkourGeneratorSubsystem::GenerateAllParkour()
{
	TArray<AParkourPath*> Paths = GetAllParkourPaths();

	for (AParkourPath* Path : Paths)
	{
		if (Path)
		{
			Path->GenerateParkour();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Generated parkour for %d paths"), Paths.Num());
}

void UParkourGeneratorSubsystem::ClearAllParkour()
{
	TArray<AParkourPath*> Paths = GetAllParkourPaths();

	for (AParkourPath* Path : Paths)
	{
		if (Path)
		{
			Path->ClearGeneratedParkour();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Cleared parkour for %d paths"), Paths.Num());
}

void UParkourGeneratorSubsystem::GenerateAllEnvironments()
{
	TArray<AParkourPlayArea*> Areas = GetAllPlayAreas();

	for (AParkourPlayArea* Area : Areas)
	{
		if (Area)
		{
			Area->GenerateEnvironment();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Generated environment for %d areas"), Areas.Num());
}

void UParkourGeneratorSubsystem::ClearAllEnvironments()
{
	TArray<AParkourPlayArea*> Areas = GetAllPlayAreas();

	for (AParkourPlayArea* Area : Areas)
	{
		if (Area)
		{
			Area->ClearGeneratedEnvironment();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("ParkourGenerator: Cleared environment for %d areas"), Areas.Num());
}

void UParkourGeneratorSubsystem::GenerateEverything()
{
	// Generate parkour first (so environment can avoid it)
	GenerateAllParkour();

	// Then generate environments
	GenerateAllEnvironments();
}

void UParkourGeneratorSubsystem::ClearEverything()
{
	ClearAllEnvironments();
	ClearAllParkour();
}

bool UParkourGeneratorSubsystem::ValidatePath(AParkourPath* Path, TArray<int32>& OutInvalidJumpIndices)
{
	OutInvalidJumpIndices.Empty();

	if (!Path || Path->GeneratedPlatformTransforms.Num() < 2)
	{
		return true; // Nothing to validate
	}

	bool bAllValid = true;

	for (int32 i = 0; i < Path->GeneratedPlatformTransforms.Num() - 1; i++)
	{
		FVector FromPos = Path->GeneratedPlatformTransforms[i].GetLocation();
		FVector ToPos = Path->GeneratedPlatformTransforms[i + 1].GetLocation();

		if (!Path->IsJumpValid(FromPos, ToPos))
		{
			OutInvalidJumpIndices.Add(i);
			bAllValid = false;
		}
	}

	return bAllValid;
}
