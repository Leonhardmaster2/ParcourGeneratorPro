// Copyright ParkourGeneratorPro. All Rights Reserved.

#include "ParkourGeneratorModule.h"
#include "LevelEditor.h"
#include "ToolMenus.h"
#include "ParkourPath.h"
#include "ParkourPlayArea.h"
#include "ParkourGeneratorSettings.h"

#define LOCTEXT_NAMESPACE "FParkourGeneratorModule"

void FParkourGeneratorModule::StartupModule()
{
	// Register slate style overrides if needed
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FParkourGeneratorModule::RegisterMenuExtensions));
}

void FParkourGeneratorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	UnregisterMenuExtensions();
}

void FParkourGeneratorModule::RegisterMenuExtensions()
{
	// Register menu entries in the Level Editor
	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");

	FToolMenuSection& Section = Menu->FindOrAddSection("ParkourGenerator");
	Section.Label = LOCTEXT("ParkourGeneratorSection", "Parkour Generator");

	Section.AddMenuEntry(
		"SpawnParkourPath",
		LOCTEXT("SpawnParkourPath", "Spawn Parkour Path"),
		LOCTEXT("SpawnParkourPathTooltip", "Spawns a new Parkour Path spline actor in the level"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			if (GEditor && GEditor->GetEditorWorldContext().World())
			{
				UWorld* World = GEditor->GetEditorWorldContext().World();
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AParkourPath* NewPath = World->SpawnActor<AParkourPath>(AParkourPath::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (NewPath)
				{
					GEditor->SelectNone(true, true);
					GEditor->SelectActor(NewPath, true, true);
				}
			}
		}))
	);

	Section.AddMenuEntry(
		"SpawnParkourPlayArea",
		LOCTEXT("SpawnParkourPlayArea", "Spawn Play Area"),
		LOCTEXT("SpawnParkourPlayAreaTooltip", "Spawns a new Parkour Play Area volume in the level"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			if (GEditor && GEditor->GetEditorWorldContext().World())
			{
				UWorld* World = GEditor->GetEditorWorldContext().World();
				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

				AParkourPlayArea* NewArea = World->SpawnActor<AParkourPlayArea>(AParkourPlayArea::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
				if (NewArea)
				{
					GEditor->SelectNone(true, true);
					GEditor->SelectActor(NewArea, true, true);
				}
			}
		}))
	);
}

void FParkourGeneratorModule::UnregisterMenuExtensions()
{
	// Cleanup if needed
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FParkourGeneratorModule, ParkourGenerator)
