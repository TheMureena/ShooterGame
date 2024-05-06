// Fill out your copyright notice in the Description page of Project Settings.


#include "SGPlayerState.h"
#include "ShooterCharacter.h"

#include "Net/UnrealNetwork.h"

ASGPlayerState::ASGPlayerState()
{
	PlayerScore = 0;
}

void ASGPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASGPlayerState, PlayerScore);
}

void ASGPlayerState::OnNewPlayerJoined()
{
}

void ASGPlayerState::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TPlayerScoreUpdate;
	GetWorld()->GetTimerManager().SetTimer(TPlayerScoreUpdate, this, &ASGPlayerState::UpdateScoreWidget, 0.05f, true);
}

void ASGPlayerState::Client_OnClientsReady_Implementation()
{
	OnClientsReady.Broadcast();
}

void ASGPlayerState::IncrementScore(uint16 Value)
{
	PlayerScore += Value;

	OnScoreChanged.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("SCORE: %d"), PlayerScore);
}

void ASGPlayerState::UpdateScoreWidget()
{
	OnScoreChanged.Broadcast();
}

int32 ASGPlayerState::GetPlayerScore()
{
	return PlayerScore;
}

bool ASGPlayerState::IsDead()
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(GetPawn());
	return Character->IsDead();
}
