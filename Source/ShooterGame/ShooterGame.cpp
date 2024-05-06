// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, ShooterGame, "ShooterGame" );

DEFINE_LOG_CATEGORY(ShooterGame_INFO);
DEFINE_LOG_CATEGORY(ShooterGame_INIT);
DEFINE_LOG_CATEGORY(ShooterGame_CRIT);
DEFINE_LOG_CATEGORY(ShooterGame_WARN);