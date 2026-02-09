#pragma once

#include "../../00_Base/SceneBase.h"
#include "02_UIObject/UIObject.h"

class UIEditor
	: public SceneBase
{
public:
	UIEditor();
	~UIEditor() override;

	void Initialize() override;
	void Create() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	HRESULT LoadData();
private:
	// 選択したシーンのUIを読み込み
	void SelectSceneLoad(const std::string& sceneName);
	// 現在シーンのUI情報を保存
	HRESULT SaveScene();
	// キー入力処理
	void KeyInput();

	//-------------------------------------------
	//	ImGui使用関数
	//-------------------------------------------
	// シーン選択関数
	void ImGuiSelectScene();
	// UIリスト検索関数
	void ImGuiSearchUI();
	// SpriteManagerからUIを追加
	void AddDeleteSprite();
	// 名前変更を行う
	void RenameUIObjects();

	// 座標調整関数(選択されたUIObect)
	void ImGuiPosEdit(std::shared_ptr<UIObject> object);
	// Z座標を元にソートする関数(選択されたUIObect)
	void SortBySpritePosZ(std::shared_ptr<UIObject> object);
	// 情報調整関数(選択されたUIObect)
	void ImGuiInfoEdit(std::shared_ptr<UIObject> object);
	// その他の情報調整関数(選択されたUIObect)
	void ImGuiEtcInfoEdit(std::shared_ptr<UIObject> object);

	void TriggeHgihLight();
	void HighLightUI(std::shared_ptr<UIObject> object);
private:
	std::string m_CurrentSceneName;
	bool m_MoveAny;	// UI情報変更の有無.

	// 画像情報リスト
	std::vector<std::string> m_SpriteDataList;	// スプライト情報をまとめる配列
	std::vector<DirectX::XMFLOAT3> m_SpritePosList;	// スプライト座標をまとめる配列

	std::vector<std::shared_ptr<UIObject>> m_pUIs;				// UIクラス
	std::vector<std::shared_ptr<Sprite2D>> m_pSprite2Ds;		// Sprite2Dクラス

	std::string m_ScenePath;	// 現在のシーンパス

	float	m_DragValue;				// 調整値
	int		m_SelectedUIIndex;			// 選択中UIのインデックス
	char	m_SearchBuffer[64] = "";	// 検索用バッファ
	
	int		m_SpriteSelectedUIIndex;	// 選択中追加可能UIのインデックス
	char	m_SpriteSearchBuffer[64] = "";	// 検索用バッファ
	std::string m_SelectedSpriteName;	// 選択されたUI名

	// マウス操作用の変数
	DirectX::XMFLOAT2 m_OffsetPos;	// マウス座標と画像座標のズレ補正値
	bool m_DoDrag;				// ドラッグ中
	bool m_MovedSpritePos;		// 画像座標が変更された際のフラグ

	float	m_HighlightTime;
};