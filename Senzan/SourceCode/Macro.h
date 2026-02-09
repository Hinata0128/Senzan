#pragma once
// 解放.
#define SAFE_RELEASE(p)do{if(p!=nullptr){(p)->Release();(p)=nullptr;}}while(0)

// 破棄.
#define SAFE_DELETE(p)do{if(p!=nullptr){delete (p);(p)=nullptr;}}while(0)

// 配列破棄.
#define SAFE_DELETE_ARRAY(p)do{if(p!=nullptr){delete[](p);(p) = nullptr;}}while(0)

// D3DXMATRIXをDirectX::XMMATRIXに変換.
inline DirectX::XMMATRIX D3DXMatrixToXMMatrix(const D3DXMATRIX & d3d9Matrix)
{
	// D3DXMATRIXの各行のfloat要素をDirectX::XMVECTORとしてロードする
	// XMMatrixは行優先でデータを格納する

	DirectX::XMVECTOR Row0 = DirectX::XMVectorSet(d3d9Matrix._11, d3d9Matrix._12, d3d9Matrix._13, d3d9Matrix._14);
	DirectX::XMVECTOR Row1 = DirectX::XMVectorSet(d3d9Matrix._21, d3d9Matrix._22, d3d9Matrix._23, d3d9Matrix._24);
	DirectX::XMVECTOR Row2 = DirectX::XMVectorSet(d3d9Matrix._31, d3d9Matrix._32, d3d9Matrix._33, d3d9Matrix._34);
	DirectX::XMVECTOR Row3 = DirectX::XMVectorSet(d3d9Matrix._41, d3d9Matrix._42, d3d9Matrix._43, d3d9Matrix._44);

	// 4つのXMVECTOR行ベクトルからXMMATRIXを構築する
	return DirectX::XMMATRIX(Row0, Row1, Row2, Row3);
}

// D3DXMATRIXをDirectX::XMMATRIXに変換.
inline void XMMatrixToD3DXMatrix(D3DXMATRIX & outD3d9Matrix, const DirectX::XMMATRIX & xmMatrix)
{
	// XMMATRIXの各行ベクトルを取得.
	DirectX::XMVECTOR Row0 = xmMatrix.r[0];
	DirectX::XMVECTOR Row1 = xmMatrix.r[1];
	DirectX::XMVECTOR Row2 = xmMatrix.r[2];
	DirectX::XMVECTOR Row3 = xmMatrix.r[3];

	// 各行ベクトルをD3DXMATRIXのメモリ領域にストアする.
	DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&outD3d9Matrix.m[0]), Row0);
	DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&outD3d9Matrix.m[1]), Row1);
	DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&outD3d9Matrix.m[2]), Row2);
	DirectX::XMStoreFloat4(reinterpret_cast<DirectX::XMFLOAT4*>(&outD3d9Matrix.m[3]), Row3);
}

// IMGUI日本語対応.
#define IMGUI_JP(str) reinterpret_cast<const char*>(u8##str)

// UIEditorに使用、ナンバリング削除マクロ.
#include <regex>
inline std::string GetBaseName(const std::string& name)
{
	std::regex re(R"((.*?)(?:_\d+)?$)");
	std::smatch match;
	if (std::regex_match(name, match, re)) {
		return match[1].str();
	}
	return name;
}