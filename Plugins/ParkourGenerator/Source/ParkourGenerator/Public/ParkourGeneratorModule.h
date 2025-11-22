// Copyright ParkourGeneratorPro. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FParkourGeneratorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterMenuExtensions();
	void UnregisterMenuExtensions();

	TSharedPtr<class FUICommandList> PluginCommands;
};
