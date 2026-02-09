#include "SpriteCollider.h"

bool SpriteCollider::PointInSquare(POINT point, DirectX::XMFLOAT2 pos, DirectX::XMFLOAT2 size)
{
	if (point.x < pos.x
		&& point.y < pos.y
		&& pos.x + size.x < point.x
		&& pos.y + size.y < point.y)
	{
		return true;
	}
	return false;
}
