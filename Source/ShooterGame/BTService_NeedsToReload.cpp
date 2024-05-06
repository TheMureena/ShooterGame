// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_NeedsToReload.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyRanged.h"


UBTService_NeedsToReload::UBTService_NeedsToReload()
{
	NodeName = TEXT("Update bNeedsToReload");
}

void UBTService_NeedsToReload::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	if (OwnerComp.GetAIOwner() == nullptr)
	{
		return;
	}

	APawn* OwnerPawn = OwnerComp.GetAIOwner()->GetPawn();
	if (OwnerPawn == nullptr)
	{
		return;
	}

	AEnemyRanged* EnemyRanged = Cast<AEnemyRanged>(OwnerPawn);
	if (EnemyRanged == nullptr)
	{
		return;
	}

	if (EnemyRanged->bNeedsToReload)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}
