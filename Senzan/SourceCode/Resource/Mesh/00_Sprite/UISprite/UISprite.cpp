#include "UISprite.h"
#include <fstream>
#include "Graphic/DirectX/DirectX11/DirectX11.h"
#include "Graphic/Shader/ShaderCompile.h"
#include "Graphic/Shader/Base/VertexShaderBase.h"
#include "Graphic/Shader/Base/PixelShaderBase.h"
#include "System/Utility/Assert/Assert.inl"

namespace
{
    constexpr TCHAR VS_FILE_PATH[] = _T("Data\\Shader\\Sprite2D\\Sprite2DVS.hlsl");
    constexpr TCHAR PS_FILE_PATH[] = _T("Data\\Shader\\Sprite2D\\Sprite2DPS.hlsl");
}

Sprite2D::Sprite2D()
    : m_pRectTransform      ( std::make_unique<RectTransform>() )
    , m_pVertexShader       ( std::make_unique<VertexShaderBase>() )
    , m_pPixelShaderBase    ( std::make_unique<PixelShaderBase>() )
    , m_pCashVertexBuffers  ()
    , m_pConstantBuffer     ( nullptr )
    , m_pTexture            ( nullptr )
    , m_pSampleLinear       ( nullptr )
    , m_ResourceName        ()
    , m_WorldMatrix         ()
    , m_DrawSize            ()
    , m_Color               ( {1.0f, 1.0f ,1.0f , 1.0f} )
{
}

//-----------------------------------------------------------------------------------------------------------------

Sprite2D::~Sprite2D()
{
    SAFE_RELEASE(m_pSampleLinear);
    SAFE_RELEASE(m_pTexture);

    for (auto& vertexBuffer : m_pCashVertexBuffers) {
        SAFE_RELEASE(vertexBuffer.second);
    }

    //SAFE_RELEASE(m_pVertexBuffer);
    SAFE_RELEASE(m_pConstantBuffer);
}

//-----------------------------------------------------------------------------------------------------------------

