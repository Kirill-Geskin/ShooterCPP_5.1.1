// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerCharacterAnimInstance.generated.h"

/**
 * 
 */

class APlayerCharacter;

UCLASS()
class SHOOTERCPP_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
	/** UpdateAnimationProperties behave like tick function from actors */
	UFUNCTION(BlueprintCallable)
	void UpdateAnimationProperties(float DeltaTime);
	
	/** NativeInitializeAnimation works like begin play for actors, but for UAnimInstance */
	virtual void NativeInitializeAnimation() override;

private:

	/** This pointer to APlayerCharacter, but before initializing int`s a pointer to nowhere */ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	APlayerCharacter* PlayerCharacter;

	/** The speed of the player character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float PlayerCharacterSpeed;

	/** Whether or not the character is in the air */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/** Whether or not the character is moving */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/** Offset yaw used for strafing */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float MovementOffsetYaw;

	/** Offset yaw the frame before we stopped moving */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float LastMovementOffsetYaw;
};
