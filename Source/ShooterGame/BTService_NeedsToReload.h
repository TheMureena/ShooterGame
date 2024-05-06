// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlackboardBase.h"
#include "BTService_NeedsToReload.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UBTService_NeedsToReload : public UBTService_BlackboardBase
{
	GENERATED_BODY()
public:
	UBTService_NeedsToReload();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds);
};
