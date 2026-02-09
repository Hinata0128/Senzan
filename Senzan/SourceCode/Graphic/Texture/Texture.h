#pragma once
/*************************************
*   テクスチャクラス.
*************************************/
class Texture final
{
public:
	Texture();
	~Texture();
    
    /**************************************************
    * @brief テクスチャの初期化.
    * @param width      ：幅.
    * @param height     ：高さ.
    * @param format     ：カラーフォーマット.
    * @param bindFlags  ：バインドフラグ.
    **************************************************/
    void Init(int width, int height, DXGI_FORMAT format, UINT bindFlags);


    /**************************************************
    * @brief テクスチャを取得.
    **************************************************/
    [[nodiscard]] ID3D11Texture2D* GetTexture() const;
private:
    ID3D11Texture2D* m_pTexture;
};