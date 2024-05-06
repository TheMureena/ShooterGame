// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PlayerLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Gameframework/GameStateBase.h"
#include "Gameframework/PlayerState.h"

UBTService_PlayerLocation::UBTService_PlayerLocation()
{
	NodeName = TEXT("Update Player Location");
}

void UBTService_PlayerLocation::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	if (LocalPlayerPawn == nullptr)
	{
		return;
	}

	AGameStateBase* GS = UGameplayStatics::GetGameState(GetWorld());

	TArray<APawn*> PlayerPawns;

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		APawn* PlayerPawn = PlayerState->GetPawn();
		PlayerPawns.Add(PlayerPawn);
		UE_LOG(LogTemp, Warning, TEXT("Player: %s"), *PlayerPawn->GetName());
	}

	OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), PlayerPawns[0]->GetActorLocation());
}
