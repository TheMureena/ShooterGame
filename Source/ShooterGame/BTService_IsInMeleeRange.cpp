// Fill out your copyright notice in the Description page of Project Settings.


#include "BTService_IsInMeleeRange.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MonsterMelee.h"

UBTService_IsInMeleeRange::UBTService_IsInMeleeRange()
{
	NodeName = TEXT("Update InMeleeRange");
}


void UBTService_IsInMeleeRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

	AMonsterMelee* MonsterMelee = Cast<AMonsterMelee>(OwnerPawn);
	if (MonsterMelee == nullptr)
	{
		return;
	}

	if (MonsterMelee->bInMeleeRange)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), true);
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsBool(GetSelectedBlackboardKey(), false);
	}
}
