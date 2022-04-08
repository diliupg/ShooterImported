// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType )
enum class EItemRarity : uint8
{
	EIR_Damaged		UMETA(DisplayName = "Damaged" ),
	EIR_Common		UMETA( DisplayName = "Common" ),
	EIR_Uncommon	UMETA( DisplayName = "Uncommon" ),
	EIR_Rare		UMETA( DisplayName = "Rare" ),
	EIR_Legendary	UMETA( DisplayName = "Legendary" ),

	EIR_Max			UMETA( DisplayName = "DefaultMAX" )
};
UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* called when overlapping AreaSphere */
	UFUNCTION()
	void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherbodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	/* called when end overlapping AreaSphere */
	UFUNCTION()
	void ONSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponenet,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex );

	/* Sets the ActiveStars array of bools based on rarity */
	void SetActiveStars( );
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	/* skeletal mesh for the item */
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
		USkeletalMeshComponent* ItemMesh;

	/* line trace collides with box to show HUD widgets */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	class UBoxComponent* CollisionBox;

	/* pop up widget for when the player looks at the item */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) ) 
	class UWidgetComponent* PickupWidget;

	/* enables item tracing when overlapped*/
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	class USphereComponent* AreaSphere;

	/* the name which appears on the pickup widget */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	FString ItemName;

	/* ItemCount (ammo et.) */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	int32 ItemCount;

	/* Item rarity - determines number of stars in Pickup Widget */
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	EItemRarity ItemRarity;
 
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = ItemProperties, meta = ( AllowPrivateAccess = "true" ) )
	TArray<bool> ActiveStars;

public:
	FORCEINLINE UWidgetComponent* GetPickupWidget( ) const { return PickupWidget; }
};
