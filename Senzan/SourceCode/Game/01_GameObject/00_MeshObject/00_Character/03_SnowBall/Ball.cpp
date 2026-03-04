#include "Ball.h"

Ball::Ball()
	: MeshObject()
	, m_IsParried(false)
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
