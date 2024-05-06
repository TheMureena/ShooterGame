// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API AShooterGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void GameHasEnded(class AActor* EndGameFocus = nullptr, bool bIsWinner = false) override;

	UPROPERTY(EditAnywhere)
	TArray< TSoftObjectPtr<UWorld> > LevelsReference = {};
	FLatentActionInfo LActionInfo;
	
	UPROPERTY(EditAnywhere)
	TArray<FString> Levels = {};

	void NextLevel();

	void NextLevelStream();

	void GameCompleted();

	bool IsDead() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	float RestartDelay = 5;

	FTimerHandle RestartTimer;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LoseScreenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> WinScreenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CompletedScreenClass;

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<class UUserWidget> HUDScreenClass;

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<class UUserWidget> MainMenuClass;

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<class UUserWidget> CrosshairsClass;

	//UPROPERTY()
	//UUserWidget* HUD;

	//UPROPERTY()
	//UUserWidget* MainMenu;

	//UPROPERTY()
	//UUserWidget* Crosshairs;

};
