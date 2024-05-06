// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MonsterMelee.generated.h"

class UNiagaraSystem;
class UAnimationAsset;
class UAnimMontage;
class ASGPlayerState;

UCLASS()
class SHOOTERGAME_API AMonsterMelee : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AMonsterMelee();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_DeathMontage();

public:

	void MeleeAttack();

	UFUNCTION()
	void OnDealDamageOverlapBegin(class UPrimitiveComponent* OverlappedComp,
		class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// override Actor.h TakeDamage with our own
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	//bp pure is callable and more sort of const pure node doesnt have execution pin white
	// doesnt change anything global only outputs it produces
	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UPROPERTY(BlueprintReadWrite)
	bool CanDealDamage;

	UPROPERTY()
	bool bInMeleeRange;

	UPROPERTY(ReplicatedUsing = OnRep_DeathMontage, EditAnywhere)
	bool bIsRagdoll = false;

	class AShooterCharacter* PlayerREF;

protected:

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100;

	UPROPERTY(VisibleAnywhere, Replicated)
	float Health = 100;

	UPROPERTY(EditAnywhere)
	float MeleeDamage = 25;

	UPROPERTY(EditAnywhere)
	float KillScore = 25;

	UPROPERTY(VisibleAnywhere)
	float MaxRange = 200.0f;

	void SubtractHealth(int32 Damage);

	void Die();

	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* MovementCollison;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* DamageCollision;

	 //							 //
	// Effects and AnimMontages //

	UPROPERTY(EditAnywhere)
	class UAnimMontage* MeleeAnimMontage;

	UPROPERTY()
	UAnimInstance* AnimInstance;

	//					//
   // NOT UPROPERTIES  //

	bool MeleeRangeTrace(FHitResult& Hit, FVector& AttackDirection);

	UAnimInstance* GetAnimInstMonster();
};
