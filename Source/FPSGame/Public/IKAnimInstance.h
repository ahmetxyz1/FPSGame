// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "IKAnimInstance.generated.h"

class AFPSGameCharacter;
UCLASS()
class FPSGAME_API UIKAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UIKAnimInstance();

	virtual void NativeBeginPlay() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	AFPSGameCharacter* Character;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	FTransform RelativeHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	FTransform SightTransform;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	FTransform LeftHandTransform;
	
	FTransform FinalHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = "something")
		float AimAlpha;

	UPROPERTY(BlueprintReadOnly, Category = "something")
		float ReloadAlpha;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "something")
	class UCurveVector* VectorCurve;

	UPROPERTY(BlueprintReadOnly, Category = "something")
		FVector SwayLocation;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	FTransform TurningSwayTransform;
	FRotator UnmodifiedTurnRotator;
	FRotator OldRotation;

	UPROPERTY(BlueprintReadOnly, Category = "something")
	FTransform RecoilTransform;
	FTransform FinalRecoilTransform;

	bool bInterpAiming;
	bool bIsAiming;
	bool bInterpRelativeHand;

protected:
	void SetSightTransform();
	void SetRelativeHandTransform();
	void SetFinalHandTransform();
	void SetLeftHandIK();

	void InterpAiming(float DeltaSeconds);
	void InterpRelativeHand(float DeltaSeconds);

	void MoveVectorCurve(float DeltaSeconds);
	void RotateWithRotation(float DeltaSeconds);

	void InterpFinalRecoil(float DeltaSeconds);
	void InterpRecoil(float DeltaSeconds);
public:
	void SetAiming(bool IsAiming);

	void CycledOptic();

	void Reload();
	UFUNCTION(BlueprintCallable, Category = "something")
	void StopReload();

	UFUNCTION(BlueprintCallable, Category = "something")
		void Fire();
};
