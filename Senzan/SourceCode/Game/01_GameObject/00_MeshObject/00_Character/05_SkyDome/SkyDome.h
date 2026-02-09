#pragma once

#include "Game/01_GameObject/00_MeshObject/MeshObject.h"

/*********************************************************
*	地面クラス
**/
class SkyDome
    : public MeshObject
{
public:
    SkyDome();
    virtual ~SkyDome();

    virtual void Update() override;
    virtual void Draw() override;

protected:

};
