#include "UIEditor.h"
#include "FileManager/FileManager.h"
#include "ResourceManager/SpriteManager/SpriteManager.h"
#include "Game/05_InputDevice/Input.h"
#include "Game/03_Collision/Sprite/SpriteCollider.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Game//04_Time//Time.h"
#include "Singleton/SceneManager/SceneManager.h"
#include "ResourceManager/ResourceManager.h"

#include "ImGui/CImGuiManager.h"

using Json = nlohmann::json;

namespace {
	char m_NewSceneName[64] = ""; // 新規作成用バッファをメンバ変数に追加.
}

UIEditor::UIEditor()
{
}
UIEditor::~UIEditor()
{
}

//-----------------------------------------------------------------------.

void UIEditor::Create()
{
	SelectSceneLoad("Title");
}

//-----------------------------------------------------------------------.

HRESULT UIEditor::LoadData()
{
	return E_NOTIMPL;
}

//-----------------------------------------------------------------------.

void UIEditor::Initialize()
{
}


//=============================================================================
//		更新処理.
//=============================================================================
void UIEditor::Update()
{
	// キー入力
	KeyInput();

	// シーンを選択する
	ImGuiSelectScene();

	//--------------------------------------------------------------
	//		UIの追加と削除.
	//--------------------------------------------------------------
	ImGui::Begin(IMGUI_JP("UIの追加・削除"));
	// シーンにUIを追加.
	AddDeleteSprite();
	// 画像名を変更し名前被りに対処.
	RenameUIObjects();
	ImGui::End();

	//--------------------------------------------------------------
	//		UI調整.
	//--------------------------------------------------------------
	ImGui::Begin(IMGUI_JP("UIエディターウィンドウ"));
	// UIリストの検索関数.
	ImGuiSearchUI();
	
	//-----------------------------------------------------------
	//		選択中のオブジェクトの編集.
	//-----------------------------------------------------------
	if (m_SelectedUIIndex >= 0 && m_SelectedUIIndex < m_pUIs.size()) {
		// 選択されているUIを表示.
		ImGui::Text(IMGUI_JP("選択されているUI: %s"), m_pUIs[m_SelectedUIIndex]->GetUIName().c_str());


		// 座標の調整.
		ImGuiPosEdit(m_pUIs[m_SelectedUIIndex]);
		// Z座標を基準にソート.
		SortBySpritePosZ(m_pUIs[m_SelectedUIIndex]);

		// 画像情報の調整.
		ImGuiInfoEdit(m_pUIs[m_SelectedUIIndex]);
		// その他の情報の調整.
		ImGuiEtcInfoEdit(m_pUIs[m_SelectedUIIndex]);

		HighLightUI(m_pUIs[m_SelectedUIIndex]);
	}

	ImGui::End();

	//--------------------------------------------------------------
	//		保存する.
	//--------------------------------------------------------------
	ImGui::Begin(IMGUI_JP("UI保存ウィンドウ"));
	if (ImGui::Button(IMGUI_JP("UIを保存"))) {
		SaveScene();
		m_MoveAny = false;
	}

	ImGui::End();
}

//-----------------------------------------------------------------------.

void UIEditor::LateUpdate() 
{
}

//-----------------------------------------------------------------------.

void UIEditor::KeyInput()
{
	m_DragValue = 1.f;
	if (GetAsyncKeyState(VK_SHIFT) && 0x8000) { m_DragValue *= 0.01f; }
	if (GetAsyncKeyState(VK_SPACE) && 0x8000) { m_DragValue *= 0.001f; }
}

//=============================================================================
//		描画処理.
//=============================================================================
void UIEditor::Draw()
{
	if (m_pUIs.empty()) { return; }
	DirectX11::GetInstance().SetDepth(false);
	for (size_t i = 0; i < m_pUIs.size(); ++i) { m_pUIs[i]->Draw(); }
	DirectX11::GetInstance().SetDepth(true);
}


