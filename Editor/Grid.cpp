#include "Grid.h"
#include "GlobalDef.h"
#include "ToolKit.h"
#include "Primative.h"
#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    Grid::Grid(UVec2 size)
    {
      AddComponent(new MeshComponent());

      // Create grid material.
      if (!GetMaterialManager()->Exist(g_gridMaterialName))
      {
        m_material = GetMaterialManager()->GetCopyOfUnlitMaterial();
        m_material->UnInit();
        m_material->m_diffuseTexture = GetTextureManager()->Create<Texture>(
          TexturePath("grid.png"));
        m_material->GetRenderState()->blendFunction =
          BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
        m_material->GetRenderState()->cullMode = CullingType::TwoSided;
        m_material->Init();

        GetMaterialManager()->m_storage[g_gridMaterialName] = m_material;
      }
      else
      {
        m_material = GetMaterialManager()->Create<Material>(g_gridMaterialName);
      }

      // Create grid mesh.
      m_size.x = size.x % 2 == 0 ? size.x : size.x + 1;
      m_size.y = size.y % 2 == 0 ? size.y : size.y + 1;
      Resize(size);
    }

    ToolKit::Vec3 getColorOfAxis(unsigned char axisIndex, AxisLabel axisCombin)
    {
      // Choose axis color
      if (axisIndex == 0)
      {
        // Set axis colors
        switch (axisCombin)
        {
          case AxisLabel::XY:
          return g_gridAxisGreen;
          break;
          case AxisLabel::YZ:
          return g_gridAxisBlue;
          break;
          case AxisLabel::ZX:
          return g_gridAxisBlue;
          break;
        }
      }
      else
      {
        // Set axis colors
        switch (axisCombin)
        {
          case AxisLabel::XY:
          return g_gridAxisRed;
          break;
          case AxisLabel::YZ:
          return g_gridAxisGreen;
          break;
          case AxisLabel::ZX:
          return g_gridAxisRed;
          break;
        }
      }
    }

    void Grid::Resize(
      UVec2 size,
      AxisLabel axis,
      float gridSpaceScale,
      float mainAxisScale)
    {
      // size should be an even number
      for (uint8_t i = 0; i < 2; i++)
      {
        m_size[i] = size[i] % 2 == 0 ? size[i] : size[i] + 1;
      }

      bool axes[3] = { false, false, false };
      switch (axis)
      {
        case AxisLabel::XY:
        axes[0] = true; axes[1] = true;
        break;
        case AxisLabel::YZ:
        axes[1] = true; axes[2] = true;
        break;
        case AxisLabel::ZX:
        axes[0] = true; axes[2] = true;
        break;
        default:
        GetLogger()->WriteConsole(
          LogType::Error,
          "Grid::Resize has invalid x,y,z info; please report this!");
        return;
      }

      MeshPtr& parentMesh = GetMeshComponent()->Mesh();
      parentMesh->UnInit();
      glm::vec2 scale = glm::vec2(m_size) * glm::vec2(0.5f);

      Vec3Array offsets =
      {
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, scale.y * 0.5f + 0.025f),
        Vec3(scale.x * 0.5f + 0.025f, 0.0f, -scale.y * 0.5f - 0.025f),
        Vec3(-scale.x * 0.5f - 0.025f, 0.0f, -scale.y * 0.5f - 0.025f)
      };

      for (int i = 0; i < 4; i++)
      {
        Quad quad;
        MeshPtr& mesh = quad.GetMeshComponent()->Mesh();
        for (int j = 0; j < 4; j++)
        {
          ToolKit::Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos =
          (
            clientVertex.pos * glm::vec3(scale.x, scale.y, 0.0f)
          ).xzy + offsets[i];
          clientVertex.tex = clientVertex.pos.xz * gridSpaceScale;


          // Convert according to new axes
          Vec3 prevVertex = clientVertex.pos;
          bool isFirstOnePicked = false;
          for (uint8_t axisIndex = 0; axisIndex < 3; axisIndex++)
          {
            if (axes[axisIndex] == false)
            {
              clientVertex.pos[axisIndex] = prevVertex.y;
            }
            else if (!isFirstOnePicked)
            {
              clientVertex.pos[axisIndex] = prevVertex.x;
              isFirstOnePicked = true;
            }
            else
            {
              clientVertex.pos[axisIndex] = prevVertex.z;
            }
          }
        }

        mesh->m_material = m_material;
        if (i == 0)
        {
          GetMeshComponent()->Mesh() = mesh;
        }
        else
        {
          parentMesh->m_subMeshes.push_back(mesh);
        }
      }

      VertexArray vertices;
      vertices.resize(2);

      // x - z lines.
      Vec3 lines[2] = {
        Vec3(mainAxisScale / 20.0f, scale.y * 2.0f, 2.0f),
        Vec3(scale.x * 2.0f, mainAxisScale / 20.0f, 2.0f)
      };
      for (int i = 0; i < 2; i++)
      {
        MaterialPtr solidMat = GetMaterialManager()->
          GetCopyOfUnlitColorMaterial();
        solidMat->GetRenderState()->cullMode = CullingType::TwoSided;
        solidMat->m_color = g_gridAxisBlue;
        Vec3 ls = lines[i];

        solidMat->m_color = getColorOfAxis(i, axis);

        Quad quad;
        MeshPtr& mesh = quad.GetMeshComponent()->Mesh();
        for (int j = 0; j < 4; j++)
        {
          Vertex& clientVertex = mesh->m_clientSideVertices[j];
          clientVertex.pos = (clientVertex.pos * ls).xzy;

          // Convert according to new axes
          Vec3 prevVertex = clientVertex.pos;
          bool isFirstOnePicked = false;
          for (uint8_t axisIndex = 0; axisIndex < 3; axisIndex++)
          {
            if (axes[axisIndex] == false)
            {
              clientVertex.pos[axisIndex] = prevVertex.y;
            }
            else if (!isFirstOnePicked)
            {
              clientVertex.pos[axisIndex] = prevVertex.x;
              isFirstOnePicked = true;
            }
            else
            {
              clientVertex.pos[axisIndex] = prevVertex.z;
            }
          }
        }
        mesh->m_material = solidMat;
        parentMesh->m_subMeshes.push_back(mesh);
      }


      parentMesh->CalculateAABB();
    }

    bool Grid::HitTest(const Ray& ray, Vec3& pos)
    {
      Mat4 ts = m_node->GetTransform(TransformationSpace::TS_WORLD);
      Mat4 its = glm::inverse(ts);
      Ray rayInObjectSpace =
      {
        its * Vec4(ray.position, 1.0f),
        its * Vec4(ray.direction, 0.0f)
      };

      float dist = 0.0f;
      if (RayBoxIntersection(rayInObjectSpace, GetAABB(), dist))
      {
        pos = PointOnRay(rayInObjectSpace, dist);
        return true;
      }

      return false;
    }
  }  //  namespace Editor
}  //  namespace ToolKit
