// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "GrapplingHook.generated.h"

// Delegate signature
DECLARE_DELEGATE_OneParam(FOnGrappleSignature, float /* GrappleForce */);

class UNiagaraSystem;
class UMaterialInstance;
class UMetaSoundSource;
class USoundBase;
class USoundAttenuation;
class USoundConcurrency;
class UAnimMontage;

#if UE_BUILD_DEVELOPMENT || WITH_EDITOR 
class UDbg;
#endif

UCLASS()
class SHOOTERGAME_API AGrapplingHook : public AActor, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

public:

	// We want this public so our character can access it to subscribe to it
	FOnGrappleSignature OnGrappleDelegate;

	// Sets default values for this actor's properties
	AGrapplingHook();

	void Fire();

	void FireGrapple();

	void ReleaseGrapple();

	void ReturnCable();

	UPROPERTY(Replicated)
	bool bIsAttached;

	UPROPERTY(Replicated)
	bool CableReturning;

	bool HasHit() const;

	FVector GrabPoint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// For replication 
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Fire();
	bool Server_Fire_Validate();
	virtual void Server_Fire_Implementation();

	// For Replicating to all clients anims, fx,sfx etc.. multicast //
	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multicast_Fire();
	bool Multicast_Fire_Validate();
	virtual void Multicast_Fire_Implementation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* HookMesh;

	UPROPERTY(VisibleAnywhere)
	class UCableComponent* GrappleCable;

	// For delay between shots when clicking rapidly
	UPROPERTY()
	bool bCanFire;

	void ControlFireDelay();

	 //				   //
	// Grapple Effects FX //

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* MuzzleFlash;

	//UPROPERTY(EditAnywhere, Category = "Firing")
	//class UNiagaraSystem* ShellEject;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactFXEnemy;

	UPROPERTY(EditAnywhere, Category = "Firing")
	UMaterialInstance* ImpactDecals;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class UNiagaraSystem* ImpactDecalFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	FRotator MuzzleFlashRotationOffset;

	UPROPERTY(EditAnywhere, Category = "Firing")
	FRotator ShellEjectRotationOffset;


	 //					    //
	// Grapple Effects SFX //

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundBase* ShotSFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundBase* MissSFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundBase* ImpactSFX;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundBase* ImpactSFXEnemy;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundAttenuation* ImpactAttenuation;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundAttenuation* ShootAttenuation;

	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundConcurrency* ImpactConcurrency;

	// this should be different for player and enemies far away etc.
	UPROPERTY(EditAnywhere, Category = "Firing")
	class USoundConcurrency* ShootConcurrency;

     //				    //
	//  Grapple Stats  //

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float MaxRange = 1600.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float GrappleForce = 10000.0f;

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float Damage = 200;

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float DelayFire;

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float DelayMissReturn;

	UPROPERTY(EditAnywhere, Category = "Grapple Stats")
	float ReturnSpeed = 2000.0f;

	//				     //
   //  NOT UPROPERTIES  //

	bool GrappleTrace(FHitResult& Hit, FVector& ShotDirection);

	AController* GetOwnerController() const;

	FTimerHandle TGunFireDelayHandle;

	FTimerHandle TGrappleMissReturnHandle;

	/// debugger class ///
#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
	UDbg* Dbg;
#endif
};
