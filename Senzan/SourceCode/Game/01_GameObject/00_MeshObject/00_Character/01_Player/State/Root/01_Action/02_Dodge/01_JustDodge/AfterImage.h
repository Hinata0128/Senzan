#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>

class SkinMesh;

/**************************************************
*	プレイヤーの残像エフェクトクラス.
*	ジャスト回避時に使用する.
**/
class AfterImage
{
public:
	// 残像1つ分のデータ.
	struct ImageData
	{
		DirectX::XMFLOAT3 Position;		// 位置.
		DirectX::XMFLOAT3 Rotation;		// 回転.
		DirectX::XMFLOAT3 Scale;		// スケール.
		double AnimTime;				// アニメーション時間.
		int AnimNo;						// アニメーション番号.
		float LifeTime;					// 残り生存時間.
		float MaxLifeTime;				// 最大生存時間.
		float Alpha;					// 現在のアルファ値.
	};

public:
	AfterImage();
	~AfterImage();

	/**********************************************************
	* @brief     : 残像を追加する.
	* @param[in] : position  位置.
	* @param[in] : rotation  回転.
	* @param[in] : scale     スケール.
	* @param[in] : animTime  アニメーション時間.
	* @param[in] : animNo    アニメーション番号.
	**********************************************************/
	void AddImage(
		const DirectX::XMFLOAT3& position,
		const DirectX::XMFLOAT3& rotation,
		const DirectX::XMFLOAT3& scale,
		double animTime,
		int animNo);

	/**********************************************************
	* @brief     : 残像を更新する.
	* @param[in] : deltaTime  デルタタイム.
	**********************************************************/
	void Update(float deltaTime);

	/**********************************************************
	* @brief     : 残像を描画する.
	* @param[in] : pSkinMesh  スキンメッシュ.
	* @param[in] : pAnimCtrl  アニメーションコントローラ.
	**********************************************************/
	void Draw(std::shared_ptr<SkinMesh> pSkinMesh, LPD3DXANIMATIONCONTROLLER pAnimCtrl);

	/**********************************************************
	* @brief     : 全ての残像をクリアする.
	**********************************************************/
	void Clear();

	/**********************************************************
	* @brief     : 残像が存在するか.
	* @return    : 残像が存在する場合 true.
	**********************************************************/
	bool HasImages() const { return !m_Images.empty(); }

	// 設定用.
	void SetLifeTime(float time) { m_LifeTime = time; }
	void SetStartAlpha(float alpha) { m_StartAlpha = alpha; }
	void SetDarkness(float darkness) { m_Darkness = darkness; }
	void SetSpawnInterval(float interval) { m_SpawnInterval = interval; }
	float GetSpawnInterval() const { return m_SpawnInterval; }

private:
	std::vector<ImageData> m_Images;	// 残像リスト.

	// 設定パラメータ.
	float m_LifeTime;		// 残像の生存時間 (デフォルト: 0.4秒).
	float m_StartAlpha;		// 開始時のアルファ値 (デフォルト: 0.6).
	float m_Darkness;		// 黒みの強さ (0.0〜1.0, デフォルト: 0.3).
	float m_SpawnInterval;	// 残像生成間隔 (デフォルト: 0.1秒).
};
