#pragma once
#include <d3d12.h>
#include "ComPtr.h"

class VertexBuffer
{
public:
	VertexBuffer(size_t size, size_t stride, const void* pInitData);	//�R���X�g���N�^�Ńo�b�t�@�𐶐�
	D3D12_VERTEX_BUFFER_VIEW view() const;	//���_�o�b�t�@�[�r���[���擾
	bool IsValid();	//�o�b�t�@�̍쐬�ɐ������������擾

private:
	bool m_IsValid = false;	//�o�b�t�@�̐����ɐ������������擾
	ComPtr<ID3D12Resource> m_pBuffer = nullptr;	//�o�b�t�@�[
	D3D12_VERTEX_BUFFER_VIEW m_View = {};	//���_�o�b�t�@�r���[

	VertexBuffer(const VertexBuffer&) = delete;
	void operator = (const VertexBuffer&) = delete;
};
