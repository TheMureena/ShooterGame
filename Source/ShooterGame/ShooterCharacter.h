// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "VisualLogger/VisualLoggerDebugSnapshotInterface.h"
#include "ShooterCharacter.generated.h"

class AGunBase;
class AGrapplingHook;
class UNiagaraSystem;
class UAnimationAsset;
class UShooterGameSaveGame;
class UCrosshair_Widget;

#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
class UDbg;
#endif

USTRUCT(BlueprintType)
struct FMontageData
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UAnimMontage* AnimMontage = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PlayRate = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FName StartSectionName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float TriggeredTime = 0.0f;
};

// Delegate signature
DECLARE_DELEGATE_OneParam(FOnVelocityChangedSignature, float /* NewVelocity */);

DECLARE_DELEGATE(FOnSwitchGun);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInteractChanged, const FString&, NewInteractMessage);

UCLASS(config=Game)
class AShooterCharacter : public ACharacter, public IVisualLoggerDebugSnapshotInterface
{
	GENERATED_BODY()

#if ENABLE_VISUAL_LOG
	virtual void GrabDebugSnapshot(FVisualLogEntry* Snapshot) const override;
#endif

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** camera manager */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class APlayerCameraManager* CameraManager;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Crouch Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SlideAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	//Shoot Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShootAction;

	//Automatic Shoot Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ShootAutoAction;

	//Reload Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* ReloadAction;

	//Switch Weapon Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SwitchWeaponAction;

	//Switch Weapon slot 1
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot1;

	//Switch Weapon slot 2
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot2;

	//Switch Weapon slot 3
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot3;

	//Switch Weapon slot 4
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot4;

	//Switch Weapon slot 5
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot5;

	//Switch Weapon slot 6
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GunSlot6;

	//Melee Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MeleeAction;

	//Run Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* RunAction;

	//Grappling hook Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* GrapplingHookAction;

	//Use Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* UseAction;

	//Save Game Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SaveGameAction;

	//Load Game Input Action
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LoadGameAction;

public:
	// Sets default values for this character's properties
	AShooterCharacter();

	// public so UI can access it to subscribe to it
	FOnVelocityChangedSignature OnVelocityChangedDelegate;

	// Function signature
	void OnVelocityChanged(float NewVelocity);

	// Function signature
	UFUNCTION(BlueprintImplementableEvent)
	void OnSwitchGun();

protected:

	UPROPERTY(BlueprintAssignable)
	FInteractChanged NewInteractMsg;

	FTimerHandle TInteractTimerHandle;
	class AInteractableBase* Interactable;

	UPROPERTY(EditDefaultsOnly, Category = "ShooterGame Settings")
	float InteractionRange;

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(class AInteractableBase* InteractingObject);
	bool Server_Interact_Validate(class AInteractableBase* InteractingObject);
	void Server_Interact_Implementation(class AInteractableBase* InteractingObject);

	void SetInteractableObject();

	UFUNCTION()
	void OnRep_AttachGun();

	UFUNCTION()
	void OnRep_AttachGrapple();

public:

	void RefreshInteractableObject();

	// Called for shooting input
	void Shoot(const FInputActionValue& Value);

	void StopShooting(const FInputActionValue& Value);

	void Reload(const FInputActionValue& Value);

	void SwitchGun(const FInputActionValue& Value);

	void SwitchWeaponSlot1();
	void SwitchWeaponSlot2();
	void SwitchWeaponSlot3();
	void SwitchWeaponSlot4();
	void SwitchWeaponSlot5();
	void SwitchWeaponSlot6();

	void Melee(const FInputActionValue& Value);

	void Run(const FInputActionValue& Value);

	void StartSlide(const FInputActionValue& Value);

	void StopSlide(const FInputActionValue& Value);

	void StopRunning(const FInputActionValue& Value);

	void ToggleGrapple();

	void StartGrappling();

	void StopGrappling();

	UFUNCTION()
	void AddGrappleForce(float GrappleForce);
	
