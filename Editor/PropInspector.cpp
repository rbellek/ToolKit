#include "PropInspector.h"
#include "GlobalDef.h"
#include "Util.h"

#include "ImGui/imgui_stdlib.h"
#include "ConsoleWindow.h"
#include "TransformMod.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    // View
    //////////////////////////////////////////////////////////////////////////

    void View::ShowVariant(ParameterVariant* var)
    {
      if (!var->m_exposed)
      {
        return;
      }

      ImGui::BeginDisabled(!var->m_editable);

      switch (var->GetType())
      {
        case ParameterVariant::VariantType::Bool:
        {
          ImGui::Checkbox(var->m_name.c_str(), var->GetVarPtr<bool>());
        }
        break;
        case ParameterVariant::VariantType::Float:
        {
          ImGui::InputFloat(var->m_name.c_str(), var->GetVarPtr<float>());
        }
        break;
        case ParameterVariant::VariantType::Int:
        {
          ImGui::InputInt(var->m_name.c_str(), var->GetVarPtr<int>());
        }
        break;
        case ParameterVariant::VariantType::Vec3:
        {
          ImGui::InputFloat3(var->m_name.c_str(), &var->GetVar<Vec3>()[0]);
        }
        break;
        case ParameterVariant::VariantType::Vec4:
        {
          ImGui::InputFloat4(var->m_name.c_str(), &var->GetVar<Vec4>()[0]);
        }
        break;
        case ParameterVariant::VariantType::String:
        {
          ImGui::InputText(var->m_name.c_str(), var->GetVarPtr<String>());
        }
        break;
        case ParameterVariant::VariantType::ULongID:
        {
          ImGui::InputScalar
          (
            var->m_name.c_str(),
            ImGuiDataType_U32,
            var->GetVarPtr<ULongID>()
          );
        }
        break;
      default:
        break;
      }

      ImGui::EndDisabled();
    }

    void View::DropZone
    (
      uint fallbackIcon,
      const String& file,
      std::function<void(const DirectoryEntry& entry)> dropAction,
      const String& dropName
    )
    {
      DirectoryEntry dirEnt;
      bool fileExist = g_app->GetAssetBrowser()->GetFileEntry(file, dirEnt);
      uint iconId = fallbackIcon;

      ImVec2 texCoords = ImVec2(1.0f, 1.0f);
      if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
      {
        texCoords = ImVec2(1.0f, -1.0f);
        iconId = thumb->m_textureId;
      }
      else if (fileExist)
      {
        dirEnt.GenerateThumbnail();

        if (RenderTargetPtr thumb = dirEnt.GetThumbnail())
        {
          iconId = thumb->m_textureId;
        }
      }

      if (ImGui::BeginTable("##DropZone", 2))
      {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();

        if (!dropName.empty())
        {
          ImGui::Text(dropName.c_str());
        }

        ImGui::Image
        (
          reinterpret_cast<void*>((intptr_t)iconId),
          ImVec2(48.0f, 48.0f),
          ImVec2(0.0f, 0.0f),
          texCoords
        );

        if (ImGui::BeginDragDropTarget())
        {
          if
          (
            const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload("BrowserDragZone")
          )
          {
            IM_ASSERT(payload->DataSize == sizeof(DirectoryEntry));
            DirectoryEntry entry = *(const DirectoryEntry*)payload->Data;
            dropAction(entry);
          }

          ImGui::EndDragDropTarget();
        }

        UI::HelpMarker(TKLoc + file, "Drop zone", 0.1f);
        ImGui::TableNextColumn();

        String fullPath = dirEnt.GetFullPath();
        ImGui::Text("%s", fullPath.c_str());
        UI::HelpMarker(TKLoc + file, fullPath.c_str(), 0.1f);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TableNextColumn();

        if (ImGui::Button("Reload"))
        {
          if (ResourceManager* man = dirEnt.GetManager())
          {
            if (man->Exist(file))
            {
              man->m_storage[file]->Reload();
              man->m_storage[file]->Init(false);
            }
          }
        }

        ImGui::EndTable();
      }
    }

    void View::DropSubZone
    (
      uint fallbackIcon,
      const String& file,
      std::function<void(const DirectoryEntry& entry)> dropAction
    )
    {
      String uid = "Resource##" + std::to_string(m_viewID);
      if (ImGui::TreeNode(uid.c_str()))
      {
        DropZone(fallbackIcon, file, dropAction);

        ImGui::TreePop();
      }
    }

    // EntityView
    //////////////////////////////////////////////////////////////////////////

    void EntityView::Show()
    {
      if (m_entity == nullptr)
      {
        return;
      }

      ShowVariants();

      // Missing data reporter.
      if (m_entity->IsDrawable())
      {
        Drawable* dw = static_cast<Drawable*> (m_entity);
        MeshPtr& mesh = dw->GetMesh();

        StringArray missingData;
        MeshRawCPtrArray meshes;
        mesh->GetAllMeshes(meshes);

        for (const Mesh* subMesh : meshes)
        {
          if (!subMesh->_missingFile.empty())
          {
            missingData.push_back(subMesh->_missingFile);
          }

          if (MaterialPtr mat = subMesh->m_material)
          {
            if (!mat->_missingFile.empty())
            {
              missingData.push_back(mat->_missingFile);
            }

            if (TexturePtr tex = mat->m_diffuseTexture)
            {
              if (!tex->_missingFile.empty())
              {
                missingData.push_back(tex->_missingFile);
              }
            }
          }
        }

        if (!missingData.empty())
        {
          ImGui::Text("Missing Data: ");
          ImGui::Separator();
          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
          for (const String& data : missingData)
          {
            ImGui::Text(data.c_str());
          }
          ImGui::PopStyleColor();
        }
      }

      if
      (
        ImGui::CollapsingHeader("Transforms", ImGuiTreeNodeFlags_DefaultOpen)
      )
      {
        Mat3 rotate;
        Vec3 scale, shear;
        Mat4 ts = m_entity->m_node->GetTransform(g_app->m_transformSpace);
        QDUDecomposition(ts, rotate, scale, shear);

        // Continous edit utils.
        static TransformAction* dragMem = nullptr;
        const auto saveDragMemFn = [this]() -> void
        {
          if (dragMem == nullptr)
          {
            dragMem = new TransformAction(m_entity);
          }
        };

        const auto saveTransformActionFn = [this]() -> void
        {
          if (ImGui::IsItemDeactivatedAfterEdit())
          {
            ActionManager::GetInstance()->AddAction(dragMem);
            dragMem = nullptr;
          }
        };

        TransformationSpace space = g_app->m_transformSpace;
        Vec3 translate = glm::column(ts, 3);
        if (ImGui::DragFloat3("Translate", &translate[0], 0.25f))
        {
          saveDragMemFn();
          m_entity->m_node->SetTranslation(translate, space);
        }

        saveTransformActionFn();

        Quaternion q0 = glm::toQuat(rotate);
        Vec3 eularXYZ = glm::eulerAngles(q0);
        Vec3 degrees = glm::degrees(eularXYZ);
        if (ImGui::DragFloat3("Rotate", &degrees[0], 0.25f))
        {
          saveDragMemFn();

          Vec3 eular = glm::radians(degrees);
          Vec3 change = eular - eularXYZ;

          bool isDrag = ImGui::IsMouseDragging(0, 0.25f);
          if (!isDrag)
          {
            change = eular;
          }

          Quaternion qx = glm::angleAxis(change.x, X_AXIS);
          Quaternion qy = glm::angleAxis(change.y, Y_AXIS);
          Quaternion qz = glm::angleAxis(change.z, Z_AXIS);
          Quaternion q = qz * qy * qx;

          if (isDrag)
          {
            m_entity->m_node->Rotate(q, space);
          }
          else
          {
            m_entity->m_node->SetOrientation(q, space);
          }
        }

        saveTransformActionFn();

        scale = m_entity->m_node->GetScale();
        if (ImGui::DragFloat3("Scale", &scale[0], 0.25f))
        {
          saveDragMemFn();
          m_entity->m_node->SetScale(scale);
        }

        saveTransformActionFn();

        if
        (
          ImGui::Checkbox("Inherit Scale", &m_entity->m_node->m_inheritScale)
        )
        {
          m_entity->m_node->SetInheritScaleDeep
          (
            m_entity->m_node->m_inheritScale
          );
        }

        ImGui::Separator();

        BoundingBox bb = m_entity->GetAABB(true);
        Vec3 dim = bb.max - bb.min;
        ImGui::Text("Bounding box dimentions:");
        ImGui::Text("x: %.2f", dim.x);
        ImGui::SameLine();
        ImGui::Text("\ty: %.2f", dim.y);
        ImGui::SameLine();
        ImGui::Text("\tz: %.2f", dim.z);
      }

      ShowCustomData();
    }

    void EntityView::ShowVariants()
    {
      ParameterBlock& localData = m_entity->m_localData;

      VariantCategoryArray categories;
      localData.GetCategories(categories, true);

      for (VariantCategory& category : categories)
      {
        if (category.Name == CustomDataCategory.Name)
        {
          continue;
        }

        if
        (
          ImGui::CollapsingHeader
          (
            category.Name.c_str(),
            ImGuiTreeNodeFlags_DefaultOpen
          )
        )
        {
          ParameterVariantRawPtrArray vars;
          localData.GetByCategory(category.Name, vars);

          for (ParameterVariant* var : vars)
          {
            ShowVariant(var);
          }
        }
      }
    }

    void EntityView::ShowCustomData()
    {
      if
      (
        ImGui::CollapsingHeader("Custom Data", ImGuiTreeNodeFlags_DefaultOpen)
      )
      {
        if
        (
          ImGui::BeginTable
          (
            "##CustomData",
            3,
            ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedSame
          )
        )
        {
          Vec2 xSize = ImGui::CalcTextSize("Name");
          xSize *= 3.0f;
          ImGui::TableSetupColumn
          (
            "Name",
            ImGuiTableColumnFlags_WidthFixed,
            xSize.x
          );
          ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);

          xSize = ImGui::CalcTextSize("X");
          xSize *= 2.5f;
          ImGui::TableSetupColumn
          (
            "##Remove",
            ImGuiTableColumnFlags_WidthFixed,
            xSize.x
          );

          ImGui::TableHeadersRow();

          ImGui::TableSetColumnIndex(0);
          ImGui::PushItemWidth(-FLT_MIN);

          ImGui::TableSetColumnIndex(1);
          ImGui::PushItemWidth(-FLT_MIN);

          ParameterVariantRawPtrArray customParams;
          m_entity->m_localData.GetByCategory
          (
            CustomDataCategory.Name,
            customParams
          );

          ParameterVariant* remove = nullptr;
          for (size_t i = 0; i < customParams.size(); i++)
          {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);

            ImGui::PushID(static_cast<int>(i));
            ParameterVariant* var = customParams[i];
            static char buff[1024];
            strcpy_s(buff, sizeof(buff), var->m_name.c_str());

            String pNameId = "##Name" + std::to_string(i);
            ImGui::InputText(pNameId.c_str(), buff, sizeof(buff));
            var->m_name = buff;

            ImGui::TableSetColumnIndex(1);

            String pId = "##" + std::to_string(i);
            switch (var->GetType())
            {
            case ParameterVariant::VariantType::String:
            {
              ImGui::InputText(pId.c_str(), var->GetVarPtr<String>());
            }
            break;
            case ParameterVariant::VariantType::Bool:
            {
              ImGui::Checkbox(pId.c_str(), var->GetVarPtr<bool>());
            }
            break;
            case ParameterVariant::VariantType::Int:
            {
              ImGui::InputInt(pId.c_str(), var->GetVarPtr<int>());
            }
            break;
            case ParameterVariant::VariantType::Float:
            {
              ImGui::InputFloat(pId.c_str(), var->GetVarPtr<float>());
            }
            break;
            case ParameterVariant::VariantType::Vec3:
            {
              ImGui::InputFloat3(pId.c_str(), &var->GetVar<Vec3>()[0]);
            }
            break;
            case ParameterVariant::VariantType::Vec4:
            {
              ImGui::InputFloat4(pId.c_str(), &var->GetVar<Vec4>()[0]);
            }
            break;
            case ParameterVariant::VariantType::Mat3:
            {
              Vec3 vec;
              Mat3 val = var->GetVar<Mat3>();
              for (int j = 0; j < 3; j++)
              {
                pId += std::to_string(j);
                vec = glm::row(val, j);
                ImGui::InputFloat3(pId.c_str(), &vec[0]);
                val = glm::row(val, j, vec);
                *var = val;
              }
            }
            break;
            case ParameterVariant::VariantType::Mat4:
            {
              Vec4 vec;
              Mat4 val = var->GetVar<Mat4>();
              for (int j = 0; j < 4; j++)
              {
                pId += std::to_string(j);
                vec = glm::row(val, j);
                ImGui::InputFloat4(pId.c_str(), &vec[0]);
                val = glm::row(val, j, vec);
                *var = val;
              }
            }
            break;
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button("X"))
            {
              remove = customParams[i];
              g_app->m_statusMsg =
              Format("Parameter %d: %s removed.", i + 1, var->m_name.c_str());
            }

            ImGui::PopID();
          }

          if (remove != nullptr)
          {
            m_entity->m_localData.Remove(remove->m_id);
          }

          ImGui::EndTable();
          ImGui::Separator();

          ImGui::PushItemWidth(150);
          static bool addInAction = false;
          if (addInAction)
          {
            int dataType = 0;
            if
            (
              ImGui::Combo
              (
                "##NewCustData",
                &dataType,
                "...\0String\0Boolean\0Int\0Float\0Vec3\0Vec4\0Mat3\0Mat4")
              )
            {
              ParameterVariant customVar;
              // This makes them only visible in Custom Data dropdown.
              customVar.m_exposed = false;
              customVar.m_editable = true;
              customVar.m_category = CustomDataCategory;

              bool added = true;
              switch (dataType)
              {
              case 1:
                customVar = "";
                break;
              case 2:
                customVar = false;
                break;
              case 3:
                customVar = 0;
                break;
              case 4:
                customVar = 0.0f;
                break;
              case 5:
                customVar = ZERO;
                break;
              case 6:
                customVar = Vec4();
                break;
              case 7:
                customVar = Mat3();
                break;
              case 8:
                customVar = Mat4();
                break;
              default:
                added = false;
                break;
              }

              if (added)
              {
                m_entity->m_localData.Add(customVar);
                addInAction = false;
              }
            }
          }
          ImGui::PopItemWidth();

          Vec2 min = ImGui::GetWindowContentRegionMin();
          Vec2 max = ImGui::GetWindowContentRegionMax();
          Vec2 size = max - min;

          ImGui::AlignTextToFramePadding();
          ImVec2 tsize = ImGui::CalcTextSize("Add Custom Data");
          float offset = (size.x - tsize.x) * 0.5f;
          ImGui::Indent(offset);

          if (ImGui::Button("Add Custom Data"))
          {
            addInAction = true;
          }

          ImGui::Indent(-offset);
        }
      }
    }

    // MeshView
    //////////////////////////////////////////////////////////////////////////

    void MeshView::Show()
    {
      MeshPtr& entry = static_cast<Drawable*> (m_entity)->GetMesh();
      if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if (ImGui::BeginTable("##MeshStats", 2))
        {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text("Face count:");
          ImGui::TableNextColumn();
          ImGui::Text("%d", (uint)entry->m_faces.size());

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text("Vertex count:");
          ImGui::TableNextColumn();
          ImGui::Text("%d", (uint)entry->m_clientSideVertices.size());

          ImGui::EndTable();
        }

        DropSubZone
        (
          UI::m_meshIcon->m_textureId,
          entry->GetFile(),
          [this](const DirectoryEntry& dirEnt) -> void
          {
            if (m_entity && m_entity->IsDrawable())
            {
              Drawable* dw = static_cast<Drawable*> (m_entity);
              dw->SetMesh
              (
                GetResourceManager(ResourceType::Mesh)->Create<Mesh>
                (
                  dirEnt.GetFullPath()
                )
              );
            }
          }
        );
      }
    }

    // MaterialView
    //////////////////////////////////////////////////////////////////////////

    void MaterialView::Show()
    {
      Drawable* drawable = static_cast<Drawable*> (m_entity);
      MaterialPtr entry;

      bool entityMod = true;
      if ((entry = m_material))
      {
        entityMod = false;
      }
      else
      {
        if (drawable == nullptr)
        {
          return;
        }
        entry = drawable->GetMesh()->m_material;
      }

      auto updateThumbFn = [&entry]() -> void
      {
        DirectoryEntry dirEnt(entry->GetFile());
        dirEnt.GenerateThumbnail();
        entry->m_dirty = true;
      };

      if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
      {
        Vec4 col = Vec4(entry->m_color, entry->m_alpha);
        if (ImGui::ColorEdit4("MatColor##1", &col.x))
        {
          entry->m_color = col.xyz;
          entry->m_alpha = col.a;
          updateThumbFn();
        }

        if (ImGui::TreeNode("Textures"))
        {
          ImGui::LabelText("##diffTexture", "Diffuse Texture: ");
          String target = GetPathSeparatorAsStr();
          if (entry->m_diffuseTexture)
          {
            target = entry->m_diffuseTexture->GetFile();
          }

          DropZone
          (
            UI::m_imageIcon->m_textureId,
            target,
            [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
            {
              // Switch from solid color material to default for texturing.
              if (entry->m_diffuseTexture == nullptr)
              {
                entry->m_fragmetShader =
                GetShaderManager()->Create<Shader>
                (
                  ShaderPath("defaultFragment.shader", true)
                );
                entry->m_fragmetShader->Init();
              }
              entry->m_diffuseTexture =
              GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
              entry->m_diffuseTexture->Init();
              updateThumbFn();
            }
          );

          ImGui::TreePop();
        }

        if (ImGui::TreeNode("Shaders"))
        {
          ImGui::LabelText("##vertShader", "Vertex Shader: ");
          DropZone
          (
            UI::m_codeIcon->m_textureId,
            entry->m_vertexShader->GetFile(),
            [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
            {
              if (strcmp(dirEnt.m_ext.c_str(), ".shader") != 0)
              {
                g_app->m_statusMsg = "An imported shader file expected!";
                return;
              }
              entry->m_vertexShader =
              GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
              entry->m_vertexShader->Init();
              updateThumbFn();
            }
          );

          ImGui::LabelText("##fragShader", "Fragment Shader: ");
          DropZone
          (
            UI::m_codeIcon->m_textureId,
            entry->m_fragmetShader->GetFile(),
            [&entry, &updateThumbFn](const DirectoryEntry& dirEnt) -> void
            {
              entry->m_fragmetShader =
              GetShaderManager()->Create<Shader>(dirEnt.GetFullPath());
              entry->m_fragmetShader->Init();
              updateThumbFn();
            }
          );
          ImGui::TreePop();
        }

        if (ImGui::TreeNode("Render State"))
        {
          int cullMode = static_cast<int>(entry->GetRenderState()->cullMode);
          if (ImGui::Combo("Cull mode", &cullMode, "Two Sided\0Front\0Back"))
          {
            entry->GetRenderState()->cullMode = (CullingType)cullMode;
            entry->m_dirty = true;
          }

          int blendMode =
          static_cast<int>(entry->GetRenderState()->blendFunction);
          if (ImGui::Combo("Blend mode", &blendMode, "None\0Alpha Blending"))
          {
            entry->GetRenderState()->blendFunction = (BlendFunction)blendMode;
            entry->m_dirty = true;
          }

          int drawType = -1;
          switch (entry->GetRenderState()->drawType)
          {
          case DrawType::Triangle:
            drawType = 0;
            break;
          case DrawType::Line:
            drawType = 1;
            break;
          case DrawType::LineStrip:
            drawType = 2;
            break;
          case DrawType::LineLoop:
            drawType = 3;
            break;
          case DrawType::Point:
            drawType = 4;
            break;
          }

          if
          (
            ImGui::Combo
            (
              "Draw mode",
              &drawType,
              "Triangle\0Line\0Line Strip\0Line Loop\0Point"
            )
          )
          {
            switch (drawType)
            {
            case 0:
              entry->GetRenderState()->drawType = DrawType::Triangle;
              break;
            case 1:
              entry->GetRenderState()->drawType = DrawType::Line;
              break;
            case 2:
              entry->GetRenderState()->drawType = DrawType::LineStrip;
              break;
            case 3:
              entry->GetRenderState()->drawType = DrawType::LineLoop;
              break;
            case 4:
              entry->GetRenderState()->drawType = DrawType::Point;
              break;
            }

            entry->m_dirty = true;
          }

          bool depthTest = entry->GetRenderState()->depthTestEnabled;
          if (ImGui::Checkbox("Enable depth test", &depthTest))
          {
            entry->GetRenderState()->depthTestEnabled = depthTest;
            entry->m_dirty = true;
          }

          ImGui::TreePop();
        }

        if (entityMod)
        {
          DropSubZone
          (
            UI::m_materialIcon->m_textureId,
            entry->GetFile(),
            [&drawable](const DirectoryEntry& dirEnt) -> void
            {
              MeshPtr& mesh = drawable->GetMesh();
              if (strcmp(dirEnt.m_ext.c_str(), ".material") != 0)
              {
                g_app->m_statusMsg = "An imported material file expected!";
                return;
              }
              mesh->m_material =
              GetMaterialManager()->Create<Material>(dirEnt.GetFullPath());
              mesh->m_material->Init();
              mesh->m_dirty = true;
            }
          );
        }
      }
    }

    // PropInspector
    //////////////////////////////////////////////////////////////////////////

    PropInspector::PropInspector(XmlNode* node)
      : PropInspector()
    {
      DeSerialize(nullptr, node);
    }

    PropInspector::PropInspector()
    {
      m_views.push_back(new EntityView());
      m_views.push_back(new MaterialView());
      m_views.push_back(new MeshView());
      m_views.push_back(new SurfaceView());
    }

    PropInspector::~PropInspector()
    {
      for (View* v : m_views)
      {
        SafeDel(v);
      }
      m_views.clear();
    }

    void PropInspector::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        Entity* curr = g_app->GetCurrentScene()->GetCurrentSelection();
        if (curr == nullptr)
        {
          ImGui::Text("Select an entity");
        }
        else
        {
          EntityView* ev = GetView<EntityView>();
          ev->m_entity = curr;
          ev->Show();

          if (curr->IsDrawable())
          {
            MeshView* mev = GetView<MeshView>();
            mev->m_entity = curr;
            mev->Show();

            MaterialView* mav = GetView <MaterialView>();
            mav->m_entity = curr;
            mav->Show();
          }

          if (curr->IsSurfaceInstance())
          {
            View* view = GetView<SurfaceView>();
            view->m_entity = curr;
            view->Show();
          }
        }
      }
      ImGui::End();
    }

    Window::Type PropInspector::GetType() const
    {
      return Window::Type::Inspector;
    }

    void PropInspector::DispatchSignals() const
    {
      ModShortCutSignals
      (
        {
          SDL_SCANCODE_DELETE,
          SDL_SCANCODE_D,
          SDL_SCANCODE_F,
          SDL_SCANCODE_R,
          SDL_SCANCODE_G,
          SDL_SCANCODE_B,
          SDL_SCANCODE_S
        }
      );
    }

    template<typename T>
    T* PropInspector::GetView()
    {
      for (View* v : m_views)
      {
        if (T* cv = dynamic_cast<T*> (v))
        {
          return cv;
        }
      }

      assert(false && "Invalid View type queried");
      return nullptr;
    }

    MaterialInspector::MaterialInspector(XmlNode* node)
      : MaterialInspector()
    {
      DeSerialize(nullptr, node);
    }

    MaterialInspector::MaterialInspector()
    {
      m_view = new MaterialView();
    }

    MaterialInspector::~MaterialInspector()
    {
      SafeDel(m_view);
    }

    void MaterialInspector::Show()
    {
      if (ImGui::Begin(m_name.c_str(), &m_visible))
      {
        HandleStates();

        if (m_material == nullptr)
        {
          ImGui::Text("Select a material");
        }
        else
        {
          m_view->m_material = m_material;
          m_view->Show();
        }
      }
      ImGui::End();
    }

    Window::Type MaterialInspector::GetType() const
    {
      return Type::MaterialInspector;
    }

    void MaterialInspector::DispatchSignals() const
    {
      ModShortCutSignals({ SDL_SCANCODE_DELETE });
    }

    void SurfaceView::Show()
    {
      if (!m_entity->IsSurfaceInstance())
      {
        return;
      }

      Surface* entry = static_cast<Surface*> (m_entity);
      if (ImGui::CollapsingHeader("Surface", ImGuiTreeNodeFlags_DefaultOpen))
      {
        if
        (
          ImGui::BeginTable
          (
            "##SurfaceProps",
            2,
            ImGuiTableFlags_SizingFixedSame
          )
        )
        {
          ImGui::TableSetupColumn("##size");
          ImGui::TableSetupColumn
          (
            "##offset",
            ImGuiTableColumnFlags_WidthStretch
          );

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text("Size: ");
          ImGui::TableNextColumn();

          ImGui::InputFloat2
          (
            "##Size",
            reinterpret_cast<float*>(&entry->m_size)
          );
          if (ImGui::IsItemDeactivatedAfterEdit())
          {
            entry->UpdateGeometry(false);
          }

          ImGui::TableNextRow();
          ImGui::TableNextColumn();

          ImGui::Text("Offset: ");
          ImGui::TableNextColumn();

          ImGui::InputFloat2
          (
            "##Offset",
            reinterpret_cast<float*>(&entry->m_pivotOffset)
          );
          if (ImGui::IsItemDeactivatedAfterEdit())
          {
            entry->UpdateGeometry(false);
          }

          ImGui::EndTable();

          const char* text = "Update Size By Texture";
          if (ImGui::Button(text))
          {
            entry->UpdateGeometry(true);
          }

          // Show additions.
          ShowButton();
        }
      }
    }

    void SurfaceView::ShowButton()
    {
      if (m_entity->GetType() == EntityType::Entity_Button)
      {
        Button* button = dynamic_cast<Button*> (m_entity);
        ImGui::Text("Button");
        ImGui::Separator();

        String file = "\\button image.";
        if (button->m_buttonImage)
        {
          file = button->m_buttonImage->GetFile();
        }

        DropZone
        (
          UI::m_imageIcon->m_textureId,
          file,
          [this](const DirectoryEntry& dirEnt) -> void
          {
            if (m_entity && m_entity->IsDrawable())
            {
              if (m_entity->GetType() == EntityType::Entity_Button)
              {
                Button* button = dynamic_cast<Button*> (m_entity);
                TexturePtr texture =
                GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
                texture->Init(false);
                button->m_buttonImage = texture;
                button->GetMesh()->m_material->m_diffuseTexture = texture;
                button->UpdateGeometry(true);
              }
            }
          },
          "Button Image:"
        );

        file = "\\mouse over image.";
        if (button->m_mouseOverImage)
        {
          file = button->m_mouseOverImage->GetFile();
        }

        DropZone
        (
          UI::m_imageIcon->m_textureId,
          file,
          [this](const DirectoryEntry& dirEnt) -> void
          {
            if (m_entity && m_entity->IsDrawable())
            {
              if (m_entity->GetType() == EntityType::Entity_Button)
              {
                Button* button = dynamic_cast<Button*> (m_entity);
                TexturePtr texture =
                GetTextureManager()->Create<Texture>(dirEnt.GetFullPath());
                texture->Init(false);
                button->m_mouseOverImage = texture;
              }
            }
          },
          "Mouse Hover Image:"
        );
      }
    }

  }  // namespace Editor
}  // namespace ToolKit

