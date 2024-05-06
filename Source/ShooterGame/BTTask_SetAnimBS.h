// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetAnimBS.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTERGAME_API UBTTask_SetAnimBS : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UBTTask_SetAnimBS();

protected:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};