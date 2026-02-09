#include "02_UIObject/UILoader/UILoader.h"
#include "FileManager/FileManager.h"
#include "ResourceManager/SpriteManager/SpriteManager.h"

// json型を作成
using Json = nlohmann::json;

//----------------------------------------------------------------.

void UILoader::LoadFromJson(
	const std::string& scenepath,
	std::vector<std::shared_ptr<UIObject>>& uis)
{
	// JSON読み込み.
	Json jsonData = FileManager::JsonLoad(scenepath);

	// 保存されたUIデータを読み込み、展開.
	for (auto& [imageName, spriteArray] : jsonData.items()) {
		// 拡張子が .json ならスキップ.
		std::string::size_type dotPos = imageName.find_last_of('.');
		if (dotPos != std::string::npos) {
			std::string ext = imageName.substr(dotPos);
			if (ext == ".json" || ext == ".JSON") continue;
		}

		// スプライト取得.
		std::shared_ptr<Sprite2D> pSprite = SpriteManager::GetSprite2D(GetBaseName(imageName));

		if (!pSprite) {
			MessageBoxA(NULL, ("スプライトが見つかりません: " + imageName).c_str(), "Error", MB_OK);
			continue;
		}

		// 各UIインスタンスを展開.
		for (auto& value : spriteArray) {
			std::shared_ptr<UIObject> ui = std::make_shared<UIObject>();

			ui->AttachSprite(pSprite);
			ui->SetUIName(imageName);

			// 各種情報を設定.
			ui->SetPosition(DirectX::XMFLOAT3(value["Pos"]["x"], value["Pos"]["y"], value["Pos"]["z"]));
			ui->SetColor(DirectX::XMFLOAT4(value["Color"]["x"], value["Color"]["y"], value["Color"]["z"], value["Color"]["a"]));
			ui->SetAlpha(value["Alpha"]);
			ui->SetScale(DirectX::XMFLOAT3(value["Scale"]["x"], value["Scale"]["y"], value["Scale"]["z"]));
			ui->SetPivot(DirectX::XMFLOAT2(value["Pivot"]["x"], value["Pivot"]["y"]));
			ui->SetRotation(DirectX::XMFLOAT3(value["Rotate"]["x"], value["Rotate"]["y"], value["Rotate"]["z"]));

			// SpriteDataの一部も上書き.
			pSprite->SetDrawSize(DirectX::XMFLOAT2(value["Disp"]["w"], value["Disp"]["h"]));

			// リストに追加.
			uis.push_back(ui);
		}
	}

	// Z座標を基準にソート.
	std::sort(uis.begin(), uis.end(),
		[](const std::shared_ptr<UIObject>& a,
			const std::shared_ptr<UIObject>& b)
		{
			if (!a || !b) return false;
			return a->GetPosition().z < b->GetPosition().z;
		});
}