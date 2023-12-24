#pragma once
#include "Graphics.h"
#include <string>
#include <memory>
#include <filesystem>

class Node;
class Mesh;
class ModelWindow;
struct aiMesh;
struct aiMaterial;
struct aiNode;

namespace Rgph
{
	class RenderGraph;
}

class Model
{
public:
	Model( Graphics& gfx,const std::string& pathString,float scale = 1.0f );
	void Submit( size_t channels ) const noxnd;
	void SetRootTransform( DirectX::FXMMATRIX tf ) noexcept;
	void SetPos(float x, float y, float z) noexcept;
	void UpdateTransform(float dt, const std::string& desiredAxis, float speed) noexcept;
	void Accept( class ModelProbe& probe );
	void LinkTechniques( Rgph::RenderGraph& );
	~Model() noexcept;
private:
	static std::unique_ptr<Mesh> ParseMesh( Graphics& gfx,const aiMesh& mesh,const aiMaterial* const* pMaterials,const std::filesystem::path& path,float scale );
	std::unique_ptr<Node> ParseNode( int& nextId,const aiNode& node,float scale ) noexcept;
private:
	std::unique_ptr<Node> pRoot;
	struct TransformParameters
	{
		float xRot = 0.0f;
		float yRot = 0.0f;
		float zRot = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};
	TransformParameters tf;
	// sharing meshes here perhaps dangerous?
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
};