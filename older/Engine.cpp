#include "Engine.h"
#include <d3d12.h>
#include <stdio.h>
#include <Windows.h>

#include <DirectXTex.h>
#include <d3dx12.h>

Engine* g_Engine;

bool Engine::Init(HWND hwnd, UINT windowWidth, UINT windowHeight)
{
	m_FrameBufferWidth = windowWidth;
	m_FrameBufferHeight = windowHeight;
	m_hWnd = hwnd;

	if (!CreateDevice())
	{
		printf("�f�o�C�X�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateCommandQueue())
	{ 
		printf("�R�}���h�L���[�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateSwapChain())
	{
		printf("�X���b�v�`�F�C���̐����Ɏ��s\n");
		return false;
	}

	if (!CreateCommandList())
	{
		printf("�R�}���h���X�g�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateFence())
	{
		printf("�t�F���X�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateRenderTarget())
	{
		printf("�����_�[�^�[�Q�b�g�̐����Ɏ��s\n");
		return false;
	}

	if (!CreateDepthStencil())
	{
		printf("�f�v�X�X�e���V���o�b�t�@�̐����Ɏ��s\n");
		return false;
	}

	// �r���[�|�[�g�ƃV�U�[��`�𐶐�
	CreateViewPort();
	CreateScissorRect();

	printf("�`��G���W���̏������ɐ���\n");
	return true;
}


bool Engine::CreateDevice()
{
	auto hr = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_pDevice.ReleaseAndGetAddressOf()));
	return SUCCEEDED(hr);
}

bool Engine::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	auto hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.ReleaseAndGetAddressOf()));

	return SUCCEEDED(hr);
}

bool Engine::CreateSwapChain()
{
	// DXGI�t�@�N�g���[�̐���
	IDXGIFactory4* pFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	if (FAILED(hr))
	{
		return false;
	}

	// �X���b�v�`�F�C���̐���
	DXGI_SWAP_CHAIN_DESC desc = {};
	desc.BufferDesc.Width = m_FrameBufferWidth;
	desc.BufferDesc.Height = m_FrameBufferHeight;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAME_BUFFER_COUNT;
	desc.OutputWindow = m_hWnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// �X���b�v�`�F�C���̐���
	IDXGISwapChain* pSwapChain = nullptr;
	hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
	if (FAILED(hr))
	{
		pFactory->Release();
		return false;
	}

	// IDXGISwapChain3���擾
	hr = pSwapChain->QueryInterface(IID_PPV_ARGS(m_pSwapChain.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		pFactory->Release();
		pSwapChain->Release();
		return false;
	}

	// �o�b�N�o�b�t�@�ԍ����擾
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	pFactory->Release();
	pSwapChain->Release();
	return true;
}

bool Engine::CreateCommandList()
{
	// �R�}���h�A���P�[�^�[�̍쐬
	HRESULT hr;
	for (size_t i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		hr = m_pDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_pAllocator[i].ReleaseAndGetAddressOf()));
	}

	if (FAILED(hr))
	{
		return false;
	}

	// �R�}���h���X�g�̐���
	hr = m_pDevice->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pAllocator[m_CurrentBackBufferIndex].Get(),
		nullptr,
		IID_PPV_ARGS(&m_pCommandList)
	);

	if (FAILED(hr))
	{
		return false;
	}

	//�R�}���h���X�g�͊J����Ă����Ԃō쐬�����̂ŁA�����������B
	m_pCommandList->Close();

	return true;
}

bool Engine::CreateFence()
{
	for (auto i = 0u; i < FRAME_BUFFER_COUNT; i++)
	{
		m_fenceValue[i] = 0;
	}

	auto hr = m_pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	m_fenceValue[m_CurrentBackBufferIndex]++;

	//�������s���Ƃ��̃C�x���g�n���h�����쐬����B
	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	return m_fenceEvent != nullptr;
}

void Engine::CreateViewPort()
{
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.Width = static_cast<float>(m_FrameBufferWidth);
	m_Viewport.Height = static_cast<float>(m_FrameBufferHeight);
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
}

void Engine::CreateScissorRect()
{
	m_Scissor.left = 0;
	m_Scissor.right = m_FrameBufferWidth;
	m_Scissor.top = 0;
	m_Scissor.bottom = m_FrameBufferHeight;
}

bool Engine::CreateRenderTarget()
{
	// RTV�p�̃f�B�X�N���v�^�q�[�v���쐬����
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FRAME_BUFFER_COUNT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	auto hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_pRtvHeap.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		return false;
	}

	// �f�B�X�N���v�^�̃T�C�Y���擾�B
	m_RtvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
	{
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(m_pRenderTargets[i].ReleaseAndGetAddressOf()));
		m_pDevice->CreateRenderTargetView(m_pRenderTargets[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += m_RtvDescriptorSize;
	}

	return true;
}

bool Engine::CreateDepthStencil()
{
	//DSV�p�̃f�B�X�N���v�^�q�[�v���쐬����
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	auto hr = m_pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&m_pDsvHeap));
	if (FAILED(hr))
	{
		return false;
	}

	//�f�B�X�N���v�^�̃T�C�Y���擾
	m_DsvDescriptorSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

	D3D12_CLEAR_VALUE dsvClearValue;
	dsvClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsvClearValue.DepthStencil.Depth = 1.0f;
	dsvClearValue.DepthStencil.Stencil = 0;

	auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC resourceDesc(
		D3D12_RESOURCE_DIMENSION_TEXTURE2D,
		0,
		m_FrameBufferWidth,
		m_FrameBufferHeight,
		1,
		1,
		DXGI_FORMAT_D32_FLOAT,
		1,
		0,
		D3D12_TEXTURE_LAYOUT_UNKNOWN,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE);
	hr = m_pDevice->CreateCommittedResource(
		&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsvClearValue,
		IID_PPV_ARGS(m_pDepthStencilBuffer.ReleaseAndGetAddressOf())
	);

	if (FAILED(hr))
	{
		return false;
	}

	//�f�B�X�N���v�^���쐬
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();

	m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), nullptr, dsvHandle);

	return true;
}

