// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GunBase.generated.h"

// Delegate signature
DECLARE_DELEGATE_OneParam(FOnSpreadAmountChangedSignature, float /* NewSpreadAmount */ );

class UNiagaraSystem;
class UMetaSoundSource;
class USoundAttenuation;
class USoundConcurrency;
class UAnimMontage;

#if WITH_EDITOR
class UDbg;
#endif

UCLASS()
class SHOOTERGAME_API AGunBase : public AActor
{
	GENERATED_BODY()
	
public:

	// We want this public so our UI can access it to subscribe to it
	FOnSpreadAmountChangedSignature OnSpreadAmountChangedDelegate;

	// Sets default values for this actor's properties
	AGunBase();

protected:

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	USkeletalMeshComponent* Mesh;

	// For delay between shots when clicking rapidly
	bool bCanFire;

	void ControlFireDelay();

public:

	UFUNCTION(BlueprintCallable)
	int32 GetTotalAmmoCount() const;

	UFUNCTION(BlueprintCallable)
	int32 GetMagazineCurrent() const;

	UFUNCTION(BlueprintCallable)
	int32 GetMagazineSize() const;

	UFUNCTION(BlueprintCallable)
	FString GetGunName() const;

	AController* GetOwnerController() const;

public:

	virtual void Fire();

	virtual void StopFiring();

	virtual void Reload();

	virtual bool IsFiring() { return bIsFiring; }

	// Function signature
	void OnSpreadAmountChanged(float NewSpreadAmount);

	/** Returns Mesh Subobject **/
	FORCEINLINE class USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetGunMesh();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//				//
	//  Gun Stats  //

	UPROPERTY(EditAnywhere, Category = "Gun Stats")
	float MaxRange = 5000;

	UPROPERTY(EditAnywhere, Category = "Gun Stats")
	float Damage = 25;

	// set the recoil amount how fast it accumulates
	UPROPERTY(EditAnywhere, Category = "Gun Stats", meta = (ClampMin = 0, UIMin = 0, ClampMax = 1, UIMax = 1))
	float RecoilAmount = 0.3f;

	UPROPERTY()
	int32 RandomX;

	UPROPERTY()
	int32 RandomY;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gun Stats", meta = (ClampMin = 0, UIMin = 0))
	int32 MagazineSize = 30;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Gun Stats", meta = (ClampMin = 0, UIMin = 0))
	int32 MagazineCurrent = MagazineSize;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Gun Stats", meta=(ClampMin=0, UIMin = 0))
	int32 TotalAmmoCount = 0;

	UPROPERTY(EditAnywhere, Category = "Gun Stats")
	FString WeaponName = "";

	// Delay between mouse clicks
	UPROPERTY(EditAnywhere)
	float DelayFire;
	UPROPERTY()
	float SpreadAmount = 0;

	UFUNCTION(BlueprintCallable)
	void RefillAmmo();

protected:

	TArray<FHitResult> DoLineTrace(class AShooterCharacter* ShootingPlayer, FVector& ShotDirection);
	TArray<FHitResult> DoLineTrace(FVector MuzzleLocation, FRotator MuzzleRotation);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire(const TArray<FHitResult>& HitResults, FVector ShotDirection);
	bool Server_Fire_Validate(const TArray<FHitResult>& HitResults, FVector ShotDirection);
	virtual void Server_Fire_Implementation(const TArray<FHitResult>& HitResults, FVector ShotDirection);

	// For Replicating to all clients anims, fx,sfx etc.. multicast //
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_Fire(const FHitResult& HitResult, FVector ShotDirection);
	bool Multicast_Fire_Validate(const FHitResult& HitResult, FVector ShotDirection);
	virtual void Multicast_Fire_Implementation(const FHitResult& HitResult, FVector ShotDirection);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();
	bool Server_Reload_Validate();
	virtual void Server_Reload_Implementation();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_Reload();
	bool Multicast_Reload_Validate();
	virtual void Multicast_Reload_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_RefillAmmo();
	void Client_RefillAmmo_Implementation();

private:
	bool bIsFiring;

protected:

	 //				   //
	// Gun Effects FX //

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ShellEject;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactFXEnemy;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactDecalFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	FRotator MuzzleFlashRotationOffset;

	UPROPERTY(EditAnywhere, Category = "Firing")
	FRotator ShellEjectRotationOffset;

	 //					   //
	// Animation Montages //

	UPROPERTY(EditAnywhere, Category = "Anim Montages")
	class UAnimMontage* ShootAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Anim Montages")
	class UAnimMontage* ReloadAnimMontage;


	UPROPERTY(EditAnywhere, Category = "Char Anim Montages")
	class UAnimMontage* CharShootAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Char Anim Montages")
	class UAnimMontage* CharReloadAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Char Anim Montages")
	class UAnimMontage* CharDryFireAnimMontage;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_Shoot;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_HitReact;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_ShootSniper;

	/// debugger class ///
#if WITH_EDITOR
	UDbg* Dbg;
#endif
};
