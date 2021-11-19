

#include "ShooterGame.h"
#include "AMissileProjectile.h"



AAMissileProjectile::AAMissileProjectile()
{

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxComponent->InitBoxExtent(FVector(140.0f,8.0f,8.0f));
	BoxComponent->AlwaysLoadOnClient = true;
	BoxComponent->AlwaysLoadOnServer = true;
	BoxComponent->BodyInstance.SetCollisionProfileName(TEXT("OverapAll"));
	BoxComponent->OnComponentBeginOverlap.AddDynamic(this,&AAMissileProjectile::OnOverlapBegin);
	RootComponent = BoxComponent;
	
	//设置静态网格体属性
	MissileStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh>MissileStaticMesh(TEXT("'/Game/CIWSTurret/Meshes/SM_Missile.SM_Missile'"));
	if(MissileStaticMesh.Succeeded())
	{
		MissileStaticMeshComponent->SetStaticMesh(MissileStaticMesh.Object);
	}
	MissileStaticMeshComponent->SetRelativeLocation(FVector(-140.f,0.0f,0.0f));
	MissileStaticMeshComponent->SetRelativeScale3D(FVector(1.0f,1.0f,1.0f));
	MissileStaticMeshComponent->SetupAttachment(RootComponent);
	
	//伴随粒子效果
	MissileParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem>MissileParticleSystemInGame(TEXT("'/Game/CIWSTurret/ExampleContent/Effects/ParticleSystems/Weapons/RocketLauncher/Muzzle/P_Launcher_proj.P_Launcher_proj'"));
	if(MissileParticleSystemInGame.Succeeded())
	{
		MissileParticleSystem->Template = MissileParticleSystemInGame.Object;
	}
	MissileParticleSystem->SetupAttachment(MissileStaticMeshComponent);
	//Audio
	MissileAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("Audio"));
	static ConstructorHelpers::FObjectFinder<USoundCue>MissileAudioInGame(TEXT("'/Game/CIWSTurret/ExampleContent/Sounds/Weapon_GrenadeLauncher/Mono/GrenadeLauncher_ProjectileLoop_Cue.GrenadeLauncher_ProjectileLoop_Cue'"));
	if(MissileAudioInGame.Succeeded())
	{
		MissileAudio->Sound = MissileAudioInGame.Object;
	}
	MissileAudio->SetupAttachment(MissileStaticMeshComponent);
	
	/*/*发生碰撞，爆炸特效#1#
	ExplodeParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System"));
	static ConstructorHelpers::FObjectFinder<UParticleSystem>ExplodeParticleSystemInGame(TEXT("'/Game/Effects/ParticleSystems/Weapons/RocketLauncher/Impact/P_Launcher_IH.P_Launcher_IH'"));
	if(ExplodeParticleSystemInGame.Succeeded())
	{
		ExplodeParticleSystem->Template = ExplodeParticleSystemInGame.Object;
	}*/
	
	//发射物移动组件
	MissileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovemen"));
	MissileMovementComponent->SetUpdatedComponent(BoxComponent);
	MissileMovementComponent->InitialSpeed = 2000.0f;
	MissileMovementComponent->MaxSpeed = 2000.0f;
	MissileMovementComponent->ProjectileGravityScale = 0.0f;
	MissileMovementComponent->bRotationFollowsVelocity;
	
 	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}


void AAMissileProjectile::BeginPlay()
{
	Super::BeginPlay();
}
/*重叠事件开始*/
void AAMissileProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

void AAMissileProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAMissileProjectile::FireInDirection(FVector direction) const
{
	MissileMovementComponent->Velocity = direction * MissileMovementComponent->InitialSpeed;
}

/*void AAMissileProjectile::OnHit()
{
	//UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeParticleSystem, GetActorLocation());
	Destroy();
}*/

