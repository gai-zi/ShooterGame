// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/ShooterWeapon_Instant.h"

#include <Actor.h>
#include <string>

#include "Particles/ParticleSystemComponent.h"
#include "Effects/ShooterImpactEffect.h"

AShooterWeapon_Instant::AShooterWeapon_Instant(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CurrentFiringSpread = 0.0f;

	bDecreaseDamage = false;
	AfterDecreaseDamage = 0.0f;
}
//////////////////////////////////////////////////////////////////////////
// Weapon usage

void AShooterWeapon_Instant::FireWeapon()
{
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const float CurrentSpread = GetCurrentSpread();
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);
	
	const FVector AimDir = GetAdjustedAim();
	const FVector StartTrace = GetCameraDamageStartLocation(AimDir);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;
	/*
	 * @Impact 子弹射线查询结果
	 */
	const FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	/*查询墙后是否有pawn*/
	FHitResult FinalImpact = HavePawnBackWall(Impact,StartTrace,EndTrace,ShootDir);

	//墙面依旧留下弹痕
	if(Impact.IsValidBlockingHit() && FinalImpact.IsValidBlockingHit())
		if(Impact.GetActor()->GetName().Compare(FinalImpact.GetActor()->GetName()))
		{
			// play FX locally
			if (GetNetMode() != NM_DedicatedServer)
			{
				SpawnImpactEffects(Impact);
			}
		}
	ProcessInstantHit(FinalImpact, StartTrace, ShootDir, RandomSeed, CurrentSpread);
	CurrentFiringSpread = FMath::Min(InstantConfig.FiringSpreadMax, CurrentFiringSpread + InstantConfig.FiringSpreadIncrement);
}

FHitResult AShooterWeapon_Instant::HavePawnBackWall(const FHitResult& Impact, const FVector& Start, const FVector& End, const FVector& ShootDir)
{
	TArray<FHitResult> Hits;
	//Trace遍历子弹路径中穿过物体
	UKismetSystemLibrary::LineTraceMulti(
	   GetWorld(),
	   Start,
	   End,
	   UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel4),
	   true,
	   ActorsToIgnore,
	   EDrawDebugTrace::None,
	   Hits,
	   true,
	   FLinearColor::Blue,
	   FLinearColor::Red,
	   20.0f
   );
	
	/*
	for(int i=0;i<Hits.Num();i++)
		UE_LOG(LogTemp,Warning,TEXT("%s"),*Hits[i].GetActor()->GetName());
	UE_LOG(LogTemp,Warning,TEXT("---------------------"));
	*/
	
	int SelfNum = 0;
	//检测到几个自己，确定SelfNum值
	for(int i=0;i<Hits.Num();i++)
	{
		//前序碰到的都是自己
		if(!Hits[i].GetActor()->GetName().Compare(this->GetPawnOwner()->GetName()))
		{
			SelfNum++;
		}
		else
			break;
	}
	//检测是否直接击中墙体，若无直接返回原Hit结果
	if(Hits.Num() >= SelfNum-1)
		if(!Hits[SelfNum].GetActor()->Tags.Contains(TEXT("Wall")))
		{
			//UE_LOG(LogTemp,Warning,TEXT("Can't Get WALL"));
			return Impact;
		}
	if(Hits.Num() >= SelfNum)
		if(Hits[SelfNum + 1].GetActor()->Tags.Contains(TEXT("Player")))
		{
			//UE_LOG(LogTemp,Warning,TEXT("Get Actor!!!"));
			FHitResult Hit; 
			//从击中敌人Pawn的位置向出发点发射Trace，	HitTrace即可
			GetWorld()->LineTraceSingleByChannel(Hit, Hits[SelfNum+1].Location, Start, COLLISION_WEAPON);
			//做差，从而得出子弹穿过墙体的长度
			DamageDecrease(FVector::Distance(Hits[SelfNum].Location,Hit.Location));
					
			/*DrawDebugPoint(GetWorld(),Hits[1].Location,50.0f,FColor::Orange,false,10.0f);
			DrawDebugPoint(GetWorld(),Hit.L ocation,50.0f,FColor::Orange,false,10.0f);*/
			bDecreaseDamage = true;
			return Hits[SelfNum + 1];
		}
	if(AfterDecreaseDamage <= 0.0f)
	{
		bDecreaseDamage = false;
		return Impact;
	}
	return Impact;
}

