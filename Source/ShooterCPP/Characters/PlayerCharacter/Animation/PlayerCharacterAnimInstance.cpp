// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ShooterCPP/Characters/PlayerCharacter/PlayerCharacter.h"

void UPlayerCharacterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (PlayerCharacter == nullptr)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	if(PlayerCharacter)
	{
		/** get speed lateral of the player character from velocity */
		FVector Velocity{ PlayerCharacter->GetVelocity() };
		Velocity.Z = 0;
		PlayerCharacterSpeed = Velocity.Size();

		/** is the player character in the air? */
		bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();

		/** is the player character accelerating? */
		if(PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			/** if player character moving around bIsAccelerating = true */
			bIsAccelerating = true;
		}
		else
		{
			/** if player character standing still around bIsAccelerating = false */
			bIsAccelerating = false;
		}

		/** GetBaseAimRotation() returns the rotator corresponding to the direction we`re aiming */
		const FRotator AimRotator = PlayerCharacter->GetBaseAimRotation();
		const FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());

		if(PlayerCharacter->GetVelocity().Size() > 0.f)
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotator).Yaw;
		
		const FString OffsetMessage = FString::Printf(TEXT("Movement Offset Yaw: %f "), MovementOffsetYaw);

		if(GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, OffsetMessage);
		}
	}
}

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	/**
	 * TryGetPawnOwner() is a function that gets the pawn that owns this anim instance,
	 * but it just returns the pawn, so we needs to cast it to a PlayerCharacter
	 */
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}
