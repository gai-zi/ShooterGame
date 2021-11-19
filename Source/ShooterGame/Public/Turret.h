// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Turret.generated.h"

UCLASS()
class SHOOTERGAME_API ATurret : public APawn
{
	GENERATED_BODY()
private:
	float MinPitch = -10.0f;
	float MaxPitch = 90.0f;
	float MinYaw = -90.0f;
	float MaxYaw = 90.0f;
	float RotationSpeed = 1.0f;
	
public:
	ATurret();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere,Category=Component)
	UBoxComponent* BoxComponent;
	//最下层底座
	UPROPERTY(EditAnywhere,Category=Component)
	UStaticMeshComponent* Base;
	//Yaw可转底座
	UPROPERTY(EditAnywhere,Category=Component)
	UStaticMeshComponent* Base2;
	
	UPROPERTY(EditAnywhere,Category=Component)
	UStaticMeshComponent* Launcher;

	UPROPERTY(EditAnywhere,Category=Component)
	UCameraComponent* OurCamera;

	//要生成的发射物类
	UPROPERTY(EditDefaultsOnly,Category=Projectile)
	TSubclassOf<class AAMissileProjectile> MissileClass;
	
public:	
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void TurnRight(float Value);
	UFUNCTION()
	void TurnUp(float Value);

	UFUNCTION()
	void Fire();
	//Get方法
	FORCEINLINE float GetMinPitch() const {return MinPitch;};
	FORCEINLINE float GetMaxPitch() const {return MaxPitch;};
	FORCEINLINE float GetMinYaw() const {return MinYaw;};
	FORCEINLINE float GetMaxYaw() const {return MinYaw;};
	FORCEINLINE float GetRotationSpeed() const {return RotationSpeed;};
};
