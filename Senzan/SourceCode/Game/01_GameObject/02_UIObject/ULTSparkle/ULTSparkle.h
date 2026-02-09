#pragma once
#pragma once
#include "02_UIObject/UIObject.h"

/*********************************************
*	ULT演出クラス.
**/

class ULTSparkle
{
public :
    struct SparkleParticle
    {
        std::shared_ptr<UIObject> ui;
        DirectX::XMFLOAT2 velocity = {0.f,0.f}; // 移動方向＋速度
    };

public:
    ULTSparkle(std::shared_ptr<UIObject> pObje);
    ~ULTSparkle();

    void Update();
    void LateUpdate();
    void Draw();

    void Create();

    // スパークルのアニメーションを実行.
    void DoPeakAnim();
    // ゲージが最大かどうかを外部からセットする
    void SetULTGaugeStatus(bool isMax, DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 size);

private:
    void UpDateParticles(float dt);
private:
    std::shared_ptr<UIObject> m_pMainSparkle;
    std::vector<SparkleParticle> m_Particles;

    DirectX::XMFLOAT2 m_GaugeX;
    DirectX::XMFLOAT2 m_GaugeY;

    bool   m_IsAnimating;
    float  m_Timer;
    float  m_Duration;
    float  m_StartScale;
    float  m_PeakScale;
    float  m_ToPeakTime;
    float  m_PeakTime;

    float  m_StartRotSpead;
    float  m_PeakRotSpead;
    float  m_EndRotSpead;

    bool  m_IsGaugeMax;
    float m_SpawnTimer;
    float m_SpawnInterval;
    float m_ParticlesStartAlpha;
    float m_ParticlesDecAlpha;
    float m_ParticlesSize;

    float m_ParticlesAngle;
    float m_ParticlesSpead;

};
