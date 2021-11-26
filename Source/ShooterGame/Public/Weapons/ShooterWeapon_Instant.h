// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterWeapon.h"
#include "ShooterWeapon_Instant.generated.h"

class AShooterImpactEffect;

USTRUCT()
struct FInstantHitInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Origin;

	UPROPERTY()
	float ReticleSpread;

	UPROPERTY()
	int32 RandomSeed;
};

USTRUCT()
struct FInstantWeaponData
{
	GENERATED_USTRUCT_BODY()

	/** base weapon spread (degrees) */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float WeaponSpread;

	/** targeting spread modifier */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float TargetingSpreadMod;

	/** continuous firing: spread increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadIncrement;

	/** continuous firing: max increment */
	UPROPERTY(EditDefaultsOnly, Category=Accuracy)
	float FiringSpreadMax;

	/** weapon range */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	float WeaponRange;

	
	/** damage amount */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	int32 HitDamage;
	
	/** type of damage */
	UPROPERTY(EditDefaultsOnly, Category=WeaponStat)
	TSubclassOf<UDamageType> DamageType;

	/** hit verification: scale for bounding box of hit actor */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float ClientSideHitLeeway;

	/** hit verification: threshold for dot product between view direction and hit direction */
	UPROPERTY(EditDefaultsOnly, Category=HitVerification)
	float AllowedViewDotHitDir;

	/** defaults */
	FInstantWeaponData()
	{
		WeaponSpread = 5.0f;
		TargetingSpreadMod = 0.25f;
		FiringSpreadIncrement = 1.0f;
		FiringSpreadMax = 10.0f;
		WeaponRange = 10000.0f;
		HitDamage = 10;
		DamageType = UDamageType::StaticClass();
		ClientSideHitLeeway = 200.0f;
		AllowedViewDotHitDir = 0.8f;
	}
};

// A weapon where the damage impact occurs instantly upon firing
UCLASS(Abstract)
class AShooterWeapon_Instant : public AShooterWeapon
{
	GENERATED_UCLASS_BODY()

	/** get current spread */
	float GetCurrentSpread() const;
	
protected:

	virtual EAmmoType GetAmmoType() const override
	{
		return EAmmoType::EBullet;
	}

	/** weapon config */
	UPROPERTY(EditAnywhere, Category=Config)
	FInstantWeaponData InstantConfig;

	/** impact effects */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	TSubclassOf<AShooterImpactEffect> ImpactTemplate;

	/** smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	UParticleSystem* TrailFX;

	/** param name for beam target in smoke trail */
	UPROPERTY(EditDefaultsOnly, Category=Effects)
	FName TrailTargetParam;

	/** instant hit notify for replication */
	UPROPERTY(Transient, ReplicatedUsing=OnRep_HitNotify)
	FInstantHitInfo HitNotify;

	/** current spread from continuous firing */
	float CurrentFiringSpread;

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** server notified of hit from client to verify */
	UFUNCTION(reliable, server, WithValidation)
	void ServerNotifyHit(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** server notified of miss to show trail FX */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerNotifyMiss(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread);

	/** process the instant hit and notify the server if necessary
	 *
	 * 处理即时命中，并在必要时通知服务器
	 */
	void ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** continue processing the instant hit, as if it has been confirmed by the server
	 *
	 * 继续处理hit，好像它已经被服务器确认
	 */
	void ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread);

	/** 对垃圾桶做出子弹碰撞反应 */
	UFUNCTION(Reliable,NetMulticast)
	void ProcessTrash(const FHitResult& Impact, const FVector& ShootDir);
	
	/** check if weapon should deal damage to actor */
	bool ShouldDealDamage(AActor* TestActor) const;

	/** handle damage */
	void DealDamage(const FHitResult& Impact, const FVector& ShootDir);
	
	/** 经穿过墙体递减后的伤害值*/
	UPROPERTY(Replicated)
	int32 AfterDecreaseDamage;
	
	/** 穿墙是否进行伤害递减*/
	UPROPERTY(Replicated)
	bool bDecreaseDamage;
	
	/** 伤害递减
	 * @return 递减后的值
	 */
	void DamageDecrease(float CrossLength);

	/** [local] weapon specific fire implementation */
	virtual void FireWeapon() override;

	/** [local + server] update spread on firing */
	virtual void OnBurstFinished() override;

	/*在墙后面是否有一个Pawn，没有其他的物体，有的话返回其他物体，没有的话返回原物体*/
	FHitResult HavePawnBackWall(const FHitResult& Impact, const FVector& Start, const FVector& End, const FVector& ShootDir);


	//////////////////////////////////////////////////////////////////////////
	// Effects replication
	
	UFUNCTION()
	void OnRep_HitNotify();

	/** called in network play to do the cosmetic fx  */
	void SimulateInstantHit(const FVector& Origin, int32 RandomSeed, float ReticleSpread);

	/** spawn effects for impact */
	void SpawnImpactEffects(const FHitResult& Impact);

	/** spawn trail effect
	 *
	 * 生成枪口轨迹
	 */
	void SpawnTrailEffect(const FVector& EndPoint);
};
