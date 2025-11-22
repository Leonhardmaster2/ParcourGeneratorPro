// Copyright ParkourGeneratorPro. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "ParkourPath.generated.h"

class UProceduralMeshComponent;

/**
 * Parkour Path Actor
 *
 * A spline-based path that defines the parkour route through the level.
 * The generator will create platforms along this path based on jump parameters.
 */
UCLASS(Blueprintable, BlueprintType)
class PARKOURGENERATOR_API AParkourPath : public AActor
{
	GENERATED_BODY()

public:
	AParkourPath();

	// Spline component that defines the parkour path
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour Path")
	USplineComponent* PathSpline;

	// ========== JUMP PARAMETERS ==========

	/** Maximum height the player can jump (in Unreal units, cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Jump Parameters", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float MaxJumpHeight = 200.0f;

	/** Maximum horizontal distance the player can jump (in Unreal units, cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Jump Parameters", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float MaxJumpDistance = 400.0f;

	/** Safety margin for jumps (multiplier, 0.8 = 80% of max) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Jump Parameters", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float JumpSafetyMargin = 0.85f;

	// ========== PLATFORM SETTINGS ==========

	/** Default platform width */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Platform Settings", meta = (ClampMin = "50.0"))
	float PlatformWidth = 200.0f;

	/** Default platform depth (along path direction) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Platform Settings", meta = (ClampMin = "50.0"))
	float PlatformDepth = 200.0f;

	/** Platform thickness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Platform Settings", meta = (ClampMin = "10.0"))
	float PlatformThickness = 50.0f;

	/** Minimum platforms to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Platform Settings", meta = (ClampMin = "2"))
	int32 MinPlatforms = 5;

	/** Maximum platforms to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Platform Settings", meta = (ClampMin = "2"))
	int32 MaxPlatforms = 50;

	// ========== VARIATION SETTINGS ==========

	/** Allow random variation in platform sizes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Variation")
	bool bAllowPlatformVariation = true;

	/** Platform size variation range (0.5-1.5 means 50% to 150% of base size) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Variation", meta = (ClampMin = "0.1", ClampMax = "0.5"))
	float PlatformSizeVariation = 0.3f;

	/** Random seed for reproducible generation (0 = random each time) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Variation")
	int32 RandomSeed = 0;

	// ========== VISUALIZATION ==========

	/** Show debug visualization of jump arcs */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Debug")
	bool bShowJumpArcs = true;

	/** Show platform preview before generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Parkour|Debug")
	bool bShowPlatformPreview = true;

	// ========== GENERATED DATA ==========

	/** Array of generated platform locations */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Parkour|Generated")
	TArray<FTransform> GeneratedPlatformTransforms;

	/** Array of spawned platform meshes */
	UPROPERTY()
	TArray<UProceduralMeshComponent*> GeneratedPlatformMeshes;

	// ========== FUNCTIONS ==========

	/** Generate platforms along the path */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void GenerateParkour();

	/** Clear all generated platforms */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void ClearGeneratedParkour();

	/** Validate jump between two points */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	bool IsJumpValid(const FVector& FromPoint, const FVector& ToPoint) const;

	/** Get safe jump distance considering margin */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	float GetSafeJumpDistance() const { return MaxJumpDistance * JumpSafetyMargin; }

	/** Get safe jump height considering margin */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	float GetSafeJumpHeight() const { return MaxJumpHeight * JumpSafetyMargin; }

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Calculate platform positions along spline */
	TArray<FTransform> CalculatePlatformPositions();

	/** Add intermediate platforms if jump is too far */
	void AddIntermediatePlatforms(TArray<FTransform>& Platforms, const FTransform& From, const FTransform& To);

	/** Create a platform mesh at the given transform */
	UProceduralMeshComponent* CreatePlatformMesh(const FTransform& PlatformTransform, int32 Index);

	/** Update preview visualization */
	void UpdatePreview();

	FRandomStream RandomStream;
};
