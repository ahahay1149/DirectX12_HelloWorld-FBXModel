#pragma once
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include "ComPtr.h"

#pragma comment(lib,"d3d12.lib")	//d3d12���C�u�����������N����
#pragma comment(lib,"dxgi.lib")		//dxgi���C�u�����������N����

class Engine
{
public:
	enum { FRAME_BUFFER_COUNT = 2 }; //�_�u���o�b�t�@�����O����̂�2

public:
	bool Init(HWND hwnd, UINT windowWidth, UINT windowHeight);	//�G���W��������

	void BeginRender();	//�`��̊J�n����
	void EndRender(); //�`��̏I������

public:		//�O����A�N�Z�X�������̂�Getter�Ƃ��Č��J�������
	ID3D12Device6* Device();
	ID3D12GraphicsCommandList* CommandList();
	UINT CurrentBackBufferIndex();

private:	//DirectX12�������Ɏg���֐�����
	bool CreateDevice();			//�f�o�C�X�𐶐�
	bool CreateCommandQueue();		//�R�}���h�L���[�𐶐�
	bool CreateSwapChain();			//�X���b�v�`�F�C���𐶐�
	bool CreateCommandList();		//�R�}���h���X�g�E�R�}���h�A���P�[�^�[�𐶐�
	bool CreateFence();				//�t�F���X�𐶐�
	void CreateViewPort();			//�r���[�|�[�g�𐶐�
	void CreateScissorRect();		//�V�U�[��`�𐶐�

private:
	HWND m_hWnd;
	UINT m_FrameBufferWidth = 0;
	UINT m_FrameBufferHeight = 0;
	UINT m_CurrentBackBufferIndex = 0;

	ComPtr<ID3D12Device6> m_pDevice = nullptr;		//�f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue = nullptr;	//�R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain = nullptr; //�X���b�v�`�F�C��
	ComPtr<ID3D12CommandAllocator> m_pAllocator[FRAME_BUFFER_COUNT] = { nullptr }; //�R�}���h�A���P�[�^�[
	ComPtr<ID3D12GraphicsCommandList> m_pCommandList = nullptr; //�R�}���h���X�g
	HANDLE m_fenceEvent = nullptr;					//�t�F���X�Ŏg���C�x���g
	ComPtr<ID3D12Fence> m_pFence = nullptr;			//�t�F���X
	UINT64 m_fenceValue[FRAME_BUFFER_COUNT];		//�t�F���X�̒l 
	D3D12_VIEWPORT m_Viewport;						//�r���[�|�[�g
	D3D12_RECT m_Scissor;							//�V�U�[��`

private: //�`��Ɏg���I�u�W�F�N�g�Ƃ��̐����֐�����
	bool CreateRenderTarget();	//�����_�[�^�[�Q�b�g�𐶐�
	bool CreateDepthStencil();	//�[�x�X�e���V���o�b�t�@�𐶐�

	UINT m_RtvDescriptorSize = 0;	//�����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�T�C�Y
	ComPtr<ID3D12DescriptorHeap> m_pRtvHeap = nullptr; //�����_�[�^�[�Q�b�g�̃f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> m_pRenderTargets[FRAME_BUFFER_COUNT] = { nullptr }; //�����_�[�^�[�Q�b�g�i�_�u���o�b�t�@�����O����̂�2�j

	UINT m_DsvDescriptorSize = 0; //�[�x�X�e���V���̃f�B�X�N���v�^�[�T�C�Y
	ComPtr<ID3D12DescriptorHeap> m_pDsvHeap = nullptr; //�[�x�X�e���V���̃f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Resource> m_pDepthStencilBuffer = nullptr; //�[�x�X�e���V���o�b�t�@�i��������1�ł����j

private: //�`�惋�[�v�Ŏg�p�������
	ID3D12Resource* m_currentRenderTarget = nullptr; //���݂̃t���[���̃����_�[�^�[�Q�b�g���ꎞ�I�ɕۑ����Ă����֐�
	void WaitRender();
};

extern Engine* g_Engine;