// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Crosshair_Widget.generated.h"

class USizeBox;
class AGunBase;
class AShooterCharacter;
class UCanvasPanelSlot;

UCLASS()
class SHOOTERGAME_API UCrosshair_Widget : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UCrosshair_Widget(const FObjectInitializer& ObjectInitializer);

protected:

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	USizeBox* SB_Top;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	USizeBox* SB_Bottom;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	USizeBox* SB_Left;

	UPROPERTY(BlueprintReadOnly, Meta = (BindWidget))
	USizeBox* SB_Right;

	// test velocity
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float TestVelocity;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float MaxVelocity;

	// maxoffset for reticle
	UPROPERTY(EditAnywhere, Category = "Crosshair")
	float MaxOffset;

	UPROPERTY(BlueprintReadOnly, Meta = (ExposeOnSpawn))
	AGunBase* Gun;
	
	UCanvasPanelSlot* CPS_Top;
	UCanvasPanelSlot* CPS_Bottom;
	UCanvasPanelSlot* CPS_Left;
	UCanvasPanelSlot* CPS_Right;


	virtual void SynchronizeProperties() override;

	virtual void NativeConstruct() override;

	void SetCanvasPanelSlots();

	UCanvasPanelSlot* GetCanvasPanelSlot(USizeBox* SizeBox);

public:

	UFUNCTION()
	void HandleCrosshairScale(float Velocity);

};
