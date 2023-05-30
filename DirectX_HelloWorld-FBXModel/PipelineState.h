#pragma once
#include "ComPtr.h"
#include <d3dx12.h>
#include <string>

class PipelineState
{
public:
	PipelineState();	//コンストラクタである程度の設定をする
	bool IsValid();		//生成に成功したかどうかを返す

	void SetInputLayout(D3D12_INPUT_LAYOUT_DESC layout);		//入力レイアウトを設定
	void SetRootSignature(ID3D12RootSignature* rootSignature);	//ルートシグネチャを設定
	void SetVS(std::wstring fliePath);	//頂点シェーダを設定
	void SetPS(std::wstring filePath);	//ピクセルシェーダを設定
	void Create();	//パイプラインステートを生成

	ID3D12PipelineState* Get();

private:
	bool m_IsValid = false;	//生成に成功したかどうか
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};	//パイプラインステートの設定
	ComPtr<ID3D12PipelineState> m_pPipelineState = nullptr;	//パイプラインステート
	ComPtr<ID3DBlob> m_pPsBlob;	//頂点シェーダー
	ComPtr<ID3DBlob> m_pVsBlob;	//ピクセルシェーダー
};