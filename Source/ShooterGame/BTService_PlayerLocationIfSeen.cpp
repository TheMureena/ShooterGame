// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_PlayerLocationIfSeen.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Gameframework/GameStateBase.h"
#include "Gameframework/PlayerState.h"

UBTService_PlayerLocationIfSeen::UBTService_PlayerLocationIfSeen()
{
	NodeName = TEXT("Update Player Location If Seen");
}

void UBTService_PlayerLocationIfSeen::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (LocalPlayerPawn == nullptr)
	{
		return;
	}

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return;
	}

	int32 NumPlayers = UGameplayStatics::GetNumPlayerStates(GetWorld());

	AGameStateBase* GS = UGameplayStatics::GetGameState(GetWorld());

	TArray<APawn*> PlayerPawns;

	for (APlayerState* PlayerState : GS->PlayerArray)
	{
		APawn* PlayerPawn = PlayerState->GetPawn();
		PlayerPawns.Add(PlayerPawn);
		UE_LOG(LogTemp, Warning, TEXT("Player: %s"),*PlayerPawn->GetName());
	}

	for (APawn* PlayerPawn : PlayerPawns)
	{
		if (OwnerComp.GetAIOwner()->LineOfSightTo(PlayerPawn))
		{
			OwnerComp.GetBlackboardComponent()->SetValueAsObject(GetSelectedBlackboardKey(), PlayerPawn);
		}
		else
		{
			OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
		}
	}
}
