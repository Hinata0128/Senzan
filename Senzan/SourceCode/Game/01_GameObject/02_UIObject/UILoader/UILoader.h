#pragma once
#include "02_UIObject/UIObject.h"

/************************************
*	UI“ÇŠÖ˜A.
*************************************/
namespace UILoader
{
	/******************************************************
	* @brief SceneJson‚ÅSpiteManager‚©‚çî•ñ‚ğæ“¾.
	* @param name	SceneJson‚ÌƒpƒX.
	* @param uis	std::vector<CUIObject*>.
	******************************************************/
	void LoadFromJson(
		const std::string& scenepath,
		std::vector<std::shared_ptr<UIObject>>& uis);
}