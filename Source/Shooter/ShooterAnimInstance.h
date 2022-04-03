// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable )
	void UpdateAnimationProperties( float DeltaTime );

	virtual void NativeInitializeAnimation( ) override;

private:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true" ) )
	class AShooterCharacter* ShooterCharacter;

	/* the speed of the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = (AllowPrivateAccess = "true" ) )
	float Speed;

	/* whether or not the character is in the air */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = ( AllowPrivateAccess = "true" ) )
	bool bIsInAir;

	/* whether or not the character is moving */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Movement, Meta = ( AllowPrivateAccess = "true" ) )
	bool bIsAccelerating;
	
	/* offset yaw used for strafing*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, Meta = ( AllowPrivateAccess = "true" ) )
	float MovementOffsetYaw;
};
