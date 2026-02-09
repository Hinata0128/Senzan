#include "CImGuiManager.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include <utility>

//使用しているフォント.
constexpr char FONT_FILE_PATH[] = "Data//ImGui//Font//NotoSansJP-SemiBold.ttf";
//フォントサイズ.
constexpr float FONT_SIZE = 18.f;
// グラフやUI要素などにおいて、同じライン上で少しずらして表示するためのオフセット値（100ピクセル）
// constexpr を使うことで、コンパイル時に定数として扱われ、最適化される
constexpr float SAMALINE_OFFSET = 100.f;    //ImGui::SameLineのオフセット値.

CImGuiManager::CImGuiManager()
{

}

CImGuiManager::~CImGuiManager()
{

}

//インスタンス時にこれを先に書く.
CImGuiManager* CImGuiManager::GetInstance()
{
    static CImGuiManager Instance;
    return &Instance;
}

HRESULT CImGuiManager::Init(HWND hWnd)
{
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   //ドッキングを有効にする.
    //マルチビューポート(複数のウィンドウ)とプラットフォームウィンドウの有効化.
    //くわしいやく
    //ImGuiのマルチビューポート機能を有効化(UIを複数ウィンドウとして表示可能にする).
    // もう少しくわしく書く
    //複数ウィンドウ表示を有効化.
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; //EnableMulti-Viewport / Platform Windows
    //前のフレームから経過時間を設定(秒単位)。ここでは、1フレーム = 1/60秒 (=60FPSを想定).
    io.DeltaTime = 1.f / 60.f;    //time elapsed since the previous frame (in seconds).
    io.DisplaySize.x = WND_W;   //横の画像サイズ.
    io.DisplaySize.y = WND_H;   //縦の画像サイズ.

    //フォントを読み込む.
    // io.Fonts->AddFontFromFileTTF 関数を使って、TrueTypeフォント（TTF）ファイルから日本語対応のフォントを追加する。
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        FONT_FILE_PATH,                         //読み込むフォントファイルのパス(例: "fonts/MSGothic.ttc" など)
        FONT_SIZE,                              //フォントサイズ(ピクセル単位)
        NULL,                                   //フォント設定(NULLでデフォルト設定を使用).
        io.Fonts->GetGlyphRangesJapanese());    //日本語の文字セット(ひらがな・カタカナ・漢字)を含むグリフ範囲を取得して設定/

    //Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //UIの文字を黒色にする.
    ImGui::StyleColorsDark();
    //スタイル設定にアクセスするための参照を取得.
    ImGuiStyle& style = ImGui::GetStyle();
    //マルチビューポート(Platform Windows)が有効な場合に追加スタイルを設定.
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.f;                 //ウィンドウの角の丸みをゼロに(四角く)する.
        style.Colors[ImGuiCol_WindowBg].w = 1.f;    //ウィンドウ背景の透明度を1(完全に不透明)にする.
    }
    
    if (ImGui_ImplWin32_Init(hWnd) == false) { return E_FAIL; };
    if (ImGui_ImplDX11_Init(DirectX11::GetInstance().GetDevice(), DirectX11::GetInstance().GetContext()) == false) { return E_FAIL; }

    return S_OK;
}

void CImGuiManager::Relese()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

//フレームの設定.
void CImGuiManager::NewFrameSetting()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

