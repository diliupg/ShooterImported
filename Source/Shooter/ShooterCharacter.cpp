// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter( ) :
	// Base rates for turning/looking up
	BaseTurnRate( 45.f ),
	BaseLookUpRate( 45.f ),
	// Turn rates for aiming/not aiming
	HipTurnRate( 90.f ),
	HipLookUpRate( 90.f ),
	AimingTurnRate( 20.f ),
	AimingLookUpRate( 20.f ),
	// Mouse look sensitivity scale factors 
	MouseHipTurnRate( 1.0f ),
	MouseHipLookUpRate( 1.0f ),
	MouseAimingTurnRate( 0.2f ),
	MouseAimingLookUpRate( 0.2f ),
	// true when aiming the weapon
	bAiming( false ),
	// Camera field of view values
	CameraDefaultFOV( 0.f ), // set in BeginPlay
	CameraZoomedFOV( 35.f ),
	CameraCurrentFOV( 0.f ),
	ZoomInterpSpeed( 20.f ),
	// crosshair spread factors
	CrosshairSpreadMultiplier( 0.f ),
	CrosshairVelocityFactor( 0.f ),
	CrosshairInAirFactor( 0.f ),
	CrosshairAimFactor( 0.f ),
	CrosshairShootingFactor( 0.f ),
	// automatic fire variables
	AutomaticFireRate( 0.1f ),
	bShouldFire( true ),
	bFireButtonPressed( false ),
	// Item trace variables
	bShouldTraceForItems( false ),
	// bullet fire timer variables
	ShootTimeDuration( 0.05f ),
	bFiringBullet( false )

{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create a camera boom (pulls in towards the character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>( TEXT( "CameraBoom" ) );
	CameraBoom->SetupAttachment( RootComponent );
	CameraBoom->TargetArmLength = 180.f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector( 0.f, 50.f, 70.f );

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>( TEXT( "FollowCamera" ) );
	FollowCamera->SetupAttachment( CameraBoom, USpringArmComponent::SocketName ); // Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement( )->bOrientRotationToMovement = false; // Character moves in the direction of input...
	GetCharacterMovement( )->RotationRate = FRotator( 0.f, 540.f, 0.f ); // ... at this rotation rate
	GetCharacterMovement( )->JumpZVelocity = 600.f;
	GetCharacterMovement( )->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay( )
{
	Super::BeginPlay( );

	if ( FollowCamera )
	{
		CameraDefaultFOV = GetFollowCamera( )->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}
	// spawn the default weapon and equip it
	EquipWeapon( SpawnDefaultWeapon() );
}

void AShooterCharacter::MoveForward( float Value )
{
	if ( ( Controller != nullptr ) && ( Value != 0.0f ) )
	{
		// find out which way is forward
		const FRotator Rotation { Controller->GetControlRotation( ) };
		const FRotator YawRotation { 0, Rotation.Yaw, 0 };

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis( EAxis::X ) };
		AddMovementInput( Direction, Value );
	}
}

void AShooterCharacter::MoveRight( float Value )
{
	if ( ( Controller != nullptr ) && ( Value != 0.0f ) )
	{
		// find out which way is right
		const FRotator Rotation { Controller->GetControlRotation( ) };
		const FRotator YawRotation { 0, Rotation.Yaw, 0 };

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis( EAxis::Y ) };
		AddMovementInput( Direction, Value );
	}
}

void AShooterCharacter::TurnAtRate( float Rate )
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput( Rate * BaseTurnRate * GetWorld( )->GetDeltaSeconds( ) ); // deg/sec * sec/frame
}

void AShooterCharacter::LookUpAtRate( float Rate )
{
	AddControllerPitchInput( Rate * BaseLookUpRate * GetWorld( )->GetDeltaSeconds( ) ); // deg/sec * sec/frame
}

void AShooterCharacter::Turn( float Value )
{
	float TurnScaleFactor {};
	if ( bAiming )
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput( Value * TurnScaleFactor );
}

void AShooterCharacter::LookUp( float Value )
{
	float LookUpScaleFactor {};
	if ( bAiming )
	{
		LookUpScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		LookUpScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput( Value * LookUpScaleFactor );
}

void AShooterCharacter::FireWeapon( )
{
	if ( FireSound )
	{
		UGameplayStatics::PlaySound2D( this, FireSound );
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh( )->GetSocketByName( "BarrelSocket" );
	if ( BarrelSocket )
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform( GetMesh( ) );

		if ( MuzzleFlash )
		{
			UGameplayStatics::SpawnEmitterAtLocation( GetWorld( ), MuzzleFlash, SocketTransform );
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(
			SocketTransform.GetLocation( ), BeamEnd );
		if ( bBeamEnd )
		{
			if ( ImpactParticles )
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld( ),
					ImpactParticles,
					BeamEnd );
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld( ),
				BeamParticles,
				SocketTransform );
			if ( Beam )
			{
				Beam->SetVectorParameter( FName( "Target" ), BeamEnd );
			}
		}
	}
	UAnimInstance* AnimInstance = GetMesh( )->GetAnimInstance( );
	if ( AnimInstance && HipFireMontage )
	{
		AnimInstance->Montage_Play( HipFireMontage );
		AnimInstance->Montage_JumpToSection( FName( "StartFire" ) );
	}

	// start bullet fire timer for crosshairs
	StartCrosshairBulletFire( );
}

bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamLocation )
{
	// check for crosshair trace hit
	FHitResult CrossHairHitResult;
	bool bCrosshairHit = TraceUnderCrossHars( CrossHairHitResult, OutBeamLocation );

	if ( bCrosshairHit )
	{
		// tentative beam l;ocation - still need to trace from gun
		OutBeamLocation = CrossHairHitResult.Location;
	}
	else // no crosshair trace hit
	{
		// out beam location is the end location for the line trace
	}

	// Perform a second trace, this time from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart { MuzzleSocketLocation };
	const FVector StartToEnd { OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd { MuzzleSocketLocation + StartToEnd * 1.25f };

	GetWorld( )->LineTraceSingleByChannel(
		WeaponTraceHit,
		WeaponTraceStart,
		WeaponTraceEnd,
		ECollisionChannel::ECC_Visibility );
	if ( WeaponTraceHit.bBlockingHit ) // object between barrel and BeamEndPoint?
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false; 
}

void AShooterCharacter::AimingButtonPressed( )
{
	bAiming = true;
}

void AShooterCharacter::AimingButtonReleased( )
{
	bAiming = false;
}

void AShooterCharacter::CameraInterpZoom( float DeltaTime )
{
	// Set current camera field of view
	if ( bAiming )
	{
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraZoomedFOV,
			DeltaTime,
			ZoomInterpSpeed );
	}
	else
	{
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraDefaultFOV,
			DeltaTime,
			ZoomInterpSpeed );
	}
	GetFollowCamera( )->SetFieldOfView( CameraCurrentFOV );
}

void AShooterCharacter::SetLookRates( )
{
	if ( bAiming )
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = HipLookUpRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread( float DeltaTime )
{
	FVector2D WalkSpeedRange { 0.f, 600.f };
	FVector2D VelocityMultiplierRange { 0.f, 1.f };
	FVector Velocity { GetVelocity( ) };
	Velocity.Z = 0.f;

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
	
		Velocity.Size( ) );

	// calculate crosshair in air factor
	if ( GetCharacterMovement( )->IsFalling( ) ) // is in air?
	{
		// spread the crosshairs slowly while in air
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			2.25f,
			DeltaTime,
			2.25f );
	}
	else // character is on the ground
	{
		// shrink the crosshairs rapidly when on the ground
		CrosshairInAirFactor = FMath::FInterpTo(
			CrosshairInAirFactor,
			0.f,
			DeltaTime,
			30.f );
	}

	// Calculate crosshair aim factor
	if ( bAiming ) // are we aiming?
	{
		// Shrink crosshairs a small amount very quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.6f,
			DeltaTime,
			30.f );
	}
	else // not aiming
	{
		// spread crosshairs back to normal very quickly
		CrosshairAimFactor = FMath::FInterpTo(
			CrosshairAimFactor,
			0.f,
			DeltaTime,
			30.f );
	}

	// true 0.05 seconds after firing
	if ( bFiringBullet )
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor, 
			0.3f, 
			DeltaTime, 
			60.f );
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(
			CrosshairShootingFactor,
			0.f,
			DeltaTime,
			60.f );
	}

	CrosshairSpreadMultiplier = 
		0.5f + 
		CrosshairVelocityFactor + 
		CrosshairInAirFactor -
		CrosshairAimFactor +
		CrosshairShootingFactor;
}

void AShooterCharacter::StartCrosshairBulletFire( )
{
	bFiringBullet = true;

	GetWorldTimerManager( ).SetTimer(
		CrosshairShootTimer,
		this,
		&AShooterCharacter::FinishCrosshairBulletFire,
		ShootTimeDuration );
}

void AShooterCharacter::FinishCrosshairBulletFire( )
{
	bFiringBullet = false;
}

void AShooterCharacter::FireButtonPressed( )
{
	bFireButtonPressed = true;
	StartFireTimer( );
}

void AShooterCharacter::FireButtonReleased( )
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer( )
{
	if ( bShouldFire )
	{
		FireWeapon( );
		bShouldFire = false;
		GetWorldTimerManager( ).SetTimer(
			AutoFireTimer,
			this,
			&AShooterCharacter::AutoFireReset,
			AutomaticFireRate );
	}
}

void AShooterCharacter::AutoFireReset( )
{
	bShouldFire = true;
	if ( bFireButtonPressed )
	{
		StartFireTimer( );
	}
}

