#pragma once

#include <set>
#include <filesystem>

#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
  public:
    void PackResources(const String& path);

  private:
    void GetUsedResourcePaths(const String& path);
    void GetAnimationPaths(const String& path);
    void CreatePackResources();
    void CreatePackDirectories();

    void CopyFontResourcesToPack();
    void CopyMaterialResourcesToPack();
    void CopyMeshResourcesToPack();
    void CopyShaderResourcesToPack();
    void CopyTextureResourcesToPack();
    void CopyAnimationResourcesToPack();

  private:
    std::set<String> m_fontResourcePaths;
    std::set<String> m_materialResourcePaths;
    std::set<String> m_meshResourcePaths;
    std::set<String> m_shaderResourcePaths;
    std::set<String> m_textureResourcePaths;
    std::set<String> m_animationResourcePaths;

    std::filesystem::path m_minFontsDirectoryPath;
    std::filesystem::path m_minMaterialsDirectoryPath;
    std::filesystem::path m_minMeshesDirectoryPath;
    std::filesystem::path m_minShadersDirectoryPath;
    std::filesystem::path m_minTexturesDirectoryPath;
    std::filesystem::path m_minAnimDirectoryPath;
  };
}  // namespace ToolKit
