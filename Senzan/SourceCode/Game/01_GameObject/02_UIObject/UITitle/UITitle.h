#pragma once
#include "02_UIObject/UIObject.h"

/*********************************************
*	タイトルUIクラス.
**/
class Select;

class UITitle
{
public:
	enum Items {
		Start,
		End,

		Max
	};

public:
	UITitle();
	~UITitle();

	void Update();
	void LateUpdate();
	void Draw();

	/****************************************
	* @brief 選択中のUI項目を取得.
	****************************************/
	Items GetSelected();

private:
	// 選択肢生成.
	void SelectCreate();
	// 選択肢更新.
	void SelectUpdate();
	// 選択肢遅更新.
	//	走査用.
	void SelectLateUpdate(std::shared_ptr<UIObject> ui);
	// アニメーション初期化.
	//	設定したいUI項目.
	void InitAnim(Items item);
	// アニメーション更新.
	void AnimUpdate();

private:
	std::vector<std::shared_ptr<UIObject>> m_pUIs;
	std::shared_ptr<Select> m_pSelect;
	Items	m_Select;
	bool	m_IsSelected;

	float	m_AnimeSpeed;
	bool	m_AnimReturn;
	float	m_InitAlpha;
	float	m_SelectAlpha;
};