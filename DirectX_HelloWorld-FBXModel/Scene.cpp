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
#include "AssimpLoader.h"
#include "DescriptorHeap.h"
#include "Texture2D.h"

Scene* g_Scene;

using namespace DirectX;

VertexBuffer* vertexBuffer;
ConstantBuffer* constantBuffer[Engine::FRAME_BUFFER_COUNT];
RootSignature* rootSignature;
PipelineState* pipelineState;
IndexBuffer* indexBuffer;

const wchar_t* modelFile = L"Assets/Alicia/FBX/Alicia_solid_Unity.FBX";
std::vector<Mesh> meshes;
std::vector<VertexBuffer*> vertexBuffers;	//���b�V���̐����̒��_�o�b�t�@
std::vector<IndexBuffer*> indexBuffers;		//���b�V���̐����̃C���f�b�N�X�o�b�t�@

bool Scene::Init()
{
	//----�A���V�A�E���f���ǂݍ��ݏ���----//
	ImportSettings importSettings = //���ꎩ�͎̂���̓ǂݍ��ݐݒ�\����
	{
		modelFile,
		meshes,
		false,
		true	//�A���V�A�̃��f���́A�e�N�X�`����UV��V�������]���Ă�H���ۂ��̂ŁAUV���W���t�]������B
	};

	AssimpLoader loader;
	if (!loader.Load(importSettings))
	{
		return false;
	}

	//���b�V���̐��������_�o�b�t�@��p�ӂ���
	vertexBuffers.reserve(meshes.size());
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto size = sizeof(Vertex) * meshes[i].Vertices.size();
		auto stride = sizeof(Vertex);
		auto vertices = meshes[i].Vertices.data();
		auto pVB = new VertexBuffer(size, stride, vertices);
		if (!pVB->IsValid())
		{
			printf("���_�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		vertexBuffers.push_back(pVB);
	}

	//���b�V���̐������C���f�b�N�X�o�b�t�@��p�ӂ���
	indexBuffers.reserve(meshes.size());
	for (size_t i = 0; i < meshes.size(); i++)
	{
		//�C���f�b�N�X�o�b�t�@�̐���
		auto size = sizeof(uint32_t) * meshes[i].Indices.size();
		auto indices = meshes[i].Indices.data();
		auto pIB = new IndexBuffer(size, indices);
		if (!pIB->IsValid())
		{
			printf("�C���f�b�N�X�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		indexBuffers.push_back(pIB);
	}

	auto eyePos = XMVectorSet(0.0f, 120.0f, 75.0f, 0.0f);	//���_�̈ʒu
	auto targetPos = XMVectorSet(0.0f,120.0f,0.0f,0.0f);	//���_����������W
	auto upward = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);	//�������\���x�N�g��
	auto fov = XMConvertToRadians(60);	//����p
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

void Scene::Update()
{
	//float rotateY = 0.0f;	//���錾
	//rotateY += 0.02f;
	//auto currentIndex = g_Engine->CurrentBackBufferIndex();	//���݂̃t���[���ԍ����擾
	//auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();	//���݂̃t���[���ԍ��ɑΉ�����萔�o�b�t�@���擾
	//currentTransform->World = DirectX::XMMatrixRotationY(rotateY);
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();	//���݂̃t���[���ԍ����擾����
	auto commandList = g_Engine->CommandList();				//�R�}���h���X�g

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = vertexBuffers[i]->view();	//���̃��b�V���ɑΉ����钸�_�o�b�t�@
		auto ibView = indexBuffers[i]->View();	//���̃��b�V���ɑΉ�����C���f�b�N�X�o�b�t�@

		commandList->SetGraphicsRootSignature(rootSignature->Get());
		commandList->SetPipelineState(pipelineState->Get());
		commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());	//�萔�o�b�t�@���Z�b�g

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);	//�O�p�`��`�悷��ݒ�ɂ���
		commandList->IASetVertexBuffers(0, 1, &vbView);	//���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
		commandList->IASetIndexBuffer(&ibView);	//�C���f�b�N�X�o�b�t�@���Z�b�g����

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0);	//�C���f�b�N�X�̐����`�悷��
	}
}
