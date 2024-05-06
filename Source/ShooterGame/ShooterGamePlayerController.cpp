// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGamePlayerController.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Crosshair_Widget.h"
#include "Kismet/GameplayStatics.h"
#include "ShooterCharacter.h"

void AShooterGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
}

bool AShooterGamePlayerController::IsDead() const
{
	AShooterCharacter* ControlledCharacter = Cast<AShooterCharacter>(GetPawn());
	if (ControlledCharacter != nullptr)
	{
		return ControlledCharacter->IsDead();
	}

	return true;
}

void AShooterGamePlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
{
	Super::GameHasEnded(EndGameFocus, bIsWinner);

	if (bIsWinner)
	{
		UUserWidget* WinScreen = CreateWidget(this, WinScreenClass);
		if (WinScreen != nullptr)
		{
			WinScreen->AddToViewport();
			// open next level with timer
			GetWorldTimerManager().SetTimer(RestartTimer, this, &AShooterGamePlayerController::NextLevel, RestartDelay);
		}
	}
	else
	{
		UUserWidget* LoseScreen = CreateWidget(this, LoseScreenClass);
		if (LoseScreen != nullptr)
		{
			LoseScreen->AddToViewport();
			// restart level with timer
			GetWorldTimerManager().SetTimer(RestartTimer, this, &APlayerController::RestartLevel, RestartDelay);
		}
	}
}

void AShooterGamePlayerController::NextLevel()
{
	FName NextLevelName;
	int LevelsAmount = Levels.Num();
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());

	for (int LevelIndex = 0; LevelIndex < LevelsAmount; LevelIndex++)
	{
		// check if last level remember -1 to avoid out of bounds!!
		if (CurrentLevelName == Levels[LevelsAmount-1])
		{
			UUserWidget* CompletedScreen = CreateWidget(this, CompletedScreenClass);
			if (CompletedScreen != nullptr)
			{
				CompletedScreen->AddToViewport();
				GetWorldTimerManager().SetTimer(RestartTimer, this, &AShooterGamePlayerController::GameCompleted, RestartDelay);
				break;
			}
		}
		else if (CurrentLevelName == Levels[LevelIndex])
		{
			NextLevelName = FName(Levels[LevelIndex + 1]);
			FString NextLevelNameString = NextLevelName.ToString();
			UE_LOG(LogTemp, Warning, TEXT("NextLevelName: %s"), *NextLevelNameString);
			UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
			break;
		}
	}
}

void AShooterGamePlayerController::NextLevelStream()
{
	FName NextLevelName;
	int LevelsAmount = Levels.Num();
	FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(GetWorld());

	for (int LevelIndex = 0; LevelIndex < LevelsAmount; LevelIndex++)
	{
		// check if last level remember -1 to avoid out of bounds!!
		if (CurrentLevelName == Levels[LevelsAmount - 1])
		{
			// if dev_build or editor start first level again
			#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
				NextLevelName = FName(Levels[0]);
				FString NextLevelNameString = NextLevelName.ToString();
				UE_LOG(LogTemp, Warning, TEXT("NextLevelName: %s"), *NextLevelNameString);
				break;
			#endif

			UUserWidget* CompletedScreen = CreateWidget(this, CompletedScreenClass);
			if (CompletedScreen != nullptr)
			{
				CompletedScreen->AddToViewport();
				GetWorldTimerManager().SetTimer(RestartTimer, this, &AShooterGamePlayerController::GameCompleted, RestartDelay);
				break;
			}
		}
		else if (CurrentLevelName == Levels[LevelIndex])
		{
			NextLevelName = FName(Levels[LevelIndex + 1]);
			FString NextLevelNameString = NextLevelName.ToString();
			UE_LOG(LogTemp, Warning, TEXT("NextLevelName: %s"), *NextLevelNameString);
			UGameplayStatics::OpenLevel(GetWorld(), NextLevelName);
			break;
		}
	}
}

void AShooterGamePlayerController::GameCompleted()
{
	Pause();
}
