

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include  "Components/BoxComponent.h"
#include "AMissileProjectile.generated.h"

UCLASS()
class SHOOTERGAME_API AAMissileProjectile : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere,Category=Component)
	UProjectileMovementComponent* MissileMovementComponent;

	UPROPERTY(VisibleDefaultsOnly,Category=Component)
	UBoxComponent* BoxComponent;

	UPROPERTY(VisibleDefaultsOnly,Category=Component)
	UParticleSystemComponent* MissileParticleSystem;

	UPROPERTY(VisibleDefaultsOnly,Category=Component)
	UParticleSystemComponent* ExplodeParticleSystem;
	UPROPERTY(VisibleDefaultsOnly,Category=Component)
	UAudioComponent* MissileAudio;
	
	UPROPERTY(VisibleDefaultsOnly,Category=Component)
	UStaticMeshComponent* MissileStaticMeshComponent;
	
	
	AAMissileProjectile();

protected:
	
	virtual void BeginPlay() override;

public:

	UFUNCTION()
		virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void FireInDirection(FVector direction) const;
};
