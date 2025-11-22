// Copyright ParkourGeneratorPro. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SplineComponent.h"
#include "ParkourPlayArea.generated.h"

class UProceduralMeshComponent;
class AParkourPath;

/** Type of area shape */
UENUM(BlueprintType)
enum class EParkourAreaShape : uint8
{
	Box UMETA(DisplayName = "Box"),
	LShape UMETA(DisplayName = "L-Shape"),
	UShape UMETA(DisplayName = "U-Shape"),
	Custom UMETA(DisplayName = "Custom Spline")
};

/** Type of environment fill */
UENUM(BlueprintType)
enum class EParkourEnvironmentStyle : uint8
{
	None UMETA(DisplayName = "None"),
	Urban UMETA(DisplayName = "Urban/City"),
	Industrial UMETA(DisplayName = "Industrial"),
	Abstract UMETA(DisplayName = "Abstract Blockout")
};

/**
 * Parkour Play Area Actor
 *
 * Defines the bounds of the play area and generates environment fill geometry.
 */
UCLASS(Blueprintable, BlueprintType)
class PARKOURGENERATOR_API AParkourPlayArea : public AActor
{
	GENERATED_BODY()

public:
	AParkourPlayArea();

	// ========== AREA BOUNDS ==========

	/** Shape of the play area */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Shape")
	EParkourAreaShape AreaShape = EParkourAreaShape::Box;

	/** Box component for simple box bounds */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Play Area|Shape")
	UBoxComponent* BoundsBox;

	/** Spline component for custom shape bounds */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Play Area|Shape")
	USplineComponent* CustomBoundsSpline;

	/** Area dimensions for box/L/U shapes (X=Length, Y=Width, Z=Height) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Shape", meta = (ClampMin = "100.0"))
	FVector AreaDimensions = FVector(5000.0f, 5000.0f, 2000.0f);

	/** For L-Shape: Width of the secondary arm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Shape", meta = (EditCondition = "AreaShape == EParkourAreaShape::LShape", ClampMin = "100.0"))
	float LShapeArmWidth = 2000.0f;

	/** For U-Shape: Width of the arms */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Shape", meta = (EditCondition = "AreaShape == EParkourAreaShape::UShape", ClampMin = "100.0"))
	float UShapeArmWidth = 1500.0f;

	// ========== ENVIRONMENT FILL ==========

	/** Style of environment fill to generate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment")
	EParkourEnvironmentStyle EnvironmentStyle = EParkourEnvironmentStyle::Urban;

	/** Reference to the parkour path (for avoiding path area) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment")
	AParkourPath* ParkourPath;

	/** Minimum distance from path to place buildings */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment", meta = (ClampMin = "100.0"))
	float PathClearance = 500.0f;

	/** Building density (0-1, affects number of buildings) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BuildingDensity = 0.5f;

	/** Minimum building size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment", meta = (ClampMin = "100.0"))
	FVector MinBuildingSize = FVector(300.0f, 300.0f, 400.0f);

	/** Maximum building size */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment", meta = (ClampMin = "100.0"))
	FVector MaxBuildingSize = FVector(1000.0f, 1000.0f, 2000.0f);

	/** Random seed for environment generation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Environment")
	int32 EnvironmentSeed = 0;

	// ========== FLOOR ==========

	/** Generate a floor plane */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Floor")
	bool bGenerateFloor = true;

	/** Floor offset below play area origin */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Floor", meta = (EditCondition = "bGenerateFloor"))
	float FloorOffset = 0.0f;

	// ========== VISUALIZATION ==========

	/** Show bounds visualization */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Debug")
	bool bShowBounds = true;

	/** Show environment preview */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Play Area|Debug")
	bool bShowEnvironmentPreview = false;

	// ========== GENERATED DATA ==========

	/** Generated building meshes */
	UPROPERTY()
	TArray<UProceduralMeshComponent*> GeneratedBuildingMeshes;

	/** Generated floor mesh */
	UPROPERTY()
	UProceduralMeshComponent* GeneratedFloorMesh;

	// ========== FUNCTIONS ==========

	/** Generate environment fill within the play area */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void GenerateEnvironment();

	/** Clear all generated environment */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void ClearGeneratedEnvironment();

	/** Generate everything (platforms + environment) */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void GenerateAll();

	/** Clear everything */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Parkour Generator")
	void ClearAll();

	/** Check if a point is within the play area */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	bool IsPointInPlayArea(const FVector& Point) const;

	/** Get random point within play area */
	UFUNCTION(BlueprintCallable, Category = "Parkour Generator")
	FVector GetRandomPointInArea(FRandomStream& RandomStream) const;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Update bounds visualization */
	void UpdateBoundsVisualization();

	/** Generate buildings for urban style */
	void GenerateUrbanBuildings(FRandomStream& RandomStream);

	/** Generate structures for industrial style */
	void GenerateIndustrialStructures(FRandomStream& RandomStream);

	/** Generate abstract blockout */
	void GenerateAbstractBlockout(FRandomStream& RandomStream);

	/** Create a building mesh */
	UProceduralMeshComponent* CreateBuildingMesh(const FVector& Location, const FVector& Size, int32 Index);

	/** Create floor mesh */
	void CreateFloorMesh();

	/** Check if building placement is valid (not blocking path) */
	bool IsPlacementValid(const FVector& Location, const FVector& Size) const;

	/** Get area bounds as box */
	FBox GetAreaBounds() const;
};
