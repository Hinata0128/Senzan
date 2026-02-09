#pragma once

#include "Game/01_GameObject/GameObject.h"
#include "System/Singleton/ResourceManager/ResourceManager.h"
#include "System/Utility/Math/Math.h"
#include "Resource/Mesh/MeshBase.h"

/**********************************************************************************
* @author    : 淵脇 未来.
* @date      : 2026/01/02.
* @brief     : メッシュを保持するゲームオブジェクト基底クラス.
**********************************************************************************/
class MeshObject
	: public GameObject
{
public:
	MeshObject();
	virtual ~MeshObject()override;

	virtual void Update() override;
	virtual void LateUpdate() override;
	virtual void Draw() override;
	void DrawDepth();

	/**********************************************************
	* @brief     : メッシュをアタッチする.
	* @param[in] : pMesh  アタッチするメッシュの共有ポインタ.
	**********************************************************/
	void AttachMesh(const std::shared_ptr<MeshBase>& pMesh);


	/**********************************************************
	* @brief     : メッシュをデタッチする.
	**********************************************************/
	void DetachMesh();

public: // Getter／Setter.

	// アタッチされたリソース名を取得.
	const std::string GetResourceName() const;

	// アタッチ中のメッシュを取得 (weak_ptrで返す).
	const std::weak_ptr<MeshBase> GetAttachMesh() const {
		return m_pMesh;
	};

	// Get currently set alpha for attached mesh (1.0 = opaque).
	float GetAlpha() const;

	// Set object-local alpha (multiplies mesh alpha). 1.0 = opaque.
	void SetAlpha(float alpha);

	// ライト有無フラグを設定.
	void SetIsLight(const bool& isLight);

	// シャドウ有無フラグを設定.
	void SetIsShadow(const bool& isShadow);

	// アニメーションの再生速度を設定.
	void SetAnimSpeed(double speed);

	// アニメーションのループ設定.
	void SetIsLoop(bool isLoop);
	
	// アニメーション開始時間を設定.
	void SetAnimTime(double StartTime);
	
	// アニメーションを変更するテンプレート.
	template<typename T>
	void ChangeAnim(T index_T)
	{
		int index = static_cast<int>(index_T);
		// 同じアニメ番号なら何もしない.
		if (m_AnimNo == index) { return; }

		m_AnimNo = index;
		if (std::shared_ptr<SkinMesh> skinMesh = std::dynamic_pointer_cast<SkinMesh>(m_pMesh.lock()))
		{
			skinMesh->ChangeAnimSet(index, m_pAnimCtrl);
			m_AnimTimer = 0.0;
		}
	}

	/**********************************************************
	* @brief     : 指定アニメの再生時間（周期）を取得.
	* @param[in] : index  アニメーションインデックス.
	* @return    : アニメの再生時間 (秒).
	**********************************************************/
	double GetAnimPeriod(int index) const;
	
	/**********************************************************
	* @brief     : 指定アニメが終了しているか判定する.
	* @tparam    : T      インデックスの型.
	* @param[in] : Index_T アニメインデックス.
	* @return    : 終了している場合 true.
	**********************************************************/
	template<typename T>
	inline bool IsAnimEnd(T Index_T) noexcept
	{
		int index = static_cast<int>(Index_T);

		double EndTime = GetAnimPeriod(index);
		return (m_AnimTimer + static_cast<double>(GetDelta())) >= EndTime;
	};

private:
	/**********************************************************
	* @brief     : ループアニメ時の時間設定を行う内部関数.
	**********************************************************/
	void IsLoopAnimTimeSet();


	/**********************************************************
	* @brief     : ポリゴン上の頂点を検索する内部ユーティリティ.
	* @param[in] : pMesh       対象メッシュ.
	* @param[in] : dwPolyIndex ポリゴンインデックス.
	* @param[out]: pVertices   頂点出力バッファ (サイズ3).
	* @return    : HRESULT     成功/失敗コード.
	**********************************************************/
	HRESULT FindVerticesOnPoly(
		LPD3DXMESH pMesh,
		DWORD dwPolyIndex,
		DirectX::XMFLOAT3* pVertices);

protected:
	std::weak_ptr<MeshBase>		m_pMesh; // アタッチ中のメッシュ (弱参照).

	bool m_IsLight; 	// ライト影響を受けるか.
	bool m_IsShadow; // シャドウを投影するか.

	// SkinMesh 用アニメコントローラ.
	LPD3DXANIMATIONCONTROLLER	m_pAnimCtrl; // アニメーションコントローラ.

	int						m_AnimNo; 	// アニメ番号.
	double					m_AnimSpeed; // アニメ再生速度.
	double					m_AnimTimer; // アニメタイマー (経過時間).
	bool					m_Isloop; 	// ループフラグ.
	DirectX::XMFLOAT3			m_BonePos; // ボーン位置.
	float m_ObjectAlpha; // object-local alpha (1.0 = opaque)
};