// ��
// �������I���
// 
// �`�揈���J�n
// ��

void Engine::BeginRender()
{
	// ���݂̃����_�[�^�[�Q�b�g���X�V
	m_currentRenderTarget = m_pRenderTargets[m_CurrentBackBufferIndex].Get();

	// �R�}���h�����������Ă��߂鏀��������
	m_pAllocator[m_CurrentBackBufferIndex]->Reset();
	m_pCommandList->Reset(m_pAllocator[m_CurrentBackBufferIndex].Get(), nullptr);

	// �r���[�|�[�g�ƃV�U�[��`��ݒ�
	m_pCommandList->RSSetViewports(1, &m_Viewport);
	m_pCommandList->RSSetScissorRects(1, &m_Scissor);

	// ���݂̃t���[���̃����_�[�^�[�Q�b�g�r���[�̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X���擾
	auto currentRtvHandle = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();
	currentRtvHandle.ptr += m_CurrentBackBufferIndex * m_RtvDescriptorSize;

	// �[�x�X�e���V���̃f�B�X�N���v�^�q�[�v�̊J�n�A�h���X�擾
	auto currentDsvHandle = m_pDsvHeap->GetCPUDescriptorHandleForHeapStart();

	// �����_�[�^�[�Q�b�g���g�p�\�ɂȂ�܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_currentRenderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g��ݒ�
	m_pCommandList->OMSetRenderTargets(1, &currentRtvHandle, FALSE, &currentDsvHandle);

	// �����_�[�^�[�Q�b�g���N���A
	const float clearColor[] = { 0.25f, 0.25f, 0.25f, 1.0f };
	m_pCommandList->ClearRenderTargetView(currentRtvHandle, clearColor, 0, nullptr);

	// �[�x�X�e���V���r���[���N���A
	m_pCommandList->ClearDepthStencilView(currentDsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}



void Engine::WaitRender()
{
	//�`��I���҂�
	const UINT64 fenceValue = m_fenceValue[m_CurrentBackBufferIndex];
	m_pQueue->Signal(m_pFence.Get(), fenceValue);
	m_fenceValue[m_CurrentBackBufferIndex]++;

	// ���̃t���[���̕`�揀�����܂��ł���Αҋ@����.
	if (m_pFence->GetCompletedValue() < fenceValue)
	{
		// �������ɃC�x���g��ݒ�.
		auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_fenceEvent);
		if (FAILED(hr))
		{
			return;
		}

		// �ҋ@����.
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE))
		{
			return;
		}
	}
}



void Engine::EndRender()
{
	// �����_�[�^�[�Q�b�g�ɏ������ݏI���܂ő҂�
	auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(m_currentRenderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_pCommandList->ResourceBarrier(1, &barrier);

	// �R�}���h�̋L�^���I��
	m_pCommandList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCommandList.Get() };
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// �X���b�v�`�F�[����؂�ւ�
	m_pSwapChain->Present(1, 0);

	// �`�抮����҂�
	WaitRender();

	// �o�b�N�o�b�t�@�ԍ��X�V
	m_CurrentBackBufferIndex = m_pSwapChain->GetCurrentBackBufferIndex();
}

//
////Getter�֐��Ƃ��Č��J����E���̃t�F�[�Y
//

ID3D12Device6* Engine::Device()
{
	return m_pDevice.Get();
}

ID3D12GraphicsCommandList* Engine::CommandList()
{
	return m_pCommandList.Get();
}

UINT Engine::CurrentBackBufferIndex()
{
	return m_CurrentBackBufferIndex;
}