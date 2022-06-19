#include "ResourceComponent.h"
#include "Mesh.h"
#include "Material.h"
#include "Entity.h"

namespace ToolKit
{

  MeshComponent::MeshComponent()
  {
    Mesh_Define
    (
      std::make_shared<ToolKit::Mesh>(),
      MeshComponentCategory.Name,
      MeshComponentCategory.Priority,
      true,
      true
    );
  }

  MeshComponent::~MeshComponent()
  {
  }

  BoundingBox MeshComponent::GetAABB()
  {
    return Mesh()->m_aabb;
  }

  void MeshComponent::Init(bool flushClientSideArray)
  {
    Mesh()->Init(flushClientSideArray);
  }

  MaterialComponent::MaterialComponent()
  {
    Material_Define
    (
      std::make_shared<ToolKit::Material>(),
      MaterialComponentCategory.Name,
      MaterialComponentCategory.Priority,
      true,
      true
    );
  }

  MaterialComponent::~MaterialComponent()
  {
  }

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    Material()->Init(flushClientSideArray);
  }

}