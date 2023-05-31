#include "Scene.h"
#include "Engine.h"
#include "App.h"
#include <d3dx12.h>
#include "SharedStruct.h"
#include "VertexBuffer.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "IndexBuffer.h"

Scene* g_Scene;

using namespace DirectX;

VertexBuffer* vertexBuffer;
ConstantBuffer* constantBuffer[Engine::FRAME_BUFFER_COUNT];
RootSignature* rootSignature;
PipelineState* pipelineState;
IndexBuffer* indexBuffer;

#define SHAPES (1)	//(1)にすると四角形、(0)は三角形

bool Scene::Init()
{
#if SHAPES
	Vertex vertices[4] = {};
	vertices[0].Position = XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[0].Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);

	vertices[1].Position = XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertices[1].Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertices[2].Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	vertices[3].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[3].Color = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
#else
	Vertex vertices[3] = {};
	vertices[0].Position = XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[0].Color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	vertices[1].Position = XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertices[1].Color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

	vertices[2].Position = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[2].Color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
#endif

	auto vertexSize = sizeof(Vertex) * std::size(vertices);
	auto vertexStride = sizeof(Vertex);
	vertexBuffer = new VertexBuffer(vertexSize, vertexStride, vertices);
	if (!vertexBuffer->IsValid())
	{
		printf("頂点バッファの生成に失敗\n");
		return false;
	}

#if SHAPES
	uint32_t indices[] = { 0,1,2,0,2,3 };

	//インデックスバッファの生成
	auto size = sizeof(uint32_t) * std::size(indices);
	indexBuffer = new IndexBuffer(size, indices);
	if (!indexBuffer->IsValid())
	{
		printf("インデックスバッファの生成に失敗\n");
		return false;
	}
#endif

	auto eyePos = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);	//視点の位置
	auto targetPos = XMVectorZero();	//視点を向ける座標
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);	//上方向を表すベクトル
	auto fov = XMConvertToRadians(37.5);	//視野角
	auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!constantBuffer[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
			return false;
		}

		//変換行列の登録
		auto ptr = constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
	}

	rootSignature = new RootSignature();
	if (!rootSignature->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	pipelineState = new PipelineState();
	pipelineState->SetInputLayout(Vertex::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());
	pipelineState->SetVS(L"../x64/Debug/SimpleVS.cso");
	pipelineState->SetPS(L"../x64/Debug/SimplePS.cso");
	pipelineState->Create();
	if (!pipelineState->IsValid())
	{
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}

	printf("シーンの初期化に成功\n");
	return true;
}

float rotateY;	//仮宣言
void Scene::Update()
{
	rotateY += 0.02f;
	auto currentIndex = g_Engine->CurrentBackBufferIndex();	//現在のフレーム番号を取得
	auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();	//現在のフレーム番号に対応する定数バッファを取得
	currentTransform->World = DirectX::XMMatrixRotationY(rotateY);
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();	//現在のフレーム番号を取得する
	auto commandList = g_Engine->CommandList();	//コマンドリスト
	auto vbView = vertexBuffer->view();	//頂点バッファビュー
#if SHAPES
	auto ibView = indexBuffer->View();	//インデックスバッファビュー
#endif

	commandList->SetGraphicsRootSignature(rootSignature->Get());
	commandList->SetPipelineState(pipelineState->Get());
	commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());	//定数バッファをセット

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//三角形を描画する設定にする
	commandList->IASetVertexBuffers(0, 1, &vbView);	//頂点バッファをスロット0番を使って1個だけ設定する
#if SHAPES
	commandList->IASetIndexBuffer(&ibView);	//インデックスバッファをセットする
#endif

#if SHAPES
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
#else
	commandList->DrawInstanced(3, 1, 0, 0);	//3個の頂点を描画する
#endif
}
