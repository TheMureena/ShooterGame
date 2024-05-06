// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GunBase.h"
#include "GunAuto.generated.h"

class UNiagaraSystem;
class UMaterialInstance;
//class UMetaSoundSource;
//class USoundBase;
//class USoundAttenuation;
//class USoundConcurrency;
class UAnimMontage;

#if WITH_EDITOR
class UDbg;
#endif

UCLASS()
class SHOOTERGAME_API AGunAuto : public AGunBase
{
	GENERATED_BODY()
	
public:

	// Sets default values for this actor's properties
	AGunAuto();

	UFUNCTION(BlueprintCallable)
	float GetFireRate() const;

public:
	virtual void Fire() override;

	virtual void StopFiring() override;

	virtual bool IsFiring() override { return bIsFiring; }

protected:
	// For automatic fire fire rate control, public for crosshair widget
	//the fire rate, fortimer = 60 / FireRate
	UPROPERTY(EditAnywhere, Category = "Gun Stats")
	float FireRate = 500;

	UPROPERTY(ReplicatedUsing = OnRep_StartFullAutoFire)//replicated using is for replicating effects on other clients
	bool bIsFiring;

	UFUNCTION()
	void OnRep_StartFullAutoFire();

	FTimerHandle GunFireHandle;
	void PlayGunFX();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_StartFullAutoFire(bool IsFiring);
	bool Server_StartFullAutoFire_Validate(bool IsFiring);
	void Server_StartFullAutoFire_Implementation(bool IsFiring);

	virtual void Server_Fire_Implementation(const TArray<FHitResult>& HitResults, FVector ShotDirection) override;

	void OnClientFire();

protected:

	/// debugger class ///
#if WITH_EDITOR
	UDbg* Dbg;
#endif
};
