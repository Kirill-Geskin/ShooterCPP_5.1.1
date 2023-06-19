// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	/** Base rates for turning/looking up */
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	/** Create CameraSpringArmComponent. Pulls towards the character if there is a collision*/
	CameraSpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArmComponent"));
	CameraSpringArmComponent->SetupAttachment(GetRootComponent());
	CameraSpringArmComponent->TargetArmLength = 300.f;
	CameraSpringArmComponent->bUsePawnControlRotation = true;
	CameraSpringArmComponent->SocketOffset = FVector(0.f,50.f,50.f);

	/** Create follow camera*/
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(CameraSpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false; // camera does not rotate to arm component

	/** Don`t rotate Player character when the controller rotates. Let the controller only affect the camera */
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	/** Rotate Player Character to the direction of movement */
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &APlayerCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Jump",IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump",IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("FireButton",IE_Pressed, this, &APlayerCharacter::FireWeapon);
}

void APlayerCharacter::MoveForward(float Value)
{
	if(Controller && Value)
	{
		const FRotator Rotation {Controller->GetControlRotation()};
		const FRotator YawRotation {0, Rotation.Yaw, 0};

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::MoveRight(float Value)
{
	if (Controller && Value)
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void APlayerCharacter::TurnAtRate(float Rate)
{
	/** Calculate delta for this frame from the Rate information. deg/sec * sec/frame */
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); 
}

void APlayerCharacter::LookUpAtRate(float Rate)
{
	/** Calculate delta for this frame from the Rate information. deg/sec * sec/frame */
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds()); 
}

void APlayerCharacter::FireWeapon()
{
	/** 1. We are checking if GunFireSound is setup in UE and play it */
	if(GunFireSound)
	{
		UGameplayStatics::PlaySound2D(this, GunFireSound);
	}

	/**
	 * 2. We are checking if BarrelSocket is setup in UE by name "BarrelSocket"
	 * and store socket location in local veriable
	 */
	if(const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket"))
	{
		/** 3. Save transform of BarrelSocket if variable*/
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());

		/** 4. We are checking if MuzzleFlashParticle is setup in UE and spawn it */
		if(MuzzleFlashParticle)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlashParticle, SocketTransform);
		}

		/** 5. We call GetBeamEndLocation() function if it return true we spawn two particle effects*/ 
		FVector BeamEnd;
		if(GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd))
		{
			if(ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),ImpactParticles, BeamEnd);
			}

			if(BeamParticles)
			{
				if(UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
					BeamParticles, SocketTransform))
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
	/** 6. Plying animation montage*/
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

bool APlayerCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	/** Get current size if the viewport */
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	/** Get screen space location of crosshairs */
	FVector2d CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	/**
	* Function DeprojectScreenToWorld return bool which you check in if() statement below.
	* Also DeprojectScreenToWorld() revrite CrosshairWorldPosition and CrosshairWorldDirection.
	*/
	if(UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this,
	0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection))
	{
		FHitResult ScreenTraceHit;
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ CrosshairWorldPosition + CrosshairWorldDirection * 50000.f };

		/** Set BeamEnPoint to line trace point. In case we didn`t hit anything, but we need smoke beam */
		OutBeamLocation = End;
		/** Trace from crosshair to location in world */ 
		GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End, ECC_Visibility);

		if(ScreenTraceHit.bBlockingHit)
		{
			/** If we hit somethigs, we are revrite BeamEndPoint by ScreenTraceHit.Location */
			OutBeamLocation = ScreenTraceHit.Location;
		}

		FHitResult WeaponTraceHit;
		const FVector WeaponTStartTrace{ MuzzleSocketLocation };
		const FVector WeaponEndTrace{ OutBeamLocation };

		GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTStartTrace, WeaponEndTrace,
			ECC_Visibility);

		/** WeaponTraceHit shows if we hit the object`s between barrel and end point */
		if(WeaponTraceHit.bBlockingHit)
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
		return true;
	}
	return false;
}
