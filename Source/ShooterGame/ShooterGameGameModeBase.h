// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AShooterGameGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public: 
	AShooterGameGameModeBase();

	virtual void PawnKilled(APawn* PawnKilled);

protected:
	//virtual void BeginPlay() override;
	//virtual void PostLogin(APlayerController* NewPlayer) override;
	//void DoStartSetup();
};
