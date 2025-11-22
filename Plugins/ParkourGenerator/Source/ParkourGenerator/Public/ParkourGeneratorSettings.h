// Copyright ParkourGeneratorPro. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ParkourGeneratorSettings.generated.h"

/**
 * Parkour Generator Plugin Settings
 *
 * Configure default values for the Parkour Generator tool.
 * Access via Project Settings -> Plugins -> Parkour Generator
 */
UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "Parkour Generator"))
class PARKOURGENERATOR_API UParkourGeneratorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UParkourGeneratorSettings();

	// ========== DEFAULT JUMP PARAMETERS ==========

	/** Default maximum jump height for new paths (cm) */
	UPROPERTY(config, EditAnywhere, Category = "Default Jump Parameters", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
	float DefaultMaxJumpHeight = 200.0f;

	/** Default maximum jump distance for new paths (cm) */
	UPROPERTY(config, EditAnywhere, Category = "Default Jump Parameters", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
	float DefaultMaxJumpDistance = 400.0f;

	/** Default safety margin for jumps */
	UPROPERTY(config, EditAnywhere, Category = "Default Jump Parameters", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float DefaultJumpSafetyMargin = 0.85f;

	// ========== DEFAULT PLATFORM SETTINGS ==========

	/** Default platform width */
	UPROPERTY(config, EditAnywhere, Category = "Default Platform Settings", meta = (ClampMin = "50.0"))
	float DefaultPlatformWidth = 200.0f;

	/** Default platform depth */
	UPROPERTY(config, EditAnywhere, Category = "Default Platform Settings", meta = (ClampMin = "50.0"))
	float DefaultPlatformDepth = 200.0f;

	/** Default platform thickness */
	UPROPERTY(config, EditAnywhere, Category = "Default Platform Settings", meta = (ClampMin = "10.0"))
	float DefaultPlatformThickness = 50.0f;

	// ========== DEFAULT ENVIRONMENT SETTINGS ==========

	/** Default building density */
	UPROPERTY(config, EditAnywhere, Category = "Default Environment Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DefaultBuildingDensity = 0.5f;

	/** Default path clearance for buildings */
	UPROPERTY(config, EditAnywhere, Category = "Default Environment Settings", meta = (ClampMin = "100.0"))
	float DefaultPathClearance = 500.0f;

	// ========== PRESETS ==========

	/** Preset: Character can jump 150cm high, 350cm far (realistic) */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	static void GetRealisticJumpPreset(float& OutMaxHeight, float& OutMaxDistance)
	{
		OutMaxHeight = 150.0f;
		OutMaxDistance = 350.0f;
	}

	/** Preset: Character can jump 250cm high, 500cm far (athletic) */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	static void GetAthleticJumpPreset(float& OutMaxHeight, float& OutMaxDistance)
	{
		OutMaxHeight = 250.0f;
		OutMaxDistance = 500.0f;
	}

	/** Preset: Character can jump 400cm high, 800cm far (superhuman) */
	UFUNCTION(BlueprintCallable, Category = "Presets")
	static void GetSuperhumanJumpPreset(float& OutMaxHeight, float& OutMaxDistance)
	{
		OutMaxHeight = 400.0f;
		OutMaxDistance = 800.0f;
	}

	// Access singleton
	UFUNCTION(BlueprintCallable, Category = "Settings")
	static const UParkourGeneratorSettings* Get()
	{
		return GetDefault<UParkourGeneratorSettings>();
	}

	// Category and section name
	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
