//////


#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterGameAnimInstance.generated.h"


UCLASS(Config = Game)
class UShooterGameAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UShooterGameAnimInstance(const FObjectInitializer& ObjectInitializer);

	/** Set in Gun.cpp*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MyVariables)
	bool IsShooting = false;

	/** Set in Gun.cpp*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = MyVariables)
	bool IsReloading = false;



};
       
