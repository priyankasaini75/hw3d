#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ModelException.h"
#include "Node.h"
#include "Mesh.h"
#include "Material.h"
#include "ChiliXM.h"

namespace dx = DirectX;

Model::Model( Graphics& gfx,const std::string& pathString,const float scale )
{
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile( pathString.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ConvertToLeftHanded |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace
	);

	if( pScene == nullptr )
	{
		throw ModelException( __LINE__,__FILE__,imp.GetErrorString() );
	}

	// parse materials
	std::vector<Material> materials;
	materials.reserve( pScene->mNumMaterials );
	for( size_t i = 0; i < pScene->mNumMaterials; i++ )
	{
		materials.emplace_back( gfx,*pScene->mMaterials[i],pathString );
	}

	for( size_t i = 0; i < pScene->mNumMeshes; i++ )
	{
		const auto& mesh = *pScene->mMeshes[i];
		meshPtrs.push_back( std::make_unique<Mesh>( gfx,materials[mesh.mMaterialIndex],mesh,scale ) );
	}

	int nextId = 0;
	pRoot = ParseNode( nextId,*pScene->mRootNode,scale );
}

void Model::Submit( size_t channels ) const noxnd
{
	pRoot->Submit( channels,dx::XMMatrixIdentity() );
}

void Model::SetRootTransform( DirectX::FXMMATRIX tf ) noexcept
{
	pRoot->SetAppliedTransform( tf );
}

void Model::SetPos(float x, float y,float z) noexcept
{
	tf.x = x;
	tf.y = y;
	tf.z = z;
	pRoot->SetAppliedTransform(
		dx::XMMatrixTranslation(tf.x, tf.y, tf.z)
	);
}

void Model::UpdateTransform(float dt, const std::string& desiredAxis, float speed) noexcept
{

	// Check the provided axis and apply the corresponding rotation
	if (desiredAxis == "roll")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixRotationX(tf.xRot += speed * dt) *
			dx::XMMatrixRotationY(tf.yRot) *
			dx::XMMatrixRotationZ(tf.zRot)
		);
	}
	else if (desiredAxis == "pitch")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixRotationX(tf.xRot) *
			dx::XMMatrixRotationY(tf.yRot += speed * dt) *
			dx::XMMatrixRotationZ(tf.zRot)
		);
	}
	else if (desiredAxis == "yaw")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixRotationX(tf.xRot) *
			dx::XMMatrixRotationY(tf.yRot) *
			dx::XMMatrixRotationZ(tf.zRot += speed * dt)
		);
	}
	else if (desiredAxis == "all")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixRotationX(tf.xRot += speed * dt) *
			dx::XMMatrixRotationY(tf.yRot += speed * dt) *
			dx::XMMatrixRotationZ(tf.zRot += speed * dt)
		);
	}

	else if (desiredAxis == "x")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixTranslation(tf.x += speed * dt, tf.y, tf.z)
		);
	}
	else if (desiredAxis == "y")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixTranslation(tf.x, tf.y += speed * dt, tf.z)
		);
	}
	else if (desiredAxis == "z")
	{
		pRoot->SetAppliedTransform(
			dx::XMMatrixTranslation(tf.x, tf.y, tf.z += speed * dt)
		);
	}
}

void Model::Accept( ModelProbe & probe )
{
	pRoot->Accept( probe );
}

void Model::LinkTechniques( Rgph::RenderGraph& rg )
{
	for( auto& pMesh : meshPtrs )
	{
		pMesh->LinkTechniques( rg );
	}
}

Model::~Model() noexcept
{}

std::unique_ptr<Node> Model::ParseNode( int& nextId,const aiNode& node,float scale ) noexcept
{
	namespace dx = DirectX;
	const auto transform = ScaleTranslation( dx::XMMatrixTranspose( dx::XMLoadFloat4x4(
		reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
	) ),scale );

	std::vector<Mesh*> curMeshPtrs;
	curMeshPtrs.reserve( node.mNumMeshes );
	for( size_t i = 0; i < node.mNumMeshes; i++ )
	{
		const auto meshIdx = node.mMeshes[i];
		curMeshPtrs.push_back( meshPtrs.at( meshIdx ).get() );
	}

	auto pNode = std::make_unique<Node>( nextId++,node.mName.C_Str(),std::move( curMeshPtrs ),transform );
	for( size_t i = 0; i < node.mNumChildren; i++ )
	{
		pNode->AddChild( ParseNode( nextId,*node.mChildren[i],scale ) );
	}

	return pNode;
}
