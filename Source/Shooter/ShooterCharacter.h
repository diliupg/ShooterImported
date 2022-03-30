// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* called for forward backward input */
	void MoveForward( float Value );

	/* called for side to side input*/
	void MoveRight( float Value );

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	/** class is forward declared
	 CameraBoom positioning the camera behind the character */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true" ) )
	class USpringArmComponent* CameraBoom;

	/* camera that follows the character*/
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	class UCameraComponent* FollowCamera;

public:
	/** returns CameraBoom subObject (child object) */
	FORCEINLINE USpringArmComponent* GetCameraBoom( ) const { return CameraBoom; }

	/* returns FollowCamera subObject*/
	FORCEINLINE UCameraComponent* GetFollowCamera( ) const { return FollowCamera; }
};
