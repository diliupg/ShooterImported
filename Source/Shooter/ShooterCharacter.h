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

	/*
	called via input to turn at given rate.
	@param Rate  This is a normalized rate i.e. 1.0 means 100% of desired turn rate
	*/
	void TurnAtRate( float Rate );

	/*
	called via input to look up/down at given rate.
	@param Rate  This is a normalized rate i.e. 1.0 means 100% of desired rate
	*/
	void LookUpAtRate( float Rate );

	/* rotate controller based on mouse X movement
	@param Value - the input value from mouse movement
	*/
	void Turn( float value );

	/* rotate controller based on mouse Y movement
	@param Value - the input value from mouse movement
	*/
	void LookUp( float vallue );

	/* called when the fire button is pressed*/
	void FireWeapon( );

	bool GetBeamEndLocation( const FVector& MuzzleSocketLocation, FVector& OutBeamLocation );

	/* set the aiming to true or false with button press */
	void AimingButtonPressed();
	void AimingButtonReleased( );

	void CameraInterpZoom(float DeltaTime);

	/* Set BaseTrunRate and BaseLookUpRate based on aiming */
	void SetLookRates( );


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

	/* Base turn rate in deg/sec.  Other scaling may affect final turn rate */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float BaseTurnRate;

	/* Base look up/down rate in deg/sec.  Other scaling may affect final turn rate */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float BaseLookUpRate;

	/* turn rate while not aiming */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float HipTurnRate;

	/* look up rate when not aiming */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float HipLookUpRate;

	/* turn rate when aiming */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float AimingTurnRate;

	/* look up rate when aiming */
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ) )
	float AimingLookUpRate;

	/* scale factor for mouse look sensitivity. Turn rate when not aiming*/
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
	float MouseHipTurnRate;

	/* scale factor for mouse look sensitivity. Look Up Rate when not aiming*/
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ), meta = ( ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
	float MouseHipLookUpRate;

	/* scale factor for mouse look sensitivity. Turn rate when aiming*/
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ), meta = ( ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
	float MouseAimingTurnRate;

	/* scale factor for mouse look sensitivity. Look Up Rate when aiming*/
	UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = ( AllowPrivateAccess = "true" ), meta = ( ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0" ) )
	float MouseAimingLookUpRate;

	/* randomized gunShot sound cue */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) );
	class USoundCue* FireSound;

	/* Flash spawned at BarrelSocket*/
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) );
	class UParticleSystem* MuzzleFlash;

	/* montage for firing the weapon */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) );
	class UAnimMontage* HipFireMontage;

	/* particles spawned upon bullet impact */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) );
	UParticleSystem* ImpactParticles;

	/* smoke trails for bullets */
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) );
	UParticleSystem* BeamParticles;

	/* true when aiming */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) )
	bool bAiming;

	/* Default camera field of view value */
	float CameraDefaultFOV;

	/* fieSld of view value when zoomed in */
	float CameraZoomedFov;

	/* current FOV this frame*/
	float cameraCurrentFOV;

	/* interp speed for zooming when aiming*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, Meta = ( AllowPrivateAccess = "true" ) )
	float ZoomInterpSpeed;

public:
	/** returns CameraBoom subObject (child object) */
	FORCEINLINE USpringArmComponent* GetCameraBoom( ) const { return CameraBoom; }

	/* returns FollowCamera subObject*/
	FORCEINLINE UCameraComponent* GetFollowCamera( ) const { return FollowCamera; }

	FORCEINLINE bool GetAiming( ) const { return bAiming; }
};
