// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ShooterGameSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterGame_Types.h"

#define _LOG_TO_SCREEN true	
static constexpr EDebugLevels s_BUILD_DEBUG_LEVEL = EDebugLevels::DEBUGTEXTANDSHAPES;

DECLARE_LOG_CATEGORY_EXTERN(ShooterGame_INFO, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(ShooterGame_INIT, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(ShooterGame_CRIT, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(ShooterGame_WARN, Log, All);