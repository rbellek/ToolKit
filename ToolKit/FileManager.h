#pragma once

#include <filesystem>

#include "zip.h"

#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    void PackResources(const String& path);

   private:
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);
    void CreatePackResources(const String& path);
    void CreatePackDirectories();
    
    bool CompressPack();
    bool AddFileToZip(zipFile zfile, const char* filename);
    
    void GetAnimationPaths(const String& path);
    void GetScenePaths(const String& path);

    void CopyFontResourcesToPack();
    void CopyMaterialResourcesToPack();
    void CopyMeshResourcesToPack();
    void CopyShaderResourcesToPack();
    void CopyTextureResourcesToPack();
    void CopyAnimationResourcesToPack(const String& path);
    void CopySceneResourcesToPack(const String& path);

   private:
    UniqueStringArray m_fontResourcePaths;
    UniqueStringArray m_materialResourcePaths;
    UniqueStringArray m_meshResourcePaths;
    UniqueStringArray m_shaderResourcePaths;
    UniqueStringArray m_textureResourcePaths;
    UniqueStringArray m_animResourcePaths;
    UniqueStringArray m_sceneResourcePaths;

    Path m_minFontsDirectoryPath;
    Path m_minMaterialsDirectoryPath;
    Path m_minMeshesDirectoryPath;
    Path m_minShadersDirectoryPath;
    Path m_minTexturesDirectoryPath;
    Path m_minAnimDirectoryPath;
    Path m_minSceneDirectoryPath;
  };
}  // namespace ToolKit