//-----------------------------------------------------------------------------
//		選択したシーンのUIを読み込み.
//-----------------------------------------------------------------------------
void UIEditor::SelectSceneLoad(const std::string& sceneName)
{
	m_pSprite2Ds.clear();
	for (auto sprite : m_pSprite2Ds) sprite.reset();
	for (auto ui : m_pUIs) ui.reset();
	m_pUIs.clear();
	m_SpritePosList.clear();

	m_CurrentSceneName = sceneName;
	m_ScenePath = "Data\\Image\\Sprite\\UIData\\" + sceneName + ".json";

	// JSON読み込み.
	Json jsonData = FileManager::JsonLoad(m_ScenePath);

	// 空なら初期UIを1個追加.
	if (jsonData.is_null() || jsonData.empty()) {
		std::shared_ptr<Sprite2D> sprite = std::make_shared<Sprite2D>();
		std::shared_ptr<UIObject> ui = std::make_shared<UIObject>();

		sprite->Initialize("Data\\Image\\Sprite\\Other\\White.png"); // 黒画像を初期で出す.

		ui->AttachSprite(sprite);
		ui->SetPosition(0.f, 0.f, 0.f);

		m_pSprite2Ds.push_back(sprite);
		m_pUIs.push_back(ui);
		m_SpritePosList.push_back(ui->GetPosition());

		// ついでにSaveScene()で書き込んでおく.
		SaveScene();
	}

	// 保存されたUIデータを読み込み、展開.
	for (auto& [imageName, spriteArray] : jsonData.items()) {
		// 拡張子が .json ならスキップ.
		std::string::size_type dotPos = imageName.find_last_of('.');
		if (dotPos != std::string::npos) {
			std::string ext = imageName.substr(dotPos);
			if (ext == ".json" || ext == ".JSON") continue;
		}

		// スプライト取得.
		std::shared_ptr<Sprite2D> Sprite = SpriteManager::GetSprite2D(GetBaseName(imageName));
		if (!Sprite) {
			MessageBoxA(NULL, ("スプライトが見つかりません: " + imageName).c_str(), "Error", MB_OK);
			continue;
		}

		// 各UIインスタンスを展開.
		for (auto& value : spriteArray) {
			std::shared_ptr<UIObject> ui = std::make_shared<UIObject>();

			ui->SetPosition(DirectX::XMFLOAT3(value["Pos"]["x"], value["Pos"]["y"], value["Pos"]["z"]));
			ui->SetColor(DirectX::XMFLOAT4(value["Color"]["x"], value["Color"]["y"], value["Color"]["z"], value["Color"]["a"]));
			ui->SetAlpha(value["Alpha"]);
			ui->SetScale(DirectX::XMFLOAT3(value["Scale"]["x"], value["Scale"]["y"], value["Scale"]["z"]));
			ui->SetPivot(DirectX::XMFLOAT2(value["Pivot"]["x"], value["Pivot"]["y"]));
			ui->SetRotation(DirectX::XMFLOAT3(value["Rotate"]["x"], value["Rotate"]["y"], value["Rotate"]["z"]));
			ui->SetDrawSize(DirectX::XMFLOAT2(value["Disp"]["w"], value["Disp"]["h"]));

			// リストに追加.
			m_pSprite2Ds.push_back(Sprite);
			ui->AttachSprite(Sprite);
			m_pUIs.push_back(ui);
			m_SpritePosList.push_back(ui->GetPosition());
		}
	}
	std::sort(m_pUIs.begin(), m_pUIs.end(), [](std::shared_ptr<UIObject> a, std::shared_ptr<UIObject> b) {
		return a->GetPosition().z < b->GetPosition().z;
		});
}

//-----------------------------------------------------------------------.

