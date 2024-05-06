// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "GunBase.h"
#include "GunSingle.generated.h"

UCLASS()
class AGunSingle : public AGunBase
{
	GENERATED_BODY()

public:

	AGunSingle();

protected:
	virtual void BeginPlay() override;
};

