// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "EnemyRanged.generated.h"

class AGun;
class UNiagaraSystem;
class UAnimationAsset;
class UAnimMontage;
class ASGPlayerState;

UCLASS()
class AEnemyRanged: public ACharacter, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

public:
	// Sets default values for this character's properties
	AEnemyRanged();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// Called for shooting input
	void Shoot();

	void StopShooting();

	void Reload();

	void Melee();

	//public so ABP can get ref and do stuff with isshooting
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttachmentReplication)
	class AGunBase* Gun;

	// For TurnInPlace
	UPROPERTY(BlueprintReadOnly)
	FVector2D TurnRate;

	void SubtractHealth(int32 Damage);

	void Die();

	// override Actor.h TakeDamage with our own
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	//bp pure is callable and more sort of const pure node doesnt have execution pin white
	// doesnt change anything global only outputs it produces
	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION(BlueprintPure)
	bool HasSeenPlayer() const;

	UPROPERTY()
	bool bHasSeenPlayer;

	UPROPERTY(ReplicatedUsing = OnRep_DeathMontage, EditAnywhere)
	bool bIsRagdoll = false;

	UPROPERTY()
	bool bNeedsToReload;

	UFUNCTION()
	void OnRep_DeathMontage();

	FORCEINLINE class AGunBase* GetGun() const { return Gun; }

protected:

	UPROPERTY(EditAnywhere)
	class TSubclassOf<AGunBase> DefaultGunClass = nullptr;

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100;

	UPROPERTY(VisibleAnywhere, Replicated)
	float Health = 100;

	UPROPERTY(EditAnywhere)
	float KillScore = 25;


	//						    //
   // Effects and AnimMontages //

	UPROPERTY()
	UAnimInstance* AnimationInstance;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* ShootAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* ReloadAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* DryFireAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* HitReactAnimMontage;


	//				   //
   // NOT UPROPERTIES //

	UAnimInstance* GetAnimInstEnemyRanged();
};