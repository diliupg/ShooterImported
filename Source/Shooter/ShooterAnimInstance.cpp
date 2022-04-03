// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties( float DeltaTime )
{
	if ( ShooterCharacter == nullptr )
	{
		ShooterCharacter = Cast<AShooterCharacter>( TryGetPawnOwner( ) );
	}
	if ( ShooterCharacter )
	{
		// get the lateral speed of the character from the velocity
		FVector Velocity { ShooterCharacter->GetVelocity( ) };
		Velocity.Z = 0;
		Speed = Velocity.Size( );

		// is the character in the air?
		bIsInAir = ShooterCharacter->GetCharacterMovement( )->IsFalling( );

		// is the character accelerating?
		if ( ShooterCharacter->GetCharacterMovement( )->GetCurrentAcceleration( ).Size( ) > 0.f )
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation( );

		FRotator MovementRotation =
			UKismetMathLibrary::MakeRotFromX(
				ShooterCharacter->GetVelocity( ) );

		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(
			MovementRotation,
			AimRotation ).Yaw;
		if ( ShooterCharacter->GetVelocity( ).Size( ) > 0.f )
		{
			LastMovementOffsetYaw = MovementOffsetYaw;
		}
		
		/*
		FString RotationMessage = 
			FString::Printf(
				TEXT( "Base Aim Rotation: %f" ),
				AimRotation.Yaw );

		FString MovementRotationMessage =
			FString::Printf(
				TEXT( "Movement Rotation: %f" ),
				MovementRotation.Yaw );
		
		FString OffsetMessage =
			FString::Printf(
				TEXT( "Movement offset Yaw: %f" ),
				MovementOffsetYaw );

		if ( GEngine )
		{
			GEngine->AddOnScreenDebugMessage( 1, 0.f, FColor::White, OffsetMessage);
		}
		*/
	}
}

void UShooterAnimInstance::NativeInitializeAnimation( )
{
	ShooterCharacter = Cast<AShooterCharacter>( TryGetPawnOwner( ) );
}