bool Sprite2D::Initialize(const std::filesystem::path& filePath)
{
    // ファイル名を資源名として保存.
    m_ResourceName = filePath.stem().string();

    // 画像サイズの読み込み.
    LoadImageSize(filePath);

    // シェーダ作成.
    CreateShader();

    // 板ポリ作成.
    CreateModel();

    // テクスチャ作成.
    CreateTexture(filePath);

    // サンプラ作成.
    CreateSampler();

    return true;
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::Render()
{
    // ワールド座標を計算.
    CalcWorldMatrix();

    // 使用するシェーダーを設定.    
    m_pVertexShader->SetVertexShader();
    m_pPixelShaderBase->SetPixelShader();


    //シェーダのコンスタントバッファに各種データを渡す.
    D3D11_MAPPED_SUBRESOURCE pData;
    SHADER_CONSTANT_BUFFER cb;

    //バッファ内のデータの書き換え開始時にmap.
    if (SUCCEEDED(
        DirectX11::GetInstance().GetContext()->Map(m_pConstantBuffer,
        0, D3D11_MAP_WRITE_DISCARD, 0, &pData)))
    {
        //ワールド行列を渡す.
        DirectX::XMMATRIX m = m_WorldMatrix;

        // 【D3DXからXMへ】行列の転置をXMMatrixTransposeに置き換え
        m = DirectX::XMMatrixTranspose(m);
        cb.mWorld = m;
        
        //カラー.
        cb.vColor = m_Color;

        //テクスチャ座標.
        //１マスあたりの割合にパターン番号（マス目）をかけて座標を設定する.
        cb.vUV.x = 0.0f;
        cb.vUV.y = 0.0f;

        //ビューポートの幅、高さを渡す.
        cb.fViewPortWidth = static_cast<float>(WND_W);
        cb.fViewPortHeight = static_cast<float>(WND_H);

        // 描画幅高さを渡す.
        cb.DawSize = m_DrawSize;

        memcpy_s(pData.pData, pData.RowPitch,
            (void*)(&cb), sizeof(cb));

        DirectX11::GetInstance().GetContext()->Unmap(m_pConstantBuffer, 0);
    }

    //このコンスタントバッファをどのシェーダで使うか？.
    DirectX11::GetInstance().GetContext()->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);
    DirectX11::GetInstance().GetContext()->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

    //頂点バッファをセット.
    UINT stride = sizeof(VERTEX);	//データの間隔.
    UINT offset = 0;
    ID3D11Buffer* pVertexBuffer = GetUseVertexBuffer();
    DirectX11::GetInstance().GetContext()->IASetVertexBuffers(0, 1,
        &pVertexBuffer, &stride, &offset);

    //頂点インプットレイアウトをセット.
    m_pVertexShader->SetInputLayout();

    //プリミティブ・トポロジーをセット.
    DirectX11::GetInstance().GetContext()->IASetPrimitiveTopology(
        D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    //テクスチャをシェーダに渡す.
    DirectX11::GetInstance().GetContext()->PSSetSamplers(0, 1, &m_pSampleLinear);
    DirectX11::GetInstance().GetContext()->PSSetShaderResources(0, 1, &m_pTexture);

    //アルファブレンド有効にする.
    DirectX11::GetInstance().SetAlphaBlend(true);

    //プリミティブをレンダリング.
    DirectX11::GetInstance().GetContext()->Draw(4, 0);

    //アルファブレンド無効にする.
    DirectX11::GetInstance().SetAlphaBlend(false);
}
//-----------------------------------------------------------------------------------------------------------------

const std::unique_ptr<RectTransform>& Sprite2D::GetRectTransform() const
{
    return m_pRectTransform;
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::SetDrawSize(const DirectX::XMFLOAT2& drawSize)
{
    m_DrawSize = drawSize;
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::SetColor(const DirectX::XMFLOAT4& color)
{
    m_Color = color;
}

//-----------------------------------------------------------------------------------------------------------------

const std::string& Sprite2D::GetResourceName() const
{
    return m_ResourceName;
}


//-----------------------------------------------------------------------------------------------------------------
void Sprite2D::SetResourceName(const std::string& name)
{
    m_ResourceName = name;
}


//-----------------------------------------------------------------------------------------------------------------

ID3D11ShaderResourceView* Sprite2D::GetTexture()
{
    return m_pTexture;
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::LoadImageSize(const std::filesystem::path& filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) 
    { 
        assert(false && "画像ファイルが開けませんでした。");
    }

    file.seekg(16);

    std::array<uint32_t, 2> size;
    file.read(reinterpret_cast<char*>(size.data()), sizeof(size));

    constexpr auto swapEndia = [](uint32_t val) -> uint32_t
        {
            return ((val >> 24) & 0x000000FF)
                | ((val >> 8) & 0x0000FF00)
                | ((val << 8) & 0x00FF0000)
                | ((val << 24) & 0xFF000000);
        };

    const float w = static_cast<float>(swapEndia(size[0]));
    const float h = static_cast<float>(swapEndia(size[1]));

    m_pRectTransform->SetSize({ w, h });
    m_DrawSize = { w, h };
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::CreateShader()
{
    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    // VSの作成.
    ShaderCompile(new std::string(VS_FILE_PATH), "main", "vs_5_0", pBlob, pErrorBlob);
    std::vector<D3D11_INPUT_ELEMENT_DESC> layout = 
    {
        {"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    m_pVertexShader->ConfigInputLayout(layout);
    m_pVertexShader->Init(pBlob);

    // PSの作成.
    ShaderCompile(new std::string(PS_FILE_PATH), "main", "ps_5_0", pBlob, pErrorBlob);
    m_pPixelShaderBase->Init(pBlob);    

    //コンスタント（定数）バッファ作成.
	//シェーダに特定の数値を送るバッファ.
	D3D11_BUFFER_DESC desc;
	desc.BindFlags            = D3D11_BIND_CONSTANT_BUFFER;		// コンスタントバッファを指定.
	desc.ByteWidth            = sizeof(SHADER_CONSTANT_BUFFER);	// コンスタントバッファのサイズ.
	desc.CPUAccessFlags       = D3D11_CPU_ACCESS_WRITE;		    // 書き込みでアクセス.
	desc.MiscFlags            = 0;	                            // その他のフラグ（未使用）.
	desc.StructureByteStride  = 0;	                            // 構造体のサイズ（未使用）.
	desc.Usage                = D3D11_USAGE_DYNAMIC;	        // 使用方法：直接書き込み.

    auto hr = DirectX11::GetInstance().GetDevice()->CreateBuffer(
        &desc, nullptr, &m_pConstantBuffer);
    assert(SUCCEEDED(hr) && "コンスタントバッファの作成失敗");
}

//-----------------------------------------------------------------------------------------------------------------

const HRESULT Sprite2D::CreateModel()
{        
    // 画像の幅高さを取得.
    float width     = m_pRectTransform->GetSize().x;   
    float height    = m_pRectTransform->GetSize().y;

    // ピボットを取得.
    DirectX::XMFLOAT2 pivot = m_pRectTransform->GetPivot();

    // 原点をPivotの位置に基づいて調整.
    float originX = -width * pivot.x;
    float originY = -height * pivot.y;

    // 頂点のUV座標 (0.0f ~ 1.0f固定).
    float u = 1.0f;
    float v = 1.0f;

    VERTEX vertices[] =
    {
        // 頂点座標(x, y, z).                                        UV座標(u, v).
        { DirectX::XMFLOAT3(originX,             originY + height, 0.0f), DirectX::XMFLOAT2(0.0f, v) },     // 左下.
        { DirectX::XMFLOAT3(originX,             originY,          0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },  // 左上.
        { DirectX::XMFLOAT3(originX + width,     originY + height, 0.0f), DirectX::XMFLOAT2(u,    v) },     // 右下.
        { DirectX::XMFLOAT3(originX + width,     originY,          0.0f), DirectX::XMFLOAT2(u,    0.0f) }   // 右上.
    };

    UINT verMax = sizeof(vertices) / sizeof(vertices[0]);

    //バッファ構造体.
	D3D11_BUFFER_DESC bd;
	bd.Usage			    = D3D11_USAGE_DEFAULT;		//使用方法（デフォルト）.
	bd.ByteWidth		    = sizeof(VERTEX) * verMax;	//頂点のサイズ.
	bd.BindFlags		    = D3D11_BIND_VERTEX_BUFFER;	//頂点バッファとして扱う.
	bd.CPUAccessFlags	    = 0;	//CPUからはアクセスしない.
	bd.MiscFlags		    = 0;	//その他のフラグ（未使用）.
	bd.StructureByteStride  = 0;	//構造体のサイズ（未使用）.

    //サブリソースデータ構造体.
    D3D11_SUBRESOURCE_DATA InitData;
    InitData.pSysMem = vertices;

    // 頂点バッファの作成.
    auto hr = DirectX11::GetInstance().GetDevice()->CreateBuffer(
        &bd, &InitData, &m_pCashVertexBuffers[pivot]);

    assert(SUCCEEDED(hr) && "頂点バッファの作成失敗");        

    return hr;
}

//-----------------------------------------------------------------------------------------------------------------

const HRESULT Sprite2D::CreateTexture(const std::wstring& filePath)
{
    auto hr = D3DX11CreateShaderResourceViewFromFileW(
        DirectX11::GetInstance().GetDevice(),
        filePath.c_str(),
        nullptr,
        nullptr,
        &m_pTexture,
        nullptr);
    assert(SUCCEEDED(hr) && "テクスチャの作成失敗");

    return hr;
}

//-----------------------------------------------------------------------------------------------------------------

const HRESULT Sprite2D::CreateSampler()
{
    D3D11_SAMPLER_DESC desc = {};
    desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;//リニアフィルタ（線形補間）.

    //POINT:高速だが粗い.
    //ラッピングモード.
    //MIRROR: 反転繰り返し.
    //CLAMP : 端の模様を引き伸ばす.
    //BORDER: 別途境界色を決める.
    desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

    auto hr = DirectX11::GetInstance().GetDevice()->CreateSamplerState(
        &desc, &m_pSampleLinear);
    assert(SUCCEEDED(hr) && "サンプラの作成失敗");

    return hr;
}

//-----------------------------------------------------------------------------------------------------------------

void Sprite2D::CalcWorldMatrix()
{
    const Transform& transform = m_pRectTransform->GetTransform();

    DirectX::XMMATRIX	mTrans, mRot, mScale;

    // 拡縮行列を計算.
    // 【D3DXからXMへ】D3DXMatrixScaling -> XMMatrixScaling
    mScale = DirectX::XMMatrixScaling(
        transform.Scale.x,
        transform.Scale.y,
        transform.Scale.z);

    // 回転行列を計算.
    DirectX::XMMATRIX mYaw, mPitch, mRoll;
    // 【D3DXからXMへ】D3DXMatrixRotation* -> XMMatrixRotation*
    mYaw = DirectX::XMMatrixRotationY(transform.Rotation.y);
    mPitch = DirectX::XMMatrixRotationX(transform.Rotation.x);
    mRoll = DirectX::XMMatrixRotationZ(transform.Rotation.z);

    // 【D3DXからXMへ】行列乗算をXMMatrixMultiplyに置き換え
    mRot = DirectX::XMMatrixMultiply(mYaw, mPitch);
    mRot = DirectX::XMMatrixMultiply(mRot, mRoll);
    // mRot = mYaw * mPitch * mRoll; (演算子オーバーロードに依存しない形式)


    // 平行行列を計算.
    DirectX::XMFLOAT3 anchoredPosition = m_pRectTransform->CalcAnchoredPosition();
    // 【D3DXからXMへ】D3DXMatrixTranslation -> XMMatrixTranslation
    mTrans = DirectX::XMMatrixTranslation(
        anchoredPosition.x,
        anchoredPosition.y,
        anchoredPosition.z);

    // 【D3DXからXMへ】ワールド行列の合成
    // m_WorldMatrix = mScale * mRot * mTrans; の代替
    DirectX::XMMATRIX mWorldTemp = DirectX::XMMatrixMultiply(mScale, mRot);
    m_WorldMatrix = DirectX::XMMatrixMultiply(mWorldTemp, mTrans);
}
//-----------------------------------------------------------------------------------------------------------------

ID3D11Buffer* Sprite2D::GetUseVertexBuffer()
{
    const DirectX::XMFLOAT2& pivot = m_pRectTransform->GetPivot();

    // キャッシュに無ければ作成.
    if (m_pCashVertexBuffers.find(pivot) == m_pCashVertexBuffers.end())
    {
        CreateModel();
    }

    return m_pCashVertexBuffers[pivot];
}
