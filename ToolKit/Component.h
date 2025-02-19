#pragma once

/**
 * @file Component.h Header file for base Component classes
 * and related structures.
 */

#include <memory>

#include "ParameterBlock.h"
#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  /**
   * @def TKComponentType(type) The macro is responsible for
   * responsible for auto generating component type information.
   */
  #define TKComponentType(type) \
  static ComponentType GetTypeStatic() { return ComponentType::type; } \
  ComponentType GetType() const override { return ComponentType::type; }

  /**
   * Enums for component types.
   */
  enum class ComponentType
  {
    Base,
    MeshComponent,
    DirectionComponent
  };

  /**
   * Base component class that represent data which can be added and queried
   * by entities. Components are responsible bringing in related functionality
   * to the attached Entity classes.
   */
  class TK_API Component : public Serializable
  {
   public:
    /**
     * Default constructor. It initializes a unique id that is not obtained
     * during the current runtime.
     */
    Component();

    /**
     * Empty destructor.
     */
    virtual ~Component();

    /**
     * Base function for type informations. Derived classes expected to use
     * TKComponentType macro to override this function.
     * @return ComponentType of the class.
     */
    virtual ComponentType GetType() const;

    /**
     * Creates a copy of the component. Derived classes should perform
     * appropriate cloning functionality that creates actual memory copy of
     * the component. It should not be an instance.
     * Base class take care of cloning ParameterBlock of the Component.
     * @return Copy of the component.
     */
    virtual ComponentPtr Copy() = 0;

    /**
     * Serializes the Component's ParameterBlock to the xml document.
     * If parent is not null appends this component to the parent node.
     */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

    /**
     * De serialize the Component's ParameterBlock from given xml node
     * and document.
     */
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    static Component* CreateByType(ComponentType t);

   public:
    ULongID m_id;  //!< Unique id of the component for the current runtime.
    ParameterBlock m_localData;  //!< Component local data.
    Entity* m_entity = nullptr;
  };

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;

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
     * Creates a copy of the MeshComponent. Contained Mesh does not get
     * copied but referenced. However Material is copied and will be serialized
     * to the scene if the containing Entity gets serialized.
     * @return Copy of the MeshComponent.
     */
    ComponentPtr Copy() override;

    /**
     * Gets the bounding box of the contained Mesh.
     * @return BoundingBox of the contained Mesh.
     */
    BoundingBox GetAABB();

    /**
     * Initiates the MeshComponent and underlying Mesh and Material resources.
     */
    void Init(bool flushClientSideArray);

    /**
    * Overrides default serialization to avoid saving dynamically created
    * resources. Because dynamic resources are created from scratch
    * in every runtime.
    */
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(MeshPtr, Mesh);  //!< Component's Mesh resource.

    /**
     * Component's material resource. In case this object is not null, Renderer
     * picks this material to render the mesh otherwise falls back to Material
     * within the Mesh.
     */
    TKDeclareParam(MaterialPtr, Material);
  };

  typedef std::shared_ptr<class DirectionComponent> DirectionComponentPtr;

  static VariantCategory DirectionComponentCategory
  {
    "Direction Component",
    10
  };

  class TK_API DirectionComponent: public Component
  {
   public:
    TKComponentType(DirectionComponent);

    DirectionComponent();
    explicit DirectionComponent(Entity* entity);
    virtual ~DirectionComponent();

    ComponentPtr Copy() override;

    // Directional functions
    Vec3 GetDirection();
    void Pitch(float angle);
    void Yaw(float angle);
    void Roll(float angle);
    void RotateOnUpVector(float angle);
    Vec3 GetUp() const;
    Vec3 GetRight() const;
    void LookAt(Vec3 target);
  };

}  // namespace ToolKit
