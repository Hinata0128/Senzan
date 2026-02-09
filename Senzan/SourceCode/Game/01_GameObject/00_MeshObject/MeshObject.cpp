#include "MeshObject.h"
#include "Resource/Mesh/MeshBase.h"
#include "Resource/Mesh/01_Static/StaticMesh.h"
#include "Resource/Mesh/02_Skin/SkinMesh.h"
#include "Graphic/DirectX/DirectX11/DirectX11.h"

MeshObject::MeshObject()
	: GameObject()
	, m_pMesh()
	, m_IsLight(false)
	, m_IsShadow(true)
	, m_pAnimCtrl(nullptr)
	, m_AnimNo(0)
	, m_AnimSpeed(0.0)
	, m_AnimTimer(0.0)
	, m_Isloop (false)
	, m_BonePos()
{
    m_ObjectAlpha = 1.0f;
}


// Set object alpha
void MeshObject::SetAlpha(float alpha)
{
    // clamp
    if (alpha < 0.0f) alpha = 0.0f;
    if (alpha > 1.0f) alpha = 1.0f;
    m_ObjectAlpha = alpha;
}

float MeshObject::GetAlpha() const
{
    return m_ObjectAlpha;
}

//------------------------------------------------------------------------------------.

MeshObject::~MeshObject()
{
}

//------------------------------------------------------------------------------------.

void MeshObject::Update()
{
	
}

void MeshObject::LateUpdate()
{
}

//------------------------------------------------------------------------------------.

void MeshObject::Draw()
{
	if (!m_IsRenderActive) { return; }

	// メッシュ未接続の場合、描画しない.
	if (m_pMesh.expired()) { return; }

	// 描画直前で必要な情報をMesh本体に送る.
	if (auto pMesh = m_pMesh.lock())
	{
		pMesh->SetPosition(m_spTransform->Position);
		pMesh->SetRotation(m_spTransform->Rotation);
		pMesh->SetScale(m_spTransform->Scale);
		pMesh->SetIsLight(m_IsLight);
		pMesh->SetIsShadow(m_IsShadow);
	}

	// StaticMesh時の描画.
	if (std::shared_ptr<StaticMesh> staticMesh = std::dynamic_pointer_cast<StaticMesh>(m_pMesh.lock()))
	{
		staticMesh->Render();
	}
	// SkinMesh時の描画.
	else if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
	{
		m_AnimTimer += m_AnimSpeed * GetDelta();
		IsLoopAnimTimeSet();
		m_pAnimCtrl->SetTrackPosition(0, m_AnimTimer);

		float objAlpha = m_ObjectAlpha;
		float prevAlpha = skinMesh->GetGlobalAlpha();
		float appliedAlpha = prevAlpha * objAlpha;

		bool useAlpha = (appliedAlpha < 0.999f);
		if (useAlpha) DirectX11::GetInstance().SetAlphaBlend(true);

		// Apply object alpha to mesh, render, then restore previous alpha
		skinMesh->SetGlobalAlpha(appliedAlpha);

		skinMesh->Render(m_pAnimCtrl);

		// Restore previous alpha
		skinMesh->SetGlobalAlpha(prevAlpha);

		if (useAlpha) DirectX11::GetInstance().SetAlphaBlend(false);

	}
}

//------------------------------------------------------------------------------------.

void MeshObject::DrawDepth()
{
	if (!m_IsRenderActive) { return; }

	// メッシュ未接続の場合、深度描画しない.
	if (m_pMesh.expired()) { return; }

	// 描画直前で必要な情報をMesh本体に送る.
	if (auto pMesh = m_pMesh.lock())
	{
		pMesh->SetPosition(m_spTransform->Position);
		pMesh->SetRotation(m_spTransform->Rotation);
		pMesh->SetScale(m_spTransform->Scale);
	}

	// StaticMesh時の深度描画.
	if (std::shared_ptr<StaticMesh> staticMesh = std::dynamic_pointer_cast<StaticMesh>(m_pMesh.lock()))
	{
		staticMesh->RenderDepth();
	}
	// SkinMesh時の深度描画.
	else if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
	{
		skinMesh->RenderDepth(m_pAnimCtrl);
	}

}

//------------------------------------------------------------------------------------.

void MeshObject::AttachMesh(const std::shared_ptr<MeshBase>& pMesh)
{
	if (!pMesh) { return; }

	m_pMesh = pMesh;

	// 接続メッシュがSkinならアニメーションコントローラーのクローンも作成する.
	if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(pMesh))
	{
		//アニメーションコントローラを取得.
		LPD3DXANIMATIONCONTROLLER pAC = skinMesh->GetAnimationController();

		//アニメーションコントローラのクローンを作成.
		if (FAILED(
			pAC->CloneAnimationController(
			pAC->GetMaxNumAnimationOutputs(),
			pAC->GetMaxNumAnimationSets(),
			pAC->GetMaxNumTracks(),
			pAC->GetMaxNumEvents(),
			&m_pAnimCtrl)))		//(out)クローン作成先.
		{
			_ASSERT_EXPR(false, L"アニメーションコントローラのクローン作成失敗");
		}
	}
}

//------------------------------------------------------------------------------------.

void MeshObject::DetachMesh()
{
	m_pMesh.reset();
}

//------------------------------------------------------------------------------------.

const std::string MeshObject::GetResourceName() const
{
	if (auto pMesh = m_pMesh.lock())
	{
		return pMesh->GetResourceName();
	}
	return "None";
}

//------------------------------------------------------------------------------------.

void MeshObject::SetIsLight(const bool& isLight)
{
	m_IsLight = isLight;
}

void MeshObject::SetIsShadow(const bool& isShadow)
{
	m_IsShadow = isShadow;
}

// アニメーションの速度を設定.
void MeshObject::SetAnimSpeed(const double speed)
{
	m_AnimSpeed = speed;
}

// アニメーションのループ設定.
void MeshObject::SetIsLoop(const bool isLoop)
{
	m_Isloop = isLoop;
}

// アニメーションループの設定.
void MeshObject::SetAnimTime(double StartTime)
{
	m_AnimTimer = StartTime;
}

// アニメーション終了時間を取得.
double MeshObject::GetAnimPeriod(int index) const
{
	if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
	{
		return skinMesh->GetAnimPeriod(index);
	}
	return 0.0;
}

void MeshObject::IsLoopAnimTimeSet()
{
    // もしループしないなら最終フレームに到達時点でアニメーション固定.
    if (!m_Isloop) {
        double EndTime = GetAnimPeriod(m_AnimNo);
        if (EndTime <= m_AnimTimer) {
            if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
            {
                m_AnimTimer = EndTime - 0.016; 
                m_pAnimCtrl->SetTrackPosition(0, EndTime);
                m_AnimSpeed = 0.0;
            }
        }
    }
}

//------------------------------------------------------------------------------------.

HRESULT MeshObject::FindVerticesOnPoly(LPD3DXMESH pMesh, DWORD dwPolyIndex, DirectX::XMFLOAT3* pVertices)
{
	//頂点ごとのバイト数を取得.
	DWORD dwStride = pMesh->GetNumBytesPerVertex();
	//頂点数を取得.
	DWORD dwVertexAmt = pMesh->GetNumVertices();
	//面数を取得.
	DWORD dwPoly = pMesh->GetNumFaces();

	WORD* pwPoly = nullptr;

	//インデックスバッファをロック(読み込みモード).
	pMesh->LockIndexBuffer(
		D3DLOCK_READONLY,
		reinterpret_cast<VOID**>(&pwPoly));

	BYTE* pbVertices = nullptr;		//頂点(バイト型).
	FLOAT* pfVertices = nullptr;	//頂点(float型).
	LPDIRECT3DVERTEXBUFFER9 VB = nullptr;	//頂点バッファ.

	//頂点情報の取得.
	pMesh->GetVertexBuffer(&VB);

	//頂点バッファのロック.
	if (SUCCEEDED(
		VB->Lock(0, 0, reinterpret_cast<VOID**>(&pbVertices), 0)))
	{
		//ポリゴンの頂点1つ目を取得.
		pfVertices = reinterpret_cast<FLOAT*>(&pbVertices[dwStride * pwPoly[dwPolyIndex * 3]]);
		pVertices[0].x = pfVertices[0];
		pVertices[0].y = pfVertices[1];
		pVertices[0].z = pfVertices[2];

		//ポリゴンの頂点2つ目を取得.
		pfVertices = reinterpret_cast<FLOAT*>(&pbVertices[dwStride * pwPoly[dwPolyIndex * 3 + 1]]);
		pVertices[1].x = pfVertices[0];
		pVertices[1].y = pfVertices[1];
		pVertices[1].z = pfVertices[2];

		//ポリゴンの頂点3つ目を取得.
		pfVertices = reinterpret_cast<FLOAT*>(&pbVertices[dwStride * pwPoly[dwPolyIndex * 3 + 2]]);
		pVertices[2].x = pfVertices[0];
		pVertices[2].y = pfVertices[1];
		pVertices[2].z = pfVertices[2];

		pMesh->UnlockIndexBuffer(); //√ロック解除.
		VB->Unlock();	//ロック解除.
	}
	VB->Release();	//不要になったので解放.

	return S_OK;
}
