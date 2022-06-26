#pragma once

#include "Component.h"

namespace ToolKit
{

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;
  typedef std::vector<MeshComponentPtr> MeshComponentPtrArray;
  static VariantCategory MeshComponentCategory
  {
    "Mesh Component",
    90
  };

  class TK_API MeshComponent : public Component
  {
    public:
    /**
    * Auto generated code for type information.
    */
    TKComponentType(MeshComponent);

    /**
    * Empty constructor.
    */
    MeshComponent();

    /**
    * Empty destructor.
    */
    virtual ~MeshComponent();

    /**
    * Gets the bounding box of the contained Mesh.
    * @return BoundingBox of the contained Mesh.
    */
    BoundingBox GetAABB();

    /**
    * Initiates the MeshComponent and underlying Mesh and Material resources.
    */
    void Init(bool flushClientSideArray);

    public:
    TKDeclareParam(MeshPtr, Mesh);  //!< Component's Mesh resource.
  };

  typedef std::shared_ptr<class MaterialComponent> MaterialComponentPtr;
  static VariantCategory MaterialComponentCategory
  {
    "Material Component",
    90
  };

  class TK_API MaterialComponent : public Component
  {   
   public:
    TKComponentType(MaterialComponent);

    MaterialComponent();
    virtual ~MaterialComponent();
    void Init(bool flushClientSideArray);

   public:
    /**
    * Component's material resource. In case this object is not null, Renderer
    * picks this material to render the mesh otherwise falls back to Material
    * within the Mesh.
    */
    TKDeclareParam(MaterialPtr, Material);
  };

}
