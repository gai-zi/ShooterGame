

#include "ShooterGame.h"
#include "Turret.h"
#include "AMissileProjectile.h"
#include "ShooterProjectile.h"


ATurret::ATurret()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	//设置盒体碰撞体
	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxComponent->SetBoxExtent(FVector(80.f,80.f,120.f));
	BoxComponent->AlwaysLoadOnClient = true;
	BoxComponent->AlwaysLoadOnServer = true;
	
	RootComponent = BoxComponent;
	//设置最下层底座
	Base = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base1"));
	Base->SetRelativeLocation(FVector(0.0f,0.0f,-120.f));
	Base->SetRelativeRotation(FRotator::ZeroRotator);
	Base->SetupAttachment(RootComponent);
	
	//设置倒数第二层底座
	Base2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base2"));
	Base2->SetRelativeLocation(FVector(0.0f,0.0f,60.0f));
	Base2->SetRelativeRotation(FRotator::ZeroRotator);
	Base2->SetupAttachment(Base);
	//设置 炮台 属性
	Launcher = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Launcher"));
	Launcher->SetRelativeLocation(FVector(0.0f,0.0f,144.5f));
	Launcher->SetupAttachment(Base2);

	// 创建相机
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OurCamera"));
	OurCamera->SetRelativeLocation(FVector(-374.0f, 0.0f, 269.0f));
	OurCamera->SetRelativeRotation(FRotator(0.880477f, -9.961546f, -5.076797f));
	OurCamera->SetupAttachment(Base2);

	this->SetActorScale3D(FVector(0.5f, 0.5f, 0.5f));
	
	static ConstructorHelpers::FClassFinder<AShooterProjectile> ProjectileBP(TEXT("Blueprint'/Game/Blueprints/Weapons/BP_Missile.BP_Missile_C'"));
	if(ProjectileBP.Succeeded())
	{
		ProjectileClass = ProjectileBP.Class;
	}
}


void ATurret::BeginPlay()
{
	Super::BeginPlay();
	
}


void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if(!bCanFire && LastFireTime <= 1.5f)
	{
		LastFireTime += DeltaTime;
		if(LastFireTime >= 1.5f) bCanFire = true;
	}
}


void ATurret::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	//設置鍵盤綁定
	PlayerInputComponent->BindAction("Fire",IE_Pressed,this,&ATurret::Fire);
	//设置鼠标绑定
	PlayerInputComponent->BindAxis("Turn",this,&ATurret::TurnRight);
	//PlayerInputComponent->BindAxis("LookUp",this,&ATurret::TurnUp);

}

void ATurret::TurnRight(float Value)
{
	//沿着Z轴旋转roll
	float Base2Yaw = Base2->GetRelativeRotation().Yaw;
	if(Value!=0.0f)
	{
		Base2->SetRelativeRotation(FRotator(0.0f,Base2Yaw+Value,0.0f));
	}
}


void ATurret::Fire()
{
	if(!bCanFire)	return;
	//获取炮台基座的世界坐标下的旋转
	FRotator BaseWorldRotation = this->GetActorRotation();
	FRotator Base2WorldRotation = Base2->GetRelativeRotation();
	
	FVector BoxWorldLocation = this->GetActorLocation();
	FVector BaseWorldLocation = Base->GetRelativeLocation();
	FVector Base2WorldLocation = Base2->GetRelativeLocation();
	FVector LuncherWorldLocation = Launcher->GetRelativeLocation();

	FRotator MissileWorldRotator = BaseWorldRotation + Base2WorldRotation;
	FVector MissileWorldLocation = BoxWorldLocation + BaseWorldLocation + Base2WorldLocation + LuncherWorldLocation - FVector(0.0f,0.0f,30.0f);
	
	UWorld* World = GetWorld();
	
	if(World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		
		if(ProjectileClass)
		{
			AShooterProjectile* Proj= World->SpawnActor<AShooterProjectile>(ProjectileClass,MissileWorldLocation,MissileWorldRotator,SpawnParams);
		}
	}
	bCanFire = false;
	LastFireTime = 0.0f;
}



