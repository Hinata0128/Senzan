/***************************************************************************************************
*	SkinMeshCode Version 2.40
*	LastUpdate	: 2024/06/14.
**/
#pragma once
//警告についてのコード分析を無効にする。4005：再定義.
#pragma warning( disable : 4005 )

#include "SkinMeshParser.h"
#include "../MeshBase.h"

//前方宣言.
class DirectX9;
class DirectX11;
class CameraBase;		// カメラクラス(描画に必要な情報を持ってくる).

/**************************************************
*	スキンメッシュクラス
**/
class SkinMesh final
	: public MeshBase
{
public:

	//ボーン単位.
	struct CBUFFER_PER_BONES
	{
		// 【修正】D3DXMATRIX を XMMATRIX に変更
		DirectX::XMMATRIX mBone[D3DXPARSER::MAX_BONES];

		CBUFFER_PER_BONES()
		{
			// 【修正】XMMatrixIdentity で初期化
			DirectX::XMMATRIX Identity = DirectX::XMMatrixIdentity();
			for (int i = 0; i < D3DXPARSER::MAX_BONES; i++)
			{
				// XMMATRIX は通常、初期化に XMStoreFloat4x4 などを使用するが、
				// C++の構造体初期化として XMMATRIX を直接代入
				mBone[i] = Identity;
			}
		}
	};

	// 頂点構造体. (変更なし)
	struct SKIN_VERTEX
		: public VERTEX
	{
		UINT BoneIndex[4];		//ボーン 番号.
		float BoneWeight[4];	//ボーン 重み.		
	};
public:
	SkinMesh();	//コンストラクタ.
	~SkinMesh();	//デストラクタ.

	// 初期化処理. (変更なし)
	HRESULT Init(std::string FileName, const std::string& Name)override;

	//解放関数. (変更なし)
	HRESULT Release();

	//描画関数. (変更なし)
	void Render(const LPD3DXANIMATIONCONTROLLER pAC);
	// 深度描画用. (変更なし)
	void RenderDepth(const LPD3DXANIMATIONCONTROLLER pAC);

	// ... アニメーション速度・時間関連の関数 (変更なし) ...
	double GetAnimSpeed() { return m_AnimSpeed; }
	void SetAnimSpeed(double Speed) { m_AnimSpeed = Speed; }
	double GetAnimTime() { return m_AnimTime; }
	void SetAnimTime(double Time) { m_AnimTime = Time; }
	LPD3DXANIMATIONCONTROLLER GetAnimationController() { return m_pD3dxMesh->m_pAnimController; }
	void ChangeAnimSet(int index, LPD3DXANIMATIONCONTROLLER pAC);
	void ChangeAnimSet_StartPos(int Index, double StartFramePos, LPD3DXANIMATIONCONTROLLER pAC);
	double GetAnimPeriod(int Index);
	int GetAnimMax(LPD3DXANIMATIONCONTROLLER pAC = nullptr);

	//指定したボーン情報(座標・行列)を取得する関数.
	// 【修正】D3DXMATRIX を XMMATRIX に変更
	bool GetMatrixFromBone(LPCSTR BoneName, DirectX::XMMATRIX* pOutMat);
	// 【修正】D3DXVECTOR3 を XMFLOAT3 に変更 (XMVECTOR のポインタ渡しは避けるため)
	bool GetPosFromBone(LPCSTR BoneName, DirectX::XMFLOAT3* pOutPos);
	// 【修正】D3DXVECTOR3 を XMFLOAT3 に変更
	bool GetDeviaPosFromBone(LPCSTR BoneName, DirectX::XMFLOAT3* pOutPos, DirectX::XMFLOAT3 SpecifiedPos = { 0.0f, 0.0f, 0.0f });
	
	LPD3DXFRAME GetFrameByName(LPCSTR frameName);

    // グローバルなアルファ値を設定（レンダリング時にマテリアルのアルファに乗算される）
    void SetGlobalAlpha(float alpha) { m_GlobalAlpha = alpha; }
    float GetGlobalAlpha() const { return m_GlobalAlpha; }

    // ディゾルブエフェクト制御
    void SetDissolveEnabled(bool enable) { m_DissolveEnabled = enable; }
    bool IsDissolveEnabled() const { return m_DissolveEnabled; }
    void SetDissolveThreshold(float threshold) { m_DissolveThreshold = (threshold < 0.0f) ? 0.0f : ((threshold > 1.0f) ? 1.0f : threshold); }
    float GetDissolveThreshold() const { return m_DissolveThreshold; }
    void SetDissolveEdgeWidth(float width) { m_DissolveEdgeWidth = (width < 0.0f) ? 0.0f : ((width > 0.5f) ? 0.5f : width); }
    float GetDissolveEdgeWidth() const { return m_DissolveEdgeWidth; }


private:
	LPD3DXFRAME FindFrame(LPD3DXFRAME frame, LPCSTR frameName);
	// ... ロード・作成関連の private 関数 (入出力の型は D3DX のまま維持) ...
	HRESULT LoadXMesh(std::string FileName);
	HRESULT CreateShader();
	HRESULT CreateIndexBuffer(DWORD Size, int* pIndex, ID3D11Buffer** ppIndexBuffer);
	HRESULT CreateAppMeshFromD3DXMesh(LPD3DXFRAME pFrame);
	HRESULT CreateCBuffer(ID3D11Buffer** pConstantBuffer, UINT Size);
	HRESULT CreateSampler(ID3D11SamplerState** pSampler);
	void BuildAllMesh(D3DXFRAME* pFrame);
	HRESULT ReadSkinInfo(MYMESHCONTAINER* pContainer, SKIN_VERTEX* pVB, SKIN_PARTS_MESH* pParts);

	// 【修正】D3DXMATRIX を XMMATRIX に変更
	DirectX::XMMATRIX GetCurrentPoseMatrix(SKIN_PARTS_MESH* pParts, int Index);

	void SetNewPoseMatrices(SKIN_PARTS_MESH* pParts, int Frame, MYMESHCONTAINER* pContainer);
	void DrawFrame(LPD3DXFRAME pFrame);
	void DrawFrameDepth(LPD3DXFRAME pFrame);

	void DrawPartsMesh(SKIN_PARTS_MESH* pMesh, DirectX::XMMATRIX World, MYMESHCONTAINER* pContainer);
	void DrawPartsMeshDepth(SKIN_PARTS_MESH* pMesh, DirectX::XMMATRIX World, MYMESHCONTAINER* pContainer);

	void SendCBufferPerBone(SKIN_PARTS_MESH* pMesh);
	void SendCBufferPerFrame();
	void SendCBufferPerMesh();
	void SendCBufferPerMeshDepth();

	void SendCBufferPerMaterial(MY_SKINMATERIAL* pMaterial);
	void SendTexture(MY_SKINMATERIAL* pMaterial);
	void DestroyAllMesh(D3DXFRAME* pFrame);
	HRESULT DestroyAppMeshFromD3DXMesh(LPD3DXFRAME p);
	void ConvertCharaMultiByteToUnicode(WCHAR* Dest, size_t DestArraySize, const CHAR* str);
private:
	ID3D11Buffer* m_pCBufferPerBone;		//コンスタントバッファ(ボーン毎).

	DirectX::XMMATRIX	m_mView;
	DirectX::XMMATRIX	m_mProj;
	DirectX::XMFLOAT3	m_CamPos;

	// アニメーション速度.
	double m_AnimSpeed;
	double m_AnimTime;

	D3DXPARSER* m_pD3dxMesh;	// メッシュ. (変更なし)
	TCHAR		m_FilePath[256];	// Xファイルのパス. (変更なし)
	int			m_Frame;			// アニメーションフレーム. (変更なし)
	// グローバルアルファ値（1.0 = 不透明）
	float m_GlobalAlpha;
	// ディゾルブエフェクト
	bool  m_DissolveEnabled = false;     // ディゾルブ有効フラグ
	float m_DissolveThreshold = 0.0f;    // ディゾルブ閾値 (0.0~1.0)
	float m_DissolveEdgeWidth = 0.05f;   // エッジ発光幅
    // (no scene-wide alpha)
};

