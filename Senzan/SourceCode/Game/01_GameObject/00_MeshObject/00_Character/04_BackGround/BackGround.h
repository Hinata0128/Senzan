#pragma once

#include "Game//01_GameObject//00_MeshObject//MeshObject.h"

/*********************************************************************
*	‹ó‚É‰æ‘œ‚ð“ü‚ê‚éƒNƒ‰ƒX.
**/

class BackGround
	: public MeshObject
{
public:
	BackGround();
	~BackGround();

	void Update() override;
	void Draw() override;

private:

};