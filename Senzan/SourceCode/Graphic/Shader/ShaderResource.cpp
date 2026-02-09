#include "ShaderResource.h"
#include "ShaderCompile.h"
#include "ShadowMapStaticMesh\VSShadowMapStaticMesh.h"
#include "ShadowMapSkinMesh\VSShadowMapSkinMesh.h"

ShaderResource::ShaderResource()
    : m_pVSShadowMapStaticMesh  (std::make_unique<VSShadowMapStaticMesh>())
    , m_pPSShadowMapStaticMesh  (std::make_unique<PixelShaderBase>())
    , m_pVSShadowMapSkinMesh    (std::make_unique<VSShadowMapSkinMesh>())
    , m_pPSShadowMapSkinMesh    (std::make_unique<PixelShaderBase>())    
{
    Init();
}

//---------------------------------------------------------------------------------------.

ShaderResource::~ShaderResource()
{
}

//---------------------------------------------------------------------------------------.

const std::unique_ptr<VertexShaderBase>& ShaderResource::GetVSShadowMapStaticMesh()
{
    return m_pVSShadowMapStaticMesh;
}

//---------------------------------------------------------------------------------------.

const std::unique_ptr<PixelShaderBase>& ShaderResource::GetPSShadowMapStaticMesh()
{
    return m_pPSShadowMapStaticMesh;
}

//---------------------------------------------------------------------------------------.

const std::unique_ptr<VertexShaderBase>& ShaderResource::GetVSShadowMapSkinMesh()
{
    return m_pVSShadowMapSkinMesh;
}

//---------------------------------------------------------------------------------------.

const std::unique_ptr<PixelShaderBase>& ShaderResource::GetPSShadowMapSkinMesh()
{
    return m_pPSShadowMapSkinMesh;
}

//---------------------------------------------------------------------------------------.

void ShaderResource::Init()
{

    ID3DBlob* pBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;

    // ShadowMap.
    ShaderCompile(new std::string("Data\\Shader\\ShadowMap\\StaticMeshShadowMap.hlsl"), "VSMain", "vs_5_0", pBlob, pErrorBlob);
    m_pVSShadowMapStaticMesh->Init(pBlob);
    ShaderCompile(new std::string("Data\\Shader\\ShadowMap\\StaticMeshShadowMap.hlsl"), "PSMain", "ps_5_0", pBlob, pErrorBlob);
    m_pPSShadowMapStaticMesh->Init(pBlob);

    ShaderCompile(new std::string("Data\\Shader\\ShadowMap\\SkinMeshShadowMap.hlsl"), "VSMain", "vs_5_0", pBlob, pErrorBlob);
    m_pVSShadowMapSkinMesh->Init(pBlob);
    ShaderCompile(new std::string("Data\\Shader\\ShadowMap\\SkinMeshShadowMap.hlsl"), "PSMain", "ps_5_0", pBlob, pErrorBlob);
    m_pPSShadowMapSkinMesh->Init(pBlob);
}