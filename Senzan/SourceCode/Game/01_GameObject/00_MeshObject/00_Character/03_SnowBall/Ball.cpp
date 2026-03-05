#include "Ball.h"

Ball::Ball()
	: MeshObject()
    , ThrowingTime(0.0f)
    , Totle_ThrowingTime(1.5f)
    , m_upColliders(std::make_unique<CompositeCollider>())
    , m_pAttackCollider(nullptr)
    , m_ParriedAnimTime(0.0f)
    , m_State(enSnowBall::Idol)
    , m_IsParried(false)
    , m_IsHitAnimPlaying(false)
    , m_IsFalling(false)
    , m_FallSpeed(50.0f)
    , m_IsLanded(false)
    , m_FadeDuration(2.0f)
    , m_FadeTimer(0.0f)
    , m_GroundY(1.f)
    , m_ShouldDestroy(false)
    , m_HasBrokenVisual(false)
    , m_BounceSpeed(60.0f)
    , m_ParryStartPos({ 0.0f,0.0f,0.0f })
    , m_ParryElapsed(0.0f)
    , m_ParryDuration(1.0f)
{
}

Ball::~Ball()
{
}

void Ball::Update()
{
	MeshObject::Update();
}

void Ball::Draw()
{
	MeshObject::Draw();
}

void Ball::Fire(const DirectX::XMFLOAT3 PlayerPos, const DirectX::XMFLOAT3 BossPos)
{

}

bool Ball::Destroy() const
{
	return false;
}

void Ball::Launch()
{
}

void Ball::HandleCollision()
{
}

void Ball::OnParried()
{
}

void Ball::HandleHitVisual()
{
}
