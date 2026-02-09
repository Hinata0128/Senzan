#pragma once
#include "02_UIObject/UIObject.h"

/*********************************************
*	エンディングUIクラス.
**/
class Select;

class UIEnding
{
public:
    UIEnding();
    ~UIEnding();

    void Update();
    void LateUpdate();
    void Draw();

private:
    // 選択肢生成.
    void SelectCreate();
    void SelectUpdate();
    void SelectLateUpdate(std::shared_ptr<UIObject> ui);
    void AnimUpdate();

private:
    std::vector<std::shared_ptr<UIObject>> m_pUIs;
    std::shared_ptr<Select> m_pSelect;

    float	m_AnimeSpeed;
    bool	m_AnimReturn;
    float	m_InitAlpha;
    float	m_SelectAlpha;
    float   m_GlobalAlpha;
    bool    m_IsFadeIn;
    float   m_FadeSpeed;
};