	//public so ABP can get ref and do stuff with isshooting, maybe not anymore
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttachGun)
	class AGunBase* CurrentGun;

	class AGunBase* PreviousGun;

	UFUNCTION(BlueprintCallable)
	AGunBase* GetGun();

	void GivePlayerNewGun(AGunBase* NewGun);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SwitchGun(class AGunBase* NewGun, int32 NewActiveGunIndex);
	bool Server_SwitchGun_Validate(class AGunBase* NewGun, int32 NewActiveGunIndex);
	void Server_SwitchGun_Implementation(class AGunBase* NewGun, int32 NewActiveGunIndex);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Grapple(class AGunBase* NewGun, int32 NewActiveGunIndex);
	bool Server_Grapple_Validate(class AGunBase* NewGun, int32 NewActiveGunIndex);
	void Server_Grapple_Implementation(class AGunBase* NewGun, int32 NewActiveGunIndex);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttachGrapple)
	class AGrapplingHook* GrapplingHook;

	UFUNCTION(BlueprintCallable)
	AGrapplingHook* GetGrapplingHook();

	// For TurnInPlace
	UPROPERTY(BlueprintReadOnly, Replicated)
	FVector2D TurnRate;

	// override Actor.h TakeDamage with our own
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	//bp pure is callable and more sort of const pure node doesnt have execution pin white
	// doesnt change anything global only outputs it produces

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

	UFUNCTION(BlueprintPure)
	bool IsRunning() const;

	UFUNCTION(BlueprintPure)
	bool IsSliding() const;

	UPROPERTY(Replicated)
	bool bIsRunning;

	UPROPERTY(Replicated)
	bool bIsSliding;

	UPROPERTY(Replicated)
	// for grappling and anims mostly
	bool bIsGrappling;

	UFUNCTION(BlueprintPure)
	bool IsGrappling() const;

	UFUNCTION(BlueprintCallable)
	void SetPlayingAction(bool NewIsPerformingAction);

	UFUNCTION(BlueprintCallable)
	bool IsPlayingAction() const { return bIsPlayingAction; }

	UFUNCTION(BlueprintPure)
	float GetHealthPercentage() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Die, EditAnywhere)
	bool bIsDead;

	UPROPERTY(Replicated)
	bool bIsRagdoll;

	UFUNCTION()
	void PlayerDie();

	UFUNCTION()
	void OnRep_Die();

public:

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns CurrentGun **/
	FORCEINLINE class AGunBase* GetGun() const { return CurrentGun; }

protected:

	// for checking if anim montage playing should block some action
	bool bIsPlayingAction;

	UPROPERTY(EditAnywhere)
	float RunSpeed = 500.f;

	UPROPERTY(EditAnywhere)
	float SlideSpeed = 800.f;

	UPROPERTY(EditAnywhere)
	float WalkSpeed = 300.0f;

	// use the default CurrentGun class useful for testing
	UPROPERTY(EditAnywhere)
	bool bUseDefaultGunClass = false;

	UPROPERTY(EditAnywhere)
	class TSubclassOf<AGunBase> DefaultGunClass = nullptr;

	UPROPERTY(EditAnywhere)
	class TSubclassOf<AGrapplingHook> GrapplingHookClass = nullptr;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AGunBase>> GunClasses = {};

	UPROPERTY(Replicated)
	TArray<AGunBase*> AllGunsEquipped = {};

	UPROPERTY(Replicated)
	int32 ActiveGunIndex = 0;

	UPROPERTY(EditDefaultsOnly)
	float MaxHealth = 100;

	UPROPERTY(VisibleAnywhere, Replicated)
	float Health = 100;

	void SubtractHealth(int32 Damage);

	 //							 //
	// Effects and AnimMontages //

	UPROPERTY()
	UAnimInstance* AnimationInstance;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_Shoot;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* SwitchWeaponAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* MeleeAnimMontage;

	UPROPERTY(EditAnywhere, Category = "Animations")
	class UAnimMontage* HitReactAnimMontage;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_HitReact;

	UPROPERTY(EditAnywhere, Category = "CameraShakes")
	TSubclassOf<UCameraShakeBase> CS_ShootSniper;


	 //					//
	// NOT UPROPERTIES //

	APlayerCameraManager* GetCameraManager();

	UAnimInstance* GetAnimInstCharacter();

	/// debugger class ///
#if UE_BUILD_DEVELOPMENT || WITH_EDITOR
	UDbg* Dbg;
#endif
};