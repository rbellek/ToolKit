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

    Material_Define
    (
      std::make_shared<ToolKit::Material>(),
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

  void MeshComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    if (!MeshC()->IsDynamic())
    {
      Component::Serialize(doc, parent);
    }
  }


  MaterialComponent::MaterialComponent()
  {
  }

  MaterialComponent::~MaterialComponent()
  {
  }

  void MaterialComponent::Init(bool flushClientSideArray)
  {
    Material()->Init(flushClientSideArray);
  }

  void MaterialComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    if (!MaterialC()->IsDynamic())
    {
      Component::Serialize(doc, parent);
    }
  }

}