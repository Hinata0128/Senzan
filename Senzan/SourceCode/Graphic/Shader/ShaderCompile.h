#pragma once

/*********************************************
* @brief シェーダーコンパイル.
*********************************************/
static bool ShaderCompile(const std::string* filePath, const char* funcName, const char* profile, ID3DBlob*& pCompiledShader, ID3DBlob*& pErrors)
{
	// シェーダーフラグ.
	UINT		uCompileFlag = 0;

#ifdef _DEBUG
	uCompileFlag = D3D10_SHADER_DEBUG | D3D10_SHADER_SKIP_OPTIMIZATION;
#endif//#ifdef _DEBUG

	//HLSLからバーテックスシェーダのブロブを作成.
	auto result = D3DX11CompileFromFile(
		filePath->c_str(),
		nullptr,
		nullptr,
		funcName,
		profile,
		uCompileFlag,
		0,
		nullptr,
		&pCompiledShader,
		&pErrors,
		nullptr);

	if (FAILED(result))
	{
		SAFE_RELEASE(pCompiledShader);
		assert(0 && "hlslの読み込みに失敗");
		return false;
	}

	return true;
}