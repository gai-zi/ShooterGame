// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterTypes.h"
#include "ShooterCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShooterCharacterEquipWeapon, AShooterCharacter*, AShooterWeapon* /* new */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnShooterCharacterUnEquipWeapon, AShooterCharacter*, AShooterWeapon* /* old */);


UCLASS(Abstract)
class AShooterCharacter : public ACharacter
{
	GENERATED_UCLASS_BODY()

	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;
	/** spawn inventory, setup initial variables 
	Actor 的组件初始化后调用*/
	virtual void PostInitializeComponents() override;

	/** Update the character. (Running, health etc).
	 * 更新角色(跑、生命值等等)
	 */
	virtual void Tick(float DeltaSeconds) override;

	/** cleanup inventory
	 * 清理目录
	 */
	virtual void Destroyed() override;

	/** update mesh for first person view
	 *
	 * 为FPS视角更新mesh
	 *基类：Tell client that the Pawn is begin restarted. Calls Restart()
	 *基类：通知客户端Pawn开始重置，调用Restart()
	 */
	virtual void PawnClientRestart() override;

	/** [server] perform PlayerState related setup
	 *
	 * [服务端] 执行玩家状态相关设置
	 * 
	 * Pawn:
	 * Called when this Pawn is possessed. Only called on the server (or in standalone).
	 * 当角色被控制时调用
	 
	 * Character:
	 * 如果我们被远程控制，则设置动画时间由客户端的网络更新驱动。因此，时间和事件保持同步。
	 */
	virtual void PossessedBy(class AController* C) override;

	/** [client] perform PlayerState related setup
	 *
	 * [客户端] 执行玩家状态相关设置
	 */
	virtual void OnRep_PlayerState() override;

	/** [server] called to determine if we should pause replication this actor to a specific player
	 *
	 * [客户端] 确定我们是否应该暂停复制这个actor给一个特定的玩家
	 */
	virtual bool IsReplicationPausedForConnection(const FNetViewer& ConnectionOwnerNetViewer) override;

	/** [client] called when replication is paused for this actor
	 *
	 * [客户端] 当这个actor暂停复制时调用
	 */
	virtual void OnReplicationPausedChanged(bool bIsReplicationPaused) override;

	/**
	* Add camera pitch to first person mesh.
	* 
	* 增加到第一人称mesh的摄像机间距
	*	@param	CameraLocation	Location of the Camera.
	*	@param	CameraRotation	Rotation of the Camera.
	* ????????????????????????????????????
	*/
	void OnCameraUpdate(const FVector& CameraLocation, const FRotator& CameraRotation);

	/** get aim offsets
	 *
	 * 获得目标偏移，一系列坐标转换
	 * 
	 * ????????????????????????????????????
	 */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	FRotator GetAimOffsets() const;

	/**
	* Check if pawn is enemy if given controller.
	* 
	*检查是否是敌人
	* @param	TestPC	Controller to check against.
	*/
	bool IsEnemyFor(AController* TestPC) const;

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/**
	* [server] add weapon to inventory
	* 
	*[服务端] 向inventory添加内容
	* @param Weapon	Weapon to add.
	*/
	void AddWeapon(class AShooterWeapon* Weapon);

	/**
	* [server] remove weapon from inventory
	* 
	*[服务端] 从Inventory中移除武器
	* @param Weapon	Weapon to remove.
	*/
	void RemoveWeapon(class AShooterWeapon* Weapon);

	/**
	* Find in inventory
	* @param WeaponClass	Class of weapon to find.
	*/
	class AShooterWeapon* FindWeapon(TSubclassOf<class AShooterWeapon> WeaponClass);

	/**
	* [server + local] equips weapon from inventory
	* 
	* [服务端+本地] 从Inventory装备武器
	* @param Weapon	Weapon to equip
	*/
	void EquipWeapon(class AShooterWeapon* Weapon);

	//////////////////////////////////////////////////////////////////////////
	// Weapon usage

	/** [local] starts weapon fire
	 */
	void StartWeaponFire();

	/** [local] stops weapon fire */
	void StopWeaponFire();

	/** check if pawn can fire weapon */
	bool CanFire() const;

	/** check if pawn can reload weapon */
	bool CanReload() const;

	/** [server + local] change targeting state */
	void SetTargeting(bool bNewTargeting);

	//////////////////////////////////////////////////////////////////////////
	// Movement

	/** [server + local] change running state */
	void SetRunning(bool bNewRunning, bool bToggle);

	//////////////////////////////////////////////////////////////////////////
	// Animations

	/** play anim montage */
	virtual float PlayAnimMontage(class UAnimMontage* AnimMontage, float InPlayRate = 1.f, FName StartSectionName = NAME_None) override;

	/** stop playing montage */
	virtual void StopAnimMontage(class UAnimMontage* AnimMontage) override;

	/** stop playing all montages */
	void StopAllAnimMontages();

	//////////////////////////////////////////////////////////////////////////
	// Input handlers

	/** setup pawn specific input handlers */
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	/**
	* Handle analog trigger for firing
	*
	* @param Val trigger input to apply
	*/
	void FireTrigger(float Val);

	/**
	* Move forward/back
	*
	* @param Val Movment input to apply
	*/
	void MoveForward(float Val);

	/**
	* Strafe right/left
	*
	* @param Val Movment input to apply
	*/
	void MoveRight(float Val);

	/**
	* Move Up/Down in allowed movement modes.
	*
	* @param Val Movment input to apply
	*/
	void MoveUp(float Val);

	/* Frame rate independent turn */
	void TurnAtRate(float Val);

	/* Frame rate independent lookup */
	void LookUpAtRate(float Val);

	/** player pressed start fire action */
	void OnStartFire();

	/** player released start fire action */
	void OnStopFire();

	/** player pressed targeting action */
	void OnStartTargeting();

	/** player released targeting action */
	void OnStopTargeting();

	/** player pressed next weapon action */
	void OnNextWeapon();

	/** player pressed prev weapon action */
	void OnPrevWeapon();

	/** player pressed reload action */
	void OnReload();

	/** player pressed jump action */
	void OnStartJump();

	/** player released jump action */
	void OnStopJump();

	/** player pressed run action */
	void OnStartRunning();

	/** player pressed toggled run action */
	void OnStartRunningToggle();

	/** player released run action */
	void OnStopRunning();

	/*当Pawn碰撞物体时会调用的函数。*/
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit);

	//////////////////////////////////////////////////////////////////////////
	// Reading data

	/** get mesh component */
	USkeletalMeshComponent* GetPawnMesh() const;

	/** get currently equipped weapon */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	class AShooterWeapon* GetWeapon() const;

	/** Global notification when a character equips a weapon. Needed for replication graph. */
	SHOOTERGAME_API static FOnShooterCharacterEquipWeapon NotifyEquipWeapon;

	/** Global notification when a character un-equips a weapon. Needed for replication graph. */
	SHOOTERGAME_API static FOnShooterCharacterUnEquipWeapon NotifyUnEquipWeapon;

	/** get weapon attach point */
	FName GetWeaponAttachPoint() const;

	/** get total number of inventory items
	 *
	 * 获得Inventory列表数量
	 */
	int32 GetInventoryCount() const;

	/**
	* get weapon from inventory at index. Index validity is not checked.
	* 
	* 拿到index下的weapon指针
	* @param Index Inventory index
	*/
	class AShooterWeapon* GetInventoryWeapon(int32 index) const;

	/** get weapon taget modifier speed	*/
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	float GetTargetingSpeedModifier() const;

	/** get targeting state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsTargeting() const;

	/** get firing state */
	UFUNCTION(BlueprintCallable, Category = "Game|Weapon")
	bool IsFiring() const;

	/** get the modifier value for running speed */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	float GetRunningSpeedModifier() const;

	/** get running state */
	UFUNCTION(BlueprintCallable, Category = Pawn)
	bool IsRunning() const;

	/** get camera view type */
	UFUNCTION(BlueprintCallable, Category = Mesh)
	virtual bool IsFirstPerson() const;

	/** get max health */
	int32 GetMaxHealth() const;

	/** check if pawn is still alive */
	bool IsAlive() const;

	/** returns percentage of health when low health effects should start */
	float GetLowHealthPercentage() const;

	/*
	* Get either first or third person mesh.
	*
	* @param	WantFirstPerson		If true returns the first peron mesh, else returns the third
	*/
	USkeletalMeshComponent* GetSpecifcPawnMesh(bool WantFirstPerson) const;

	/** Update the team color of all player meshes. */
	void UpdateTeamColorsAllMIDs();

public:
	UFUNCTION(BlueprintImplementableEvent)
		void LetPawnDissolve();

	/** pawn mesh: 1st person view */
	UPROPERTY(BlueprintReadWrite, Category = Mesh)
	USkeletalMeshComponent* Mesh1P;
protected:
	/** socket or bone name for attaching weapon mesh */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	FName WeaponAttachPoint;

	/** default inventory list */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	TArray<TSubclassOf<class AShooterWeapon> > DefaultInventoryClasses;

	/** weapons in inventory
	 * 武器背包
	 */
	UPROPERTY(Transient, Replicated)
	TArray<class AShooterWeapon*> Inventory;

	/** currently equipped weapon */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_CurrentWeapon)
	class AShooterWeapon* CurrentWeapon;

	/** Replicate where this pawn was last hit and damaged */
	UPROPERTY(Transient, ReplicatedUsing = OnRep_LastTakeHitInfo)
	struct FTakeHitInfo LastTakeHitInfo;

	/** Time at which point the last take hit info for the actor times out and won't be replicated; Used to stop join-in-progress effects all over the screen */
	float LastTakeHitTimeTimeout;

	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Inventory)
	float TargetingSpeedModifier;

	/** current targeting state */
	UPROPERTY(Transient, Replicated)
	uint8 bIsTargeting : 1;
	
	float FirstJumpVelocityZ = 720.0f;
	float SecondJumpVelocityZ = 1000.0f;
	
	/** 是否能进行二段跳 */
	UPROPERTY(Replicated, VisibleDefaultsOnly, Category = Pawn)
	bool bCanDoubleJump;
	
	/** modifier for max movement speed */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	float RunningSpeedModifier;

	/** current running state
	 */
	UPROPERTY(Transient, Replicated)
	uint8 bWantsToRun : 1;

	/** from gamepad running is toggled */
	uint8 bWantsToRunToggled : 1;

	/** current firing state
	 *
	 * bWantsToFire 只有1位，只能表示0或1
	 */
	uint8 bWantsToFire : 1;

	/** when low health effects should start */
	float LowHealthPercentage;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	float BaseTurnRate;

	/** Base lookup rate, in deg/sec. Other scaling may affect final lookup rate. */
	float BaseLookUpRate;

	/** material instances for setting team color in mesh (3rd person view)
	 *
	 *修改材质的实例，设置队伍的mesh颜色(仅第三人称)*/
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> MeshMIDs;

	/** animation played on death */
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* DeathAnim;

	/** sound played on death, local player only */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* DeathSound;

	/** effect played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	UParticleSystem* RespawnFX;

	/** sound played on respawn */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RespawnSound;

	/** sound played when health is low */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* LowHealthSound;

	/** sound played when running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunLoopSound;

	/** sound played when stop running */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* RunStopSound;

	/** sound played when targeting state changes */
	UPROPERTY(EditDefaultsOnly, Category = Pawn)
	USoundCue* TargetingSound;

	/** used to manipulate with run loop sound */
	UPROPERTY()
	UAudioComponent* RunLoopAC;

	/** hook to looped low health sound used to stop/adjust volume */
	UPROPERTY()
	UAudioComponent* LowHealthWarningPlayer;

	/** handles sounds for running */
	void UpdateRunSounds();

	/** handle mesh visibility and updates */
	void UpdatePawnMeshes();

	/** handle mesh colors on specified material instance */
	void UpdateTeamColors(UMaterialInstanceDynamic* UseMID);

	/** Responsible for cleaning up bodies on clients. */
	virtual void TornOff();

private:

	/** Whether or not the character is moving (based on movement input). */
	bool IsMoving();

	//////////////////////////////////////////////////////////////////////////
	// Damage & death

public:

	/** Identifies if pawn is in its dying state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Health)
	uint32 bIsDying : 1;

	// Current health of the Pawn
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = Health)
	float Health;

	/** Take damage, handle death
	 *
	 * 承受伤害、处理死亡
	 *
	 * ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！
	 */
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	/** Pawn suicide */
	virtual void Suicide();

	/** Kill this pawn */
	virtual void KilledBy(class APawn* EventInstigator);

	/** Returns True if the pawn can die in the current state */
	virtual bool CanDie(float KillingDamage, FDamageEvent const& DamageEvent, AController* Killer, AActor* DamageCauser) const;

	/**
	* Kills pawn.  Server/authority only.
	* @param KillingDamage - Damage amount of the killing blow
	* @param DamageEvent - Damage event of the killing blow
	* @param Killer - Who killed this pawn
	* @param DamageCauser - the Actor that directly caused the damage (i.e. the Projectile that exploded, the Weapon that fired, etc)
	* @returns true if allowed
	*/
	virtual bool Die(float KillingDamage, struct FDamageEvent const& DamageEvent, class AController* Killer, class AActor* DamageCauser);

	// Die when we fall out of the world.
	virtual void FellOutOfWorld(const class UDamageType& dmgType) override;

	/** Called on the actor right before replication occurs */
	virtual void PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker) override;
protected:
	/** notification when killed, for both the server and client. */
	virtual void OnDeath(float KillingDamage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser);

	/** play effects on hit */
	virtual void PlayHit(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator, class AActor* DamageCauser);

	/** switch to ragdoll */
	void SetRagdollPhysics();

	/** sets up the replication for taking a hit */
	void ReplicateHit(float Damage, struct FDamageEvent const& DamageEvent, class APawn* InstigatingPawn, class AActor* DamageCauser, bool bKilled);

	/** play hit or death on client */
	UFUNCTION()
	void OnRep_LastTakeHitInfo();

	//////////////////////////////////////////////////////////////////////////
	// Inventory

	/** updates current weapon */
	void SetCurrentWeapon(class AShooterWeapon* NewWeapon, class AShooterWeapon* LastWeapon = NULL);

	/** current weapon rep handler */
	UFUNCTION()
	void OnRep_CurrentWeapon(class AShooterWeapon* LastWeapon);

	/** [server] spawns default inventory */
	void SpawnDefaultInventory();

	/** [server] remove all weapons from inventory and destroy them */
	void DestroyInventory();

	/** equip weapon */
	UFUNCTION(reliable, server, WithValidation)
	void ServerEquipWeapon(class AShooterWeapon* NewWeapon);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetTargeting(bool bNewTargeting);

	/** update targeting state */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSetRunning(bool bNewRunning, bool bToggle);

	/** Builds list of points to check for pausing replication for a connection*/
	void BuildPauseReplicationCheckPoints(TArray<FVector>& RelevancyCheckPoints);

	void TogglePerspective();

	bool IsFPP;

public:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "TogglePerspectiveCamera", ScriptName = "TogglePerspectiveCamera"))
	void K2_TogglePerspectiveCamera(bool bIsFPP);

protected:
	/** Returns Mesh1P subobject **/
	FORCEINLINE USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
};