bool AShooterWeapon_Instant::ServerNotifyHit_Validate(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AShooterWeapon_Instant::ServerNotifyHit_Implementation(const FHitResult& Impact, FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const float WeaponAngleDot = FMath::Abs(FMath::Sin(ReticleSpread * PI / 180.f));
	//Instigator伤害发起者
	// if we have an instigator, calculate dot between the view and the shot
	//如果有伤害发起者，计算
	if (GetInstigator() && (Impact.GetActor() || Impact.bBlockingHit))
	{
		const FVector Origin = GetMuzzleLocation();		//枪口位置
		const FVector ViewDir = (Impact.Location - Origin).GetSafeNormal();		//射线向量

		// is the angle between the hit and the view within allowed limits (limit + weapon max angle)
		//hit和view的角度是否在限制范围之内，DotProduct表示点积
		const float ViewDotHitDir = FVector::DotProduct(GetInstigator()->GetViewRotation().Vector(), ViewDir);
		//如果 点积 > 
		if (ViewDotHitDir > InstantConfig.AllowedViewDotHitDir - WeaponAngleDot)
		{
			if (CurrentState != EWeaponState::Idle)
			{
				if (Impact.GetActor() == NULL)
				{
					if (Impact.bBlockingHit)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
				}
				// assume it told the truth about static things because the don't move and the hit
				// usually doesn't have significant gameplay implications
				//击中的物体是静态不可移动的
				else if (Impact.GetActor()->IsRootComponentStatic() || Impact.GetActor()->IsRootComponentStationary())
				{
					ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
				}
				else
				{
					// Get the component bounding box
					const FBox HitBox = Impact.GetActor()->GetComponentsBoundingBox();
	
					// calculate the box extent, and increase by a leeway
					FVector BoxExtent = 0.5 * (HitBox.Max - HitBox.Min);
					BoxExtent *= InstantConfig.ClientSideHitLeeway;

					// avoid precision errors with really thin objects
					BoxExtent.X = FMath::Max(20.0f, BoxExtent.X);
					BoxExtent.Y = FMath::Max(20.0f, BoxExtent.Y);
					BoxExtent.Z = FMath::Max(20.0f, BoxExtent.Z);

					// Get the box center
					const FVector BoxCenter = (HitBox.Min + HitBox.Max) * 0.5;

					// if we are within client tolerance
					if (FMath::Abs(Impact.Location.Z - BoxCenter.Z) < BoxExtent.Z &&
						FMath::Abs(Impact.Location.X - BoxCenter.X) < BoxExtent.X &&
						FMath::Abs(Impact.Location.Y - BoxCenter.Y) < BoxExtent.Y)
					{
						ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
					}
					else
					{
						UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s (outside bounding box tolerance)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
					}
				}
			}
		}	//如果点积 < 允许的阙值
		else if (ViewDotHitDir <= InstantConfig.AllowedViewDotHitDir)
		{
			UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s (facing too far from the hit direction)"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
		else
		{
			UE_LOG(LogShooterWeapon, Log, TEXT("%s Rejected client side hit of %s"), *GetNameSafe(this), *GetNameSafe(Impact.GetActor()));
		}
	}
}

bool AShooterWeapon_Instant::ServerNotifyMiss_Validate(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	return true;
}

void AShooterWeapon_Instant::ServerNotifyMiss_Implementation(FVector_NetQuantizeNormal ShootDir, int32 RandomSeed, float ReticleSpread)
{
	const FVector Origin = GetMuzzleLocation();

	// play FX on remote clients
	HitNotify.Origin = Origin;
	HitNotify.RandomSeed = RandomSeed;
	HitNotify.ReticleSpread = ReticleSpread;

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		SpawnTrailEffect(EndTrace);
	}
}

void AShooterWeapon_Instant::ProcessInstantHit(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	if (MyPawn && MyPawn->IsLocallyControlled() && GetNetMode() == NM_Client)
	{
		// if we're a client and we've hit something that is being controlled by the server
		if (Impact.GetActor() && Impact.GetActor()->GetRemoteRole() == ROLE_Authority)
		{
			// notify the server of the hit
			ServerNotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
		}
		else if (Impact.GetActor() == NULL)
		{
			if (Impact.bBlockingHit)
			{
				// notify the server of the hit
				ServerNotifyHit(Impact, ShootDir, RandomSeed, ReticleSpread);
			}
			else
			{
				// notify server of the miss
				ServerNotifyMiss(ShootDir, RandomSeed, ReticleSpread);
			}
		}
	}

	// process a confirmed hit
	ProcessInstantHit_Confirmed(Impact, Origin, ShootDir, RandomSeed, ReticleSpread);
}

void AShooterWeapon_Instant::ProcessInstantHit_Confirmed(const FHitResult& Impact, const FVector& Origin, const FVector& ShootDir, int32 RandomSeed, float ReticleSpread)
{
	//判断是否击中的为垃圾桶，处理垃圾桶受击反馈
	if(Impact.bBlockingHit)
	{
		if(Impact.GetActor()->Tags.Contains(TEXT("Trash")) && Impact.GetActor()->IsRootComponentMovable())
		{
			ProcessTrash(Impact, ShootDir);
		}
	}
	// handle damage
	if (ShouldDealDamage(Impact.GetActor()))
	{
		DealDamage(Impact, ShootDir);
	}

	// play FX on remote clients
	if (GetLocalRole() == ROLE_Authority)
	{
		HitNotify.Origin = Origin;
		HitNotify.RandomSeed = RandomSeed;
		HitNotify.ReticleSpread = ReticleSpread;
	}

	// play FX locally
	if (GetNetMode() != NM_DedicatedServer)
	{
		const FVector EndTrace = Origin + ShootDir * InstantConfig.WeaponRange;
		const FVector EndPoint = Impact.GetActor() ? Impact.ImpactPoint : EndTrace;

		SpawnTrailEffect(EndPoint);
		SpawnImpactEffects(Impact);
	}
}

void AShooterWeapon_Instant::ProcessTrash_Implementation(const FHitResult& Impact, const FVector& ShootDir)
{
	Impact.GetComponent()->AddImpulseAtLocation(ShootDir*5000.0f,Impact.ImpactPoint);
}

bool AShooterWeapon_Instant::ShouldDealDamage(AActor* TestActor) const
{
	// if we're an actor on the server, or the actor's role is authoritative, we should register damage
	if (TestActor)
	{
		if (GetNetMode() != NM_Client ||
			TestActor->GetLocalRole() == ROLE_Authority ||
			TestActor->GetTearOff())
		{
			return true;
		}
	}

	return false;
}

void AShooterWeapon_Instant::DealDamage(const FHitResult& Impact, const FVector& ShootDir)
{
	FPointDamageEvent PointDmg;
	PointDmg.DamageTypeClass = InstantConfig.DamageType;
	PointDmg.HitInfo = Impact;
	PointDmg.ShotDirection = ShootDir;
	//判断是否进行伤害递减
	if(bDecreaseDamage)
	{
		PointDmg.Damage = AfterDecreaseDamage;
	}
	else
	{
		PointDmg.Damage = InstantConfig.HitDamage;
	}
	//处理伤害
	Impact.GetActor()->TakeDamage(PointDmg.Damage, PointDmg, MyPawn->Controller, this);
	//还原
	AfterDecreaseDamage = 0.0f;
	bDecreaseDamage = false;
}

void AShooterWeapon_Instant::DamageDecrease(float CrossLength)
{
	AfterDecreaseDamage = InstantConfig.HitDamage - CrossLength / 10.0f;
	//UE_LOG(LogTemp,Warning,TEXT("子弹在墙体中运动的长度为：%.2f; 子弹伤害由15衰减为%d"),CrossLength,AfterDecreaseDamage)
}

void AShooterWeapon_Instant::OnBurstFinished()
{
	Super::OnBurstFinished();

	CurrentFiringSpread = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
// Weapon usage helpers

float AShooterWeapon_Instant::GetCurrentSpread() const
{
	float FinalSpread = InstantConfig.WeaponSpread + CurrentFiringSpread;
	if (MyPawn && MyPawn->IsTargeting())
	{
		FinalSpread *= InstantConfig.TargetingSpreadMod;
	}

	return FinalSpread;
}

//////////////////////////////////////////////////////////////////////////
// Replication & effects

void AShooterWeapon_Instant::OnRep_HitNotify()
{
	SimulateInstantHit(HitNotify.Origin, HitNotify.RandomSeed, HitNotify.ReticleSpread);
}

void AShooterWeapon_Instant::SimulateInstantHit(const FVector& ShotOrigin, int32 RandomSeed, float ReticleSpread)
{
	FRandomStream WeaponRandomStream(RandomSeed);
	const float ConeHalfAngle = FMath::DegreesToRadians(ReticleSpread * 0.5f);

	const FVector StartTrace = ShotOrigin;
	const FVector AimDir = GetAdjustedAim();
	const FVector ShootDir = WeaponRandomStream.VRandCone(AimDir, ConeHalfAngle, ConeHalfAngle);
	const FVector EndTrace = StartTrace + ShootDir * InstantConfig.WeaponRange;
	
	FHitResult Impact = WeaponTrace(StartTrace, EndTrace);
	if (Impact.bBlockingHit)
	{
		SpawnImpactEffects(Impact);
		SpawnTrailEffect(Impact.ImpactPoint);
	}
	else
	{
		SpawnTrailEffect(EndTrace);
	}
}

void AShooterWeapon_Instant::SpawnImpactEffects(const FHitResult& Impact)
{
	if (ImpactTemplate && Impact.bBlockingHit)
	{
		FHitResult UseImpact = Impact;

		// trace again to find component lost during replication
		if (!Impact.Component.IsValid())
		{
			const FVector StartTrace = Impact.ImpactPoint + Impact.ImpactNormal * 10.0f;
			const FVector EndTrace = Impact.ImpactPoint - Impact.ImpactNormal * 10.0f;
			FHitResult Hit = WeaponTrace(StartTrace, EndTrace);
			UseImpact = Hit;
		}

		FTransform const SpawnTransform(Impact.ImpactNormal.Rotation(), Impact.ImpactPoint);
		AShooterImpactEffect* EffectActor = GetWorld()->SpawnActorDeferred<AShooterImpactEffect>(ImpactTemplate, SpawnTransform);
		if (EffectActor)
		{
			EffectActor->SurfaceHit = UseImpact;
			UGameplayStatics::FinishSpawningActor(EffectActor, SpawnTransform);
		}
	}
}

void AShooterWeapon_Instant::SpawnTrailEffect(const FVector& EndPoint)
{
	if (TrailFX)
	{
		const FVector Origin = GetMuzzleLocation();

		UParticleSystemComponent* TrailPSC = UGameplayStatics::SpawnEmitterAtLocation(this, TrailFX, Origin);
		if (TrailPSC)
		{
			TrailPSC->SetVectorParameter(TrailTargetParam, EndPoint);
		}
	}
}

void AShooterWeapon_Instant::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
	Super::GetLifetimeReplicatedProps( OutLifetimeProps );

	DOREPLIFETIME_CONDITION( AShooterWeapon_Instant, HitNotify, COND_SkipOwner );

	DOREPLIFETIME(AShooterWeapon_Instant,bDecreaseDamage);
	DOREPLIFETIME(AShooterWeapon_Instant,AfterDecreaseDamage);
}