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

#define SHAPES (1)	//(1)�ɂ���Ǝl�p�`�A(0)�͎O�p�`

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
		printf("���_�o�b�t�@�̐����Ɏ��s\n");
		return false;
	}

#if SHAPES
	uint32_t indices[] = { 0,1,2,0,2,3 };

	//�C���f�b�N�X�o�b�t�@�̐���
	auto size = sizeof(uint32_t) * std::size(indices);
	indexBuffer = new IndexBuffer(size, indices);
	if (!indexBuffer->IsValid())
	{
		printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
		return false;
	}
#endif

	auto eyePos = XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);	//���_�̈ʒu
	auto targetPos = XMVectorZero();	//���_����������W
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);	//�������\���x�N�g��
	auto fov = XMConvertToRadians(37.5);	//����p
	auto aspect = static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT);

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!constantBuffer[i]->IsValid())
		{
			printf("�ϊ��s��p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		//�ϊ��s��̓o�^
		auto ptr = constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
	}

	rootSignature = new RootSignature();
	if (!rootSignature->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
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
		printf("�p�C�v���C���X�e�[�g�̐����Ɏ��s\n");
		return false;
	}

	printf("�V�[���̏������ɐ���\n");
	return true;
}

float rotateY;	//���錾
void Scene::Update()
{
	rotateY += 0.02f;
	auto currentIndex = g_Engine->CurrentBackBufferIndex();	//���݂̃t���[���ԍ����擾
	auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();	//���݂̃t���[���ԍ��ɑΉ�����萔�o�b�t�@���擾
	currentTransform->World = DirectX::XMMatrixRotationY(rotateY);
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();	//���݂̃t���[���ԍ����擾����
	auto commandList = g_Engine->CommandList();	//�R�}���h���X�g
	auto vbView = vertexBuffer->view();	//���_�o�b�t�@�r���[
#if SHAPES
	auto ibView = indexBuffer->View();	//�C���f�b�N�X�o�b�t�@�r���[
#endif

	commandList->SetGraphicsRootSignature(rootSignature->Get());
	commandList->SetPipelineState(pipelineState->Get());
	commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());	//�萔�o�b�t�@���Z�b�g

	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//�O�p�`��`�悷��ݒ�ɂ���
	commandList->IASetVertexBuffers(0, 1, &vbView);	//���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
#if SHAPES
	commandList->IASetIndexBuffer(&ibView);	//�C���f�b�N�X�o�b�t�@���Z�b�g����
#endif

#if SHAPES
	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
#else
	commandList->DrawInstanced(3, 1, 0, 0);	//3�̒��_��`�悷��
#endif
}
