// Copyright ParkourGeneratorPro. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "ParkourGeneratorSubsystem.generated.h"

class AParkourPath;
class AParkourPlayArea;

/**
 * Parkour Generator Editor Subsystem
 *
 * Provides Blueprint-callable functions for spawning and managing parkour generator actors.
 */
UCLASS()
class PARKOURGENERATOR_API UParkourGeneratorSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	// ========== SPAWNING ==========

	/** Spawn a new Parkour Path at the given location */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	AParkourPath* SpawnParkourPath(const FVector& Location = FVector::ZeroVector);

	/** Spawn a new Play Area at the given location */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	AParkourPlayArea* SpawnPlayArea(const FVector& Location = FVector::ZeroVector, const FVector& Dimensions = FVector(5000, 5000, 2000));

	/** Spawn a complete parkour setup (Path + Play Area linked together) */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void SpawnCompleteParkourSetup(const FVector& Location, AParkourPath*& OutPath, AParkourPlayArea*& OutPlayArea);

	// ========== FINDING ==========

	/** Find all Parkour Paths in the current level */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	TArray<AParkourPath*> GetAllParkourPaths() const;

	/** Find all Play Areas in the current level */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	TArray<AParkourPlayArea*> GetAllPlayAreas() const;

	// ========== GENERATION ==========

	/** Generate parkour for all paths in the level */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void GenerateAllParkour();

	/** Clear all generated parkour in the level */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void ClearAllParkour();

	/** Generate environment for all play areas */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void GenerateAllEnvironments();

	/** Clear all generated environments */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void ClearAllEnvironments();

	/** Generate everything (parkour + environments) */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void GenerateEverything();

	/** Clear everything */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	void ClearEverything();

	// ========== UTILITY ==========

	/** Validate that all jumps in a path are achievable */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	bool ValidatePath(AParkourPath* Path, TArray<int32>& OutInvalidJumpIndices);

	/** Get the editor world */
	UWorld* GetEditorWorld() const;
};
