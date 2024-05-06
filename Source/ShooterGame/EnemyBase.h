// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyBase.generated.h"

UCLASS()
class SHOOTERGAME_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void SubtractHealth(int32 Damage);

	void Die();

protected:

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100;

	UPROPERTY(VisibleAnywhere, Replicated)
	float Health = 100;

	UPROPERTY(EditAnywhere)
	float KillScore = 25;

	UPROPERTY(ReplicatedUsing = OnRep_Die, EditAnywhere)
	bool bIsDead;

	UPROPERTY()
	bool bIsRagdoll;

	UFUNCTION()
	void OnRep_Die();

public:
	void Hit(class AShooterCharacter* Player, FHitResult HitResult);

	// override Actor.h TakeDamage with our own
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;



	//						    //
	// Effects and AnimMontages //

	//UPROPERTY(EditAnywhere, Category = "Animations")
	//class UAnimMontage* ShootAnimMontage;

	//UPROPERTY(EditAnywhere, Category = "Animations")
	//class UAnimMontage* ReloadAnimMontage;

	//UPROPERTY(EditAnywhere, Category = "Animations")
	//class UAnimMontage* DryFireAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* HitReactAnimMontage;
};