//描画.
void CImGuiManager::Render()
{
    //ImGuiIO型のioをvoidとして使用？
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::Render();

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

//入力ボックス表示.
template<typename T>
bool CImGuiManager::Input(const char* label,
    T& value,
    bool isLabel,
    float step,
    float stepFast,
    const char* format,
    ImGuiInputTextFlags flags)
{
    //ラベルの文字列を作成.
    const std::string newlabel = "##" + std::string(label);

    //ラベル設定が有効ならラベルを表示.
    if (isLabel == true)
    {
        //ラベル(テキスト)を表示する.
        ImGui::Text(label);
        //ラベルとスライダーを同じ行に配置するために横並びにする.
        //SAMALINE_OFFSETだけ右にずらして表示.
        ImGui::SameLine(SAMALINE_OFFSET);
    }

    // int型で表示.
    if constexpr (std::is_same_v<T, int>) 
    {
        return ImGui::InputInt(newlabel.c_str(),
            &value,
            static_cast<int>(step),
            static_cast<int>(stepFast),
            flags);
    }
    // float型で表示.
    else if constexpr (std::is_same_v<T, float>) 
    {
        return ImGui::InputFloat(newlabel.c_str(),
            &value,
            step,
            stepFast,
            format,
            flags);
    }
    // double型で表示.
    else if constexpr (std::is_same_v<T, double>) 
    {
        return ImGui::InputDouble(newlabel.c_str(),
            &value,
            static_cast<double>(step),
            static_cast<double>(stepFast),
            format,
            flags);
    }
    // D3DXVECTOR2型で表示.
    else if constexpr (std::is_same_v<T, D3DXVECTOR2>) 
    {
        //D3DXVECTOR2型を使用するときのInput関数.
        return ImGui::InputFloat2(newlabel.c_str(),
            &value.x,
            format,
            flags);
    }
    // D3DXVECTOR3型で表示.
    else if constexpr (std::is_same_v<T, D3DXVECTOR3>) 
    {
        //D3DXVECTOR3型を使用するときのInput関数.
        return ImGui::InputFloat3(newlabel.c_str(),
            &value.x,
            format,
            flags);
    }
    // string型で表示.
    else if constexpr (std::is_same_v<T, std::string>) 
    {
        char buffer[256];
        strncpy_s(buffer,
            value.c_str(),
            sizeof(buffer));
        //std::stringを使用するときのInput関数.
        if (ImGui::InputText(newlabel.c_str(),
            buffer,
            sizeof(buffer)))
        {
            value = buffer;
            return true;
        }
    }
    return false;
}
template bool CImGuiManager::Input<int>(const char*,
    int&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);
template bool CImGuiManager::Input<float>(const char*,
    float&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);
template bool CImGuiManager::Input<double>(const char*,
    double&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);
template bool CImGuiManager::Input<D3DXVECTOR2>(const char*,
    D3DXVECTOR2&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);
template bool CImGuiManager::Input<D3DXVECTOR3>(const char*,
    D3DXVECTOR3&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);
template bool CImGuiManager::Input<std::string>(const char*,
    std::string&,
    bool,
    float,
    float,
    const char*,
    ImGuiInputTextFlags);

//スライダーの表示.
template<typename T>
void CImGuiManager::Slider(const char* label, T& value, T valueMin, T valueMax, bool isLabel)
{
    //ラベルの文字列を作成.
    const std::string newlabel = "##" + std::string(label);

    //ラベル設定が有効ならラベルを表示.
    if (isLabel == true)
    {
        //ラベル(テキスト)を表示する.
        ImGui::Text(label);
        //ラベルとスライダーを同じ行に配置するために横並びにする.
        //SAMALINE_OFFSETだけ右にずらして表示.
        ImGui::SameLine(SAMALINE_OFFSET);
    }

    //int型で表示.
    if constexpr (std::is_same_v<T, int>)
    {
        ImGui::SliderInt(
            newlabel.c_str(),
            &value,
            valueMin,
            valueMax);
    }
    //float型で表示
    else if constexpr (std::is_same_v<T, float>)
    {
        ImGui::SliderFloat(
            newlabel.c_str(),
            &value,
            valueMin,
            valueMax);
    }
}
template void CImGuiManager::Slider<int>(
    const char*,
    int&,
    int,
    int, 
    bool);
template void CImGuiManager::Slider<float>(const char*,
    float&,
    float, 
    float,
    bool);

//コンボボックス.
std::string CImGuiManager::Combo(const char* Label,
    std::string& NowItem,
    const std::vector<std::string>& List,
    bool isLabel,
    float space)
{
    //選択中のリストの要素番号.
    int No = 0;

    //リストの最大数を取得.
    const int List_Max = static_cast<int>(List.size());

    //現在選択されているListの要素番号を保存.
    for (int i = 0; i < List_Max; ++i)
    {
        if (List[i] == NowItem)
        {
            No = i;
            break;
        }
    }

    //ラベル設定が有効ならテキストを表示.
    if (isLabel == true)
    {
        ImGui::Text(Label);
        ImGui::SameLine(space);
    }

    //コンボボックスの表示.
    if (ImGui::BeginCombo(std::string("##" + std::string(Label)).c_str(),
        NowItem.c_str()))
    {
        const int Size = static_cast<int>(List.size());
        for (int i = 0; i < Size; ++i)
        {
            //選択状態を取得.
            bool is_Selected = (NowItem == List[i]);

            //選択中のリストの要素番号を更新する.
            if (ImGui::Selectable(List[i].c_str(),
                is_Selected))
            {
                No = i;
            }

            //選択されたListにフォーカスを設定.
            if (is_Selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    return NowItem = List[No];
}

//チェックボックスの表示.
bool CImGuiManager::CheckBox(
    const char* label,
    bool& flag,
    bool isLabel)
{
    //ラベル設定が有効ならテキストを表示.
    if (isLabel == true)
    {
        //ラベル(テキスト)を表示する.
        ImGui::Text(label);
        //ラベルとスライダーを同じ行に配置するために横並びにする.
        //SAMALINE_OFFSETだけ右にずらして表示.
        ImGui::SameLine(SAMALINE_OFFSET);
    }
    return ImGui::Checkbox(
        std::string("##" + std::string(label)).c_str(),
        &flag);
}

//グラフを表示.
void CImGuiManager::Graph(
    const char* Label,
    std::vector<float>& Data,
    const ImVec2& Size)
{
    ImGui::PlotLines(std::string("##" + std::string(Label)).c_str(),
        Data.data(), static_cast<int>(Data.size()),
        0, nullptr,
        FLT_MAX,
        FLT_MAX,
        Size);
}