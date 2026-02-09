#pragma once
#include "02_UIObject/UIObject.h"

/*********************************************
*	選択アニメーションクラス.
**/

class Select
{
public:
	struct WhiteInfo
	{
		DirectX::XMFLOAT3 basePos;	// White の初期座標.
		DirectX::XMFLOAT3 moveDir;	// ZNode 方向へのベクトル（初期化時に計算）.
		DirectX::XMFLOAT3 scale;
	};

public:
	Select();
	~Select();

	void Update();
	void LateUpdate();
	void Draw();

	void Create();

	/****************************************
	* @brief 選択アニメーションの開始.
	* @param pos アニメーションを実行する中心座標.
	****************************************/
	void IsSelect(DirectX::XMFLOAT3 pos);

private:
	// 更新・描画しないUIの除外.
	bool IsHiddenUI(const std::string& name);

private:
	std::vector<std::shared_ptr<UIObject>> m_pUIs;
	std::vector<WhiteInfo> m_Whites;
	DirectX::XMFLOAT3 m_OffsetPos;

	bool m_DoAnimation;
	float m_StartRate;
	float m_Rate;
	float m_Alpha;

	float m_Time;
	float m_Duration;
	float m_MoveDistance;
};