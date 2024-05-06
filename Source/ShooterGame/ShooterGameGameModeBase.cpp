// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterGameGameModeBase.h"
#include "ShooterCharacter.h"
#include "UObject/ConstructorHelpers.h"

AShooterGameGameModeBase::AShooterGameGameModeBase()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP/BP_ShooterCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	///AGameModeBase::bUseSeamlessTravel = true;
}

void AShooterGameGameModeBase::PawnKilled(APawn* PawnKilled)
{
	
}
//
//void AShooterGameGameModeBase::BeginPlay()
//{
//
//}
//
//void AShooterGameGameModeBase::PostLogin(APlayerController* NewPlayer)
//{
//}
//
//void AShooterGameGameModeBase::DoStartSetup()
//{
//	
//}