bool AShooterCharacter::TraceUnderCrossHars( FHitResult& OutHitResult, FVector& OutHitLocation    )
{
	// get viewport size
	FVector2D ViewportSize;
	if ( GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->GetViewportSize( ViewportSize );
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation( ViewportSize.X / 2.f, ViewportSize.Y / 2.f );
	//CrosshairLocation.Y -= 50.f; // don't need this line as crosshairs now in screen center and NOT 50 up
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position and direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController( this, 0 ),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection );

	if ( bScreenToWorld )
	{
		// trace from crosshair world location outward
		const FVector Start { CrosshairWorldPosition };
		const FVector End { Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;

		GetWorld( )->LineTraceSingleByChannel(
			OutHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility );
		if ( OutHitResult.bBlockingHit )
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems( )
{
	if ( bShouldTraceForItems )
	{
		FHitResult ItemTraceResult;
		FVector HiiLocation;
		TraceUnderCrossHars( ItemTraceResult, HiiLocation );
		if ( ItemTraceResult.bBlockingHit )
		{
			AItem* HitItem = Cast<AItem>( ItemTraceResult.Actor );
			if ( HitItem && HitItem->GetPickupWidget( ) )
			{
				//show items pickup widget
				HitItem->GetPickupWidget( )->SetVisibility( true );
			}
			// we hit an AItem last frame
			if ( TraceHitItemLastFrame )
			{
				if ( HitItem != TraceHitItemLastFrame )
				{
					// we are hitting a different AItem this frame than the last frame or AItem is null
					TraceHitItemLastFrame->GetPickupWidget()->SetVisibility( false );
				}
			}
			// store a reference to HitItem for next frame
			TraceHitItemLastFrame = HitItem;
		}
	}

	else if ( TraceHitItemLastFrame )
	{
		// no longer overlapping any item, so item last frame should not show any widget
		TraceHitItemLastFrame->GetPickupWidget( )->SetVisibility( false );
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon( )
{
	// check the TSubClassOf Variable
	if ( DefaultWeaponClass )
	{
		// Spawn the Weapon
		return GetWorld( )->SpawnActor<AWeapon>( DefaultWeaponClass );
	}

	return nullptr;
}

void AShooterCharacter::EquipWeapon( AWeapon* WeaponToEquip )
{
	if ( WeaponToEquip )
	{
		// set area sphere to ignore all collision channels
		WeaponToEquip->GetAreaSphere( )->SetCollisionResponseToAllChannels(
			ECollisionResponse::ECR_Ignore );
		// set CollisionBox to ignore all collision channels
		WeaponToEquip->GetCollisionBox( )->SetCollisionResponseToAllChannels(
			ECollisionResponse::ECR_Ignore );

		// Get the Hand Socket
		const USkeletalMeshSocket* HandSocket = GetMesh( )->GetSocketByName(
			FName( "RightHandSocket" ) );
		if ( HandSocket )
		{
			// attach the weapon to the hand socket  RightHandSocket
			HandSocket->AttachActor( WeaponToEquip, GetMesh( ) );
		}
		// set equipped weapon to the newly spawned weapon
			EquippedWeapon = WeaponToEquip;

	}
}

// Called every frame
void AShooterCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Handle interpolation for zoom when aiming
	CameraInterpZoom( DeltaTime );
	// Change look sensitivity based on aiming
	SetLookRates( );
	// Calculate crosshair spread multiplier
	CalculateCrosshairSpread( DeltaTime );
	// Check OverlappedItemCount, then trace for items
	TraceForItems( );
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	Super::SetupPlayerInputComponent( PlayerInputComponent );
	check( PlayerInputComponent );

	PlayerInputComponent->BindAxis( "MoveForward", this, &AShooterCharacter::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &AShooterCharacter::MoveRight );
	PlayerInputComponent->BindAxis( "TurnRate", this, &AShooterCharacter::TurnAtRate );
	PlayerInputComponent->BindAxis( "LookUpRate", this, &AShooterCharacter::LookUpAtRate );
	PlayerInputComponent->BindAxis( "Turn", this, &AShooterCharacter::Turn );
	PlayerInputComponent->BindAxis( "LookUp", this, &AShooterCharacter::LookUp );

	PlayerInputComponent->BindAction( "Jump", IE_Pressed, this, &ACharacter::Jump );
	PlayerInputComponent->BindAction( "Jump", IE_Released, this, &ACharacter::StopJumping );

	PlayerInputComponent->BindAction( "FireButton", IE_Pressed, this,
		&AShooterCharacter::FireButtonPressed );
	PlayerInputComponent->BindAction( "FireButton", IE_Released, this,
		&AShooterCharacter::FireButtonReleased );

	PlayerInputComponent->BindAction( "AimingButton", IE_Pressed, this,
		&AShooterCharacter::AimingButtonPressed );
	PlayerInputComponent->BindAction( "AimingButton", IE_Released, this,
		&AShooterCharacter::AimingButtonReleased );
}

float AShooterCharacter::GetCrosshairSpreadMultiplier( ) const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlappedItemCount( int8 Amount )
{
	if ( OverlappedItemCount + Amount <= 0 )
	{
		OverlappedItemCount = 0;
		bShouldTraceForItems = false;
	}
	else
	{
		OverlappedItemCount += Amount;
		bShouldTraceForItems = true;
	}
}

