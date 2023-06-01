#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct Mesh;
struct Vertex;

struct aiMesh;
struct aiMaterial;

struct ImportSettings
{
	const wchar_t* filename = nullptr;
	std::vector<Mesh>& meshes;
	bool inverseU = false;	//U���W�𔽓]�����邩
	bool inverseV = false;	//V���W�𔽓]�����邩
};

class AssimpLoader
{
public:
	bool Load(ImportSettings setting);	//���f�������[�h����

private:
	void LoadMesh(Mesh& dst, const aiMesh* sec, bool inverseU, bool inverseV);
	void LoadTexture(const wchar_t* filename, Mesh& dst, const aiMaterial* src);
};