HRESULT UIEditor::SaveScene()
{
	Json jsonData;
	for (size_t i = 0; i < m_pUIs.size(); ++i)
	{
		std::string imageName = m_pUIs[i]->GetUIName();

		// 画像名ごとのリストにUI情報を追加.
		Json SpriteState;
		SpriteState["Pos"]["x"] = m_pUIs[i]->GetPosition().x;
		SpriteState["Pos"]["y"] = m_pUIs[i]->GetPosition().y;
		SpriteState["Pos"]["z"] = m_pUIs[i]->GetPosition().z;
		SpriteState["Disp"]["w"] = m_pUIs[i]->GetDrawSize().x;
		SpriteState["Disp"]["h"] = m_pUIs[i]->GetDrawSize().y;

		SpriteState["Color"]["x"] = m_pUIs[i]->GetColor().x;
		SpriteState["Color"]["y"] = m_pUIs[i]->GetColor().y;
		SpriteState["Color"]["z"] = m_pUIs[i]->GetColor().z;
		SpriteState["Color"]["a"] = m_pUIs[i]->GetColor().w;
		SpriteState["Alpha"] = m_pUIs[i]->GetAlpha();

		SpriteState["Scale"]["x"] = m_pUIs[i]->GetScale().x;
		SpriteState["Scale"]["y"] = m_pUIs[i]->GetScale().y;
		SpriteState["Scale"]["z"] = m_pUIs[i]->GetScale().z;
		SpriteState["Pivot"]["x"] = m_pUIs[i]->GetPivot().x;
		SpriteState["Pivot"]["y"] = m_pUIs[i]->GetPivot().y;
		SpriteState["Rotate"]["x"] = m_pUIs[i]->GetRotation().x;
		SpriteState["Rotate"]["y"] = m_pUIs[i]->GetRotation().y;
		SpriteState["Rotate"]["z"] = m_pUIs[i]->GetRotation().z;

		// jsonData[画像名] に配列として追加.
		jsonData[imageName].push_back(SpriteState);
	}
	std::string outPath = "Data\\Image\\Sprite\\UIData\\" + m_CurrentSceneName + ".json";
	if (!SUCCEEDED(FileManager::JsonSave(outPath, jsonData))) return E_FAIL;

	return S_OK;
}

//-----------------------------------------------------------------------.

void UIEditor::SortBySpritePosZ(std::shared_ptr<UIObject> object)
{
	// 何も座標に関して変更がない場合早期リターン.
	if (!m_MovedSpritePos) { return; }

	// ソート前の選択されていた UI のポインタを保存.
	std::shared_ptr<UIObject> pPreviousSelectedUI = object;

	// Z座標を基準にソートする.
	std::sort(m_pUIs.begin(), m_pUIs.end(),
		[](const std::shared_ptr<UIObject>& a, const std::shared_ptr<UIObject>& b) {
			if (!a || !b) return false; // UIObjectがnullptrなら後ろに.

			const auto& posA = a->GetPosition();
			const auto& posB = b->GetPosition();

			return posA.z < posB.z;
		});

	// ソート後に、以前選択されていた UI がまだリストに存在するか確認し、再選択.
	if (pPreviousSelectedUI != nullptr)
	{
		auto it = std::find(m_pUIs.begin(), m_pUIs.end(), pPreviousSelectedUI);
		if (it != m_pUIs.end()) {
			// 再選択.
			m_SelectedUIIndex = static_cast<int>(std::distance(m_pUIs.begin(), it));
		}
		else {
			// 以前選択されていたUIがリストにない場合.
			object = nullptr;
			m_SelectedUIIndex = 0; // または適切なデフォルト値.
		}
	}

	m_MovedSpritePos = false;
}

//-----------------------------------------------------------------------.

