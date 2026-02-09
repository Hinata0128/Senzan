#pragma once
#include "..\UIObject.h"

class Fade
	: public UIObject
{
public:
	enum class FadeType
	{
		FadeIn,
		FadeOut,
	};
public:
	Fade();
	virtual ~Fade() override;

	virtual void Update()	override;
	virtual void Draw()		override;

	/****************************************
	* @brief フェードを開始.
	* @param type：開始するフェードの種類.
	****************************************/
	void StartFade(const FadeType type);

public: // Getter、Setter.

	/****************************************
	* @brief フェード中か判定.
	****************************************/
	const bool IsFading() const;


	/****************************************
	* @brief フェードが終了しているか判定.
	* @param type：終了判定をするフェードの種類.
	****************************************/
	const bool IsFadeCompleted(const FadeType type) const;

private:
	FadeType m_LastFadeType;// 最後に使用したフェードの種類.
	float m_AlphaSpeed;		// α値の減算速度.
	bool m_IsStartFade;		// フェードを開始するか.	
	bool m_IsFadeCompleted;	// フェードが完了したか.
};
