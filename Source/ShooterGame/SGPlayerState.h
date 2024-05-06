// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SGPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnScoreChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClientsReady);

UCLASS()
class SHOOTERGAME_API ASGPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ASGPlayerState();

protected:

	UPROPERTY(BlueprintAssignable)
	FOnScoreChanged OnScoreChanged;

	UPROPERTY(BlueprintAssignable)
	FOnClientsReady OnClientsReady;

	UPROPERTY(Replicated, EditDefaultsOnly)//Set To Replicate MOVE TO PLAYER STATE WHEN CREATED
	int32 PlayerScore;

	void OnNewPlayerJoined();

public:

	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void Client_OnClientsReady();
	void Client_OnClientsReady_Implementation();

	void IncrementScore(uint16 Value);

	UFUNCTION(BlueprintCallable)
	void UpdateScoreWidget();

	UFUNCTION(BlueprintCallable)
	int32 GetPlayerScore();

	bool IsDead();
};