void UIEditor::ImGuiSelectScene()
{
	ImGui::Begin(IMGUI_JP("シーン管理"));

	// 新規シーンの作成.
	ImGui::InputText(IMGUI_JP("新規シーン名"), m_NewSceneName, IM_ARRAYSIZE(m_NewSceneName));
	if (ImGui::Button(IMGUI_JP("新規シーン作成"))) {
		std::string newPath = "Data\\Image\\Sprite\\UIData\\" + std::string(m_NewSceneName) + ".json";
		if (!std::filesystem::exists(newPath)) {
			std::ofstream ofs(newPath);
			ofs << "{}"; // 空のJSONを書き込む.
			ofs.close();
		}
		m_CurrentSceneName = m_NewSceneName;
		SelectSceneLoad(m_CurrentSceneName);
		m_MoveAny = false;
	}

	ImGui::Separator();
	ImGui::Text(IMGUI_JP("既存のシーン"));

	static std::string sceneToDelete; // 削除候補のシーン名.
	static bool showDeleteConfirm = false; // 削除確認ダイアログ表示フラグ.

	for (const auto& entry : std::filesystem::directory_iterator("Data\\Image\\Sprite\\UIData\\")) {
		if (entry.path().extension() == ".json") {
			std::string sceneName = entry.path().stem().string();

			ImGui::PushID(sceneName.c_str());

			if (ImGui::Button(sceneName.c_str())) {
				m_CurrentSceneName = sceneName;
				SelectSceneLoad(m_CurrentSceneName);
				m_MoveAny = false;
			}
			ImGui::SameLine();

			if (ImGui::Button(IMGUI_JP("削除"))) {
				sceneToDelete = sceneName;
				showDeleteConfirm = true;
			}

			ImGui::PopID();
		}
	}

	// 削除確認モーダル.
	if (showDeleteConfirm) {
		ImGui::OpenPopup(IMGUI_JP("削除確認"));
	}
	if (ImGui::BeginPopupModal(IMGUI_JP("削除確認"), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("%s\n%s", sceneToDelete.c_str(), IMGUI_JP("を削除しますか？"));
		ImGui::Separator();

		if (ImGui::Button(IMGUI_JP("はい"), ImVec2(120, 0))) {
			std::string deletePath = "Data\\Image\\Sprite\\UIData\\" + sceneToDelete + ".json";
			if (std::filesystem::exists(deletePath)) {
				try {
					std::filesystem::remove(deletePath);
				}
				catch (...) {}
			}

			if (m_CurrentSceneName == sceneToDelete) {
				// 削除対象のシーンを現在のシーンから外す.
				m_CurrentSceneName.clear();

				// UIなどのデータをクリア.
				m_pUIs.clear();
				m_pSprite2Ds.clear();
				m_SpritePosList.clear();
				m_MoveAny = false;
			}

			sceneToDelete.clear();
			showDeleteConfirm = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button(IMGUI_JP("いいえ"), ImVec2(120, 0))) {
			sceneToDelete.clear();
			showDeleteConfirm = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}

//-----------------------------------------------------------------------------
//		UIリスト検索関数.
//-----------------------------------------------------------------------------
void UIEditor::ImGuiSearchUI()
{
	if (ImGui::TreeNodeEx(IMGUI_JP("UIリスト"), ImGuiTreeNodeFlags_DefaultOpen)) {
		// 検索バー.
		ImGui::InputText(IMGUI_JP("検索"), m_SearchBuffer, IM_ARRAYSIZE(m_SearchBuffer));
		// スクロール可能なリスト.
		ImGui::BeginChild(IMGUI_JP("リスト"), ImVec2(315, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (int i = 0; i < m_pUIs.size(); ++i) {
			// 検索フィルタリング.
			if (strlen(m_SearchBuffer) > 0
				&& m_pUIs[i]->GetUIName().find(m_SearchBuffer) == std::string::npos) {
				continue;
			}

			bool isSelected = (m_SelectedUIIndex == i);
			if (ImGui::Selectable(m_pUIs[i]->GetUIName().c_str(), isSelected)) {
				m_SelectedUIIndex = i; // 選択更新.
				TriggeHgihLight();
			}
		}
		ImGui::EndChild();
		ImGui::TreePop();
	}
}

//-----------------------------------------------------------------------.

void UIEditor::AddDeleteSprite()
{
	std::vector<std::string> spriteNames = SpriteManager::GetSprite2D2List();
	if (ImGui::TreeNodeEx(IMGUI_JP("追加可能UIリスト"), ImGuiTreeNodeFlags_DefaultOpen)) {
		// 検索バー.
		ImGui::InputText(IMGUI_JP("検索"), m_SpriteSearchBuffer,
			IM_ARRAYSIZE(m_SpriteSearchBuffer));
		// スクロール可能なリスト.
		ImGui::BeginChild(IMGUI_JP("リスト"),
			ImVec2(315, 200), true, ImGuiWindowFlags_HorizontalScrollbar);

		for (int i = 0; i < spriteNames.size(); ++i) {
			const std::string& name = spriteNames[i];

			// 検索フィルタ.
			if (strlen(m_SpriteSearchBuffer) > 0 &&
				name.find(m_SpriteSearchBuffer) == std::string::npos) {
				continue;
			}

			bool isSelected = (m_SelectedSpriteName == name);
			if (ImGui::Selectable(name.c_str(), isSelected)) {
				m_SelectedSpriteName = name; // 選択更新.
			}
		}
		ImGui::EndChild();

		// 選択されたスプライトをUIとして追加.
		if (ImGui::Button(IMGUI_JP("UI追加"))) {
			if (!m_SelectedSpriteName.empty()) {
				std::shared_ptr<Sprite2D> pSprite = SpriteManager::GetSprite2D(m_SelectedSpriteName);
				if (!pSprite) return;

				std::shared_ptr<UIObject> ui = std::make_shared<UIObject>();
				ui->AttachSprite(pSprite);

				m_pSprite2Ds.push_back(pSprite);
				m_pUIs.push_back(ui);
				m_SpritePosList.push_back(ui->GetPosition());

				TriggeHgihLight();
			}
		}

		// 選択されたUIを削除.
		if (ImGui::Button(IMGUI_JP("UI削除")) && !m_pUIs.empty()) {
			m_pUIs[m_SelectedUIIndex].reset();
			
			m_pUIs.erase(m_pUIs.begin() + m_SelectedUIIndex);
			m_pSprite2Ds.erase(m_pSprite2Ds.begin() + m_SelectedUIIndex);
			m_SpritePosList.erase(m_SpritePosList.begin() + m_SelectedUIIndex);
			// インデックスをリセット.
			m_SelectedUIIndex = 0;
		}

		ImGui::TreePop();
	}
}

//-----------------------------------------------------------------------.

void UIEditor::RenameUIObjects()
{
	if (m_pUIs.empty()) { return; }
	std::vector<std::pair<std::string, std::shared_ptr<UIObject>>> nameUIList;

	// UI名とUIオブジェクトのペアを収集.
	for (std::shared_ptr<UIObject> ui : m_pUIs) {
		std::string baseName = GetBaseName(ui->GetResourceName());
		nameUIList.emplace_back(baseName, ui);
	}

	// ナンバリングして名前を再設定.
	std::unordered_map<std::string, int> nameCount;

	for (auto& [baseName, ui] : nameUIList) {
		int index = nameCount[baseName]++;
		std::string newName = baseName + "_" + std::to_string(index);
		ui->SetUIName(newName);
	}
}


//-----------------------------------------------------------------------------
//		座標調整関数(選択されたUIObect).
//-----------------------------------------------------------------------------
void UIEditor::ImGuiPosEdit(std::shared_ptr<UIObject> object)
{
	if (ImGui::TreeNode(IMGUI_JP("座標")))
	{
		// ドラッグ&ドロップ用にマウス操作のDirectInputを用意.
		DirectX::XMFLOAT3 pos = object->GetPosition();
		bool posdrag = ImGui::DragFloat3("##Position", &pos.x, m_DragValue);
	
		POINT MousePos;
		GetCursorPos(&MousePos);
		RECT rect;
		GetWindowRect(GetForegroundWindow(), &rect);
		DirectX::XMFLOAT2 objectpos = DirectX::XMFLOAT2(object->GetPosition().x + rect.left, object->GetPosition().y + rect.top);

		// 画像範囲内で左クリック入力中の場合、ドラッグ操作を開始.
		if (SpriteCollider::PointInSquare(MousePos, objectpos,object->GetDrawSize()) && !m_DoDrag) {
			if (Input::IsMouseGrab()) {
				m_DoDrag = true;
				m_OffsetPos = DirectX::XMFLOAT2(pos.x - MousePos.x, pos.y - MousePos.y);
			}
		}
		if (m_DoDrag) {
			posdrag = true;
			// 補正値+マウス座標した座標を入れる.
			pos = DirectX::XMFLOAT3(MousePos.x + m_OffsetPos.x, MousePos.y + m_OffsetPos.y, pos.z);
			// マウスの左クリックを話した場合、ドラッグ操作を停止.
			if (!Input::IsMouseGrab()) { m_DoDrag = false; }
		}

		// 変更があった場合保存する.
		if (posdrag) {
			object->SetPosition(pos);
			m_MoveAny = true;
			m_MovedSpritePos = true;
		}
		ImGui::TreePop();
	}
}

//-----------------------------------------------------------------------.

void UIEditor::ImGuiInfoEdit(std::shared_ptr<UIObject> object)
{
	if (ImGui::TreeNode(IMGUI_JP("画像情報")))
	{
		DirectX::XMFLOAT2 disp = DirectX::XMFLOAT2(
			object->GetDrawSize().x,
			object->GetDrawSize().y);

		ImGui::Text(IMGUI_JP("表示サイズ(x,y)"));
		bool dispdrag = ImGui::DragFloat2("##DispDrag", &disp.x, m_DragValue);

		// 変更があった場合保存する.
		if (dispdrag)
		{
			object->SetDrawSize(disp);
			m_MoveAny = true;
		}
		ImGui::TreePop();
	}
}

//-----------------------------------------------------------------------------
//		その他の情報調整関数(選択されたUIObect).
//-----------------------------------------------------------------------------
void UIEditor::ImGuiEtcInfoEdit(std::shared_ptr<UIObject> object)
{
	if (ImGui::TreeNode(IMGUI_JP("その他")))
	{
		DirectX::XMFLOAT4 color = object->GetColor();
		DirectX::XMFLOAT3 scale = object->GetScale();
		DirectX::XMFLOAT3 rot = object->GetRotation();
		DirectX::XMFLOAT2 pivot = object->GetPivot();

		ImGui::Text(IMGUI_JP("カラー"));
		bool colorslider = ImGui::ColorEdit4("##Color", &color.x);
		ImGui::Text(IMGUI_JP("スケール"));
		bool scaledrag = ImGui::DragFloat3("##ScaleDrag", &scale.x, m_DragValue);
		ImGui::Text(IMGUI_JP("回転軸"));
		bool Pivotdrag = ImGui::DragFloat2("##PivotDrag", &pivot.x, m_DragValue);
		ImGui::Text(IMGUI_JP("回転"));
		bool rotdrag = ImGui::DragFloat3("##RotDrag", &rot.x, m_DragValue);

		// 変更があった場合保存する.
		if (scaledrag
			|| Pivotdrag
			|| rotdrag
			|| colorslider)
		{
			object->SetColor(DirectX::XMFLOAT4(color.x, color.y, color.z,color.w));
			object->SetAlpha(color.w);
			object->SetScale(scale);
			object->SetPivot(pivot);
			object->SetRotation(rot);
			m_MoveAny = true;
		}
		ImGui::TreePop();
	}
}

//-----------------------------------------------------------------------.

void UIEditor::TriggeHgihLight()
{
	m_HighlightTime = 30.f * Time::GetInstance().GetDeltaTime();
}

//-----------------------------------------------------------------------.

void UIEditor::HighLightUI(std::shared_ptr<UIObject> object)
{
	if (m_HighlightTime <= 0.0f) return;

	DirectX::XMFLOAT3 pos = object->GetPosition();
	DirectX::XMFLOAT2 size = object->GetDrawSize();
	DirectX::XMFLOAT3 scale = object->GetScale();

	POINT clientLT = { 0, 0 };
	ClientToScreen(ResourceManager::GethWnd(), &clientLT);

	ImVec2 iwp = ImVec2(
		clientLT.x + pos.x + WND_W / 2 - size.x / 2,
		clientLT.y + pos.y + WND_H / 2 - size.y / 2);
	ImVec2 iws = ImVec2(size.x, size.y);

	ImGui::SetNextWindowPos(iwp);
	ImGui::SetNextWindowSize(iws);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoBackground;

	ImGui::Begin("##UIHighlightOverlay", nullptr, flags);

	ImDrawList* draw = ImGui::GetWindowDrawList();
	ImVec2 fillsize = ImVec2(iwp.x + size.x, iwp.y + size.y);

	draw->AddRectFilled(
		iwp,
		fillsize,
		IM_COL32(255, 0, 0, 100)
	);

	ImGui::End();

	m_HighlightTime = m_HighlightTime - Time::GetInstance().GetDeltaTime();
}