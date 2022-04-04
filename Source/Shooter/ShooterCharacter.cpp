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

// Sets default values
AShooterCharacter::AShooterCharacter( ) :
	BaseTurnRate( 45.f ),
	BaseLookUpRate( 45.f ),
	bAiming(false ),
	CameraDefaultFOV( 0.f ), // set in begin play
	CameraZoomedFov( 40.f ),
	cameraCurrentFOV( 0.f ),
	ZoomInterpSpeed( 50.f )
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// create a camera boom (pulls in towards the character is there is a collision
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>( TEXT( "CameraBoom" ) );
	CameraBoom->SetupAttachment( RootComponent );
	// the camera follows at this distance behind the controller
	CameraBoom->TargetArmLength = 250.f;
	// rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = FVector( 0.f, 80.f, 75.f );

	//create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>( TEXT( "FollowCamera" ) );
	// attach camera to end of boom
	FollowCamera->SetupAttachment( CameraBoom, USpringArmComponent::SocketName );
	// camera does not rotate relative to arm
	FollowCamera->bUsePawnControlRotation = false;

	// don't rotate when the controller rotates. Let the controller only affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true ;
	bUseControllerRotationRoll = false;

	// configure character movement
	GetCharacterMovement( )->bOrientRotationToMovement = false; // character moves in the direction of input..
	GetCharacterMovement( )->RotationRate = FRotator( 0.f, 540.f, 0.f );// ... at this rotation rate.
	GetCharacterMovement( )->JumpZVelocity = 600.f;
	GetCharacterMovement( )->AirControl = 0.2f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	if ( FollowCamera )
	{
		CameraDefaultFOV = GetFollowCamera( )->FieldOfView;
		cameraCurrentFOV = CameraDefaultFOV; 
	}
}

void AShooterCharacter::MoveForward( float Value )
{
	if ( ( Controller != nullptr ) && ( Value != 0.f ) )
	{
		// find out which way is forward
		const FRotator Rotation { Controller->GetControlRotation( ) };
		const FRotator YawRotation { 0.f, Rotation.Yaw, 0.f };

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis( EAxis::X ) };
		AddMovementInput( Direction, Value );
	}
}

void AShooterCharacter::MoveRight( float Value )
{
	if ( ( Controller != nullptr ) && ( Value != 0.f ) )
	{
		// find out which way is forward
		const FRotator Rotation { Controller->GetControlRotation( ) };
		const FRotator YawRotation { 0.f, Rotation.Yaw, 0.f };

		const FVector Direction { FRotationMatrix{YawRotation}.GetUnitAxis( EAxis::Y ) };
		AddMovementInput( Direction, Value );
	}
}

void AShooterCharacter::TurnAtRate( float Rate )
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput( Rate * BaseTurnRate * GetWorld( )->GetDeltaSeconds( ) ); // deg/sec * sec/frame = deg/frame
}

void AShooterCharacter::LookUpAtRate( float Rate )
{
	AddControllerPitchInput( Rate * BaseLookUpRate * GetWorld( )->GetDeltaSeconds( ) ); // deg/sec * sec/frame = deg/frame
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
}

bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzleSocketLocation,
	FVector& OutBeamLocation )
{
	// get current viewport size
	FVector2D ViewportSize;
	if ( GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->GetViewportSize( ViewportSize );
	}

	// get screen space location of cross-hairs
	FVector2D CrosshairLocation( ViewportSize.X / 2.f, ViewportSize.Y / 2.f );
	CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// get world position and direction of cross-hairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController( this, 0 ),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection );

	if ( bScreenToWorld ) // was deprojection successful?
	{
		FHitResult  ScreenTraceHit;
		const FVector Start { CrosshairWorldPosition };
		const FVector End { CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f };

		// set beam endpoint to line trace endpoint
		OutBeamLocation = End;

		// trace outward from crosshairs world location
		GetWorld( )->LineTraceSingleByChannel(
			ScreenTraceHit,
			Start,
			End,
			ECollisionChannel::ECC_Visibility );

		if ( ScreenTraceHit.bBlockingHit ) // was there a trace hit?
		{
			// beam endpoint is now trace hit location
			OutBeamLocation = ScreenTraceHit.Location;
		}

		// perform a second trace, this time from the gun barrel
		FHitResult WeaponTraceHit;
		const FVector WeaponTraceStart { MuzzleSocketLocation};
		const FVector WeaponTraceEnd { OutBeamLocation };
		GetWorld( )->LineTraceSingleByChannel(
			WeaponTraceHit,
			WeaponTraceStart,
			WeaponTraceEnd,
			ECollisionChannel::ECC_Visibility );
		if ( WeaponTraceHit.bBlockingHit ) // object between barrel and beam endpoint?
		{
			OutBeamLocation = WeaponTraceHit.Location;
		}
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
	// set camera current FOV
	if ( bAiming )
	{
		// Interpolate to zoomed FOV
		cameraCurrentFOV = FMath::FInterpTo(
			cameraCurrentFOV,
			CameraZoomedFov,
			DeltaTime,
			ZoomInterpSpeed );
	}
	else
	{
		// interpolate to default FOV
		cameraCurrentFOV = FMath::FInterpTo(
			cameraCurrentFOV,
			CameraDefaultFOV,
			DeltaTime,
			ZoomInterpSpeed );
	}
	GetFollowCamera( )->SetFieldOfView( cameraCurrentFOV );
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// handle interpolation for zoom when aiming
	CameraInterpZoom( DeltaTime );
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
	PlayerInputComponent->BindAxis( "Turn", this, &APawn::AddControllerYawInput );
	PlayerInputComponent->BindAxis( "LookUp", this, &APawn::AddControllerPitchInput );

	PlayerInputComponent->BindAction( "Jump", IE_Pressed, this, &ACharacter::Jump );
	PlayerInputComponent->BindAction( "Jump", IE_Released, this, &ACharacter::StopJumping );

	PlayerInputComponent->BindAction( "FireButton", IE_Pressed, this, &AShooterCharacter::FireWeapon );

	PlayerInputComponent->BindAction( "AimingButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed );
	PlayerInputComponent->BindAction( "AImingButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased );
}

