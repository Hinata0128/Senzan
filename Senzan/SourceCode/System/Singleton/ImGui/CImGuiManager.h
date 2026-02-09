#pragma once

#include "../../Data/ImGui/Library/imgui_impl_dx11.h"
#include "../../Data/ImGui/Library/imgui_impl_win32.h"

#include <string>
#include <vector>

/*****************************************************
*	ImGuiマネージャークラス.
* 　(シングルトンで作成).
**/

class CImGuiManager final
{
public:
	//インスタンスを取得.
	static CImGuiManager* GetInstance();

	//初期化.
	//引数にhWndを入れている理由(予想)
	//ウィンドウだけでしか使わないのでゲーム内では使用しないためだとおもう.
	static HRESULT Init(HWND hWnd);

	//リリース関数.
	static void Relese();

	//フレームの設定.
	static void NewFrameSetting();

	//Render関数は準備する関数です.
	//描画命令を準備する関数.
	//ここが、Draw()関数ではない理由は、
	//Draw関数は、すぐに描画するイメージ関数です.
	static void Render();

	//入力ボックスの表示.
	template<typename T>
	static bool Input(
		const char* label,				//ラベル文字(ImGuiでの表示名など)
		T& value,						//ユーザーが入力する値への参照(型はT)
		bool isLabel = true,			//ラベル表示するかどうか(デフォルトtrue)
		float step = 0.f,				//ステップ量(例: 数値の増減ボタン用)
		float steoFast = 0.f,			//ステップ量(高速バージョン)※typoかも？
		const char* format = "% .3f",	//表示フォーマット
		ImGuiInputTextFlags flags = 0);	//ImGuiの入力オプション.

	//スライダーの表示.
	//ImGuiで値をスライダーで調整できるようにする.
	template<typename T>
	static void Slider(
		const char* label,		//ラベル名(UIに表示される名前(例: "Speed"))
		T& value,				//調整対象の値(参照渡し、例: speed)
		T valueMin,				//スライダーの最小値.
		T valieMax,				//スライダーの最大値(最小値は通常0と仮定される)
		bool isLabel = true);	//ラベルをUIに表示するか(trueなら表示)

	//コンボボックス.
	static std::string Combo(
		const char* label,						//ラベル名(UIに表示される名前).
		std::string& NowItem,					//現在選ばれている項目の名前(選択状態の保持に使う).
		const std::vector<std::string>& List,	//選択肢の一覧(表示するリスト)
		bool isLabel = false,					//ラベルをコンボボックスの横に表示するか？true: 表示, false: 非表示.
		float space = 100.f);					//コンボボックスの幅(横幅をピクセルで指定).

	//チェックボックスの表示.
	//bool&は、呼び出し元の変数そのものを操作する.
	static bool CheckBox(
		const char* Label,		//チェックボックスの横に表示されるラベル(名前).
		bool& flag,				//チェック状態を管理する変数(参照渡しで、ON/OFF状態を直接変更する).
		bool isLabel = true);	//ラベルを表示、false: ラベル非表示(省略時はtrue).

	//グラフを表示.
	static void Graph(
		const char* Label,							//グラフのタイトル(ラベル)
		std::vector<float>& Data,					//グラフに表示する数値データの配列(折れ線グラフや棒グラフの元).
		const ImVec2& Size = ImVec2(300.f, 300.f));	//グラフのサイズ(横幅300、高さ300)→省略可能.

private:
	CImGuiManager();
	~CImGuiManager();

	//生成やコピーを削除.
	CImGuiManager(const CImGuiManager& rhs) = delete;
	CImGuiManager& operator = (const CImGuiManager& rhs) = delete;

};