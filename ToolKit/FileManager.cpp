
#include "FileManager.h"

#include "ToolKit.h"

namespace ToolKit
{
  void FileManager::PackResources(const String& path)
  {
    GetUsedResourcePaths(path);
    GetAnimationPaths(ConcatPaths({ path, "..", "Meshes", "anim" }));
    CreatePackResources();
  }

  void FileManager::GetUsedResourcePaths(const String& path)
  {
    // Resources in the scene files
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go scenes in directories
        GetUsedResourcePaths(entry.path().string());
        continue;
      }

      // Traverse scene file
      String pt = entry.path().string();
      ScenePtr scene = GetSceneManager()->Create<Scene>(pt);
      scene->Load();
      scene->Init();

      // Look for mesh component
      for (Entity* ntt : scene->GetEntities())
      {
        MeshComponentPtr mc = ntt->GetComponent<MeshComponent>();
        if (mc != nullptr)
        {
          // Mesh files
          String file = mc->Mesh()->GetFile();
          if (!file.empty())
          {
            m_meshResourcePaths.insert(mc->Mesh()->GetFile());
          }
          else
          {
            continue;
          }

          // Material files
          MaterialPtr mat = mc->Mesh()->m_material;
          if (mat != nullptr)
          {
            file = mat->GetFile();
            if (!file.empty())
            {
              m_materialResourcePaths.insert(mat->GetFile());
            }
          }
          else
          {
            continue;
          }

          // Textures
          if (mat->m_diffuseTexture != nullptr)
          {
            m_textureResourcePaths.insert(mat->m_diffuseTexture->GetFile());
          }

          // Shaders
          if (mat->m_vertexShader != nullptr)
          {
            m_shaderResourcePaths.insert(mat->m_vertexShader->GetFile());
          }
          if (mat->m_fragmetShader != nullptr)
          {
            m_shaderResourcePaths.insert(mat->m_fragmetShader->GetFile());
          }
          if (mat->m_cubeMap != nullptr)
          {
            m_shaderResourcePaths.insert(mat->m_cubeMap->GetFile());
          }
        }
      }
    }
  }

  void FileManager::GetAnimationPaths(const String& path)
  {
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go animations in directories
        GetAnimationPaths(entry.path().string());
        continue;
      }

      m_animationResourcePaths.insert(entry.path().string());
    }
  }

  void FileManager::CreatePackResources()
  {
    // Create directories
    CreatePackDirectories();

    // Copy the files into the directories
    CopyFontResourcesToPack();
    CopyMaterialResourcesToPack();
    CopyMeshResourcesToPack();
    CopyShaderResourcesToPack();
    CopyTextureResourcesToPack();
    CopyAnimationResourcesToPack();

    GetLogger()->WriteConsole(LogType::Warning, "Resources packed.");
  }

  void FileManager::CreatePackDirectories()
  {
    String minResourceDirectoryPath = ConcatPaths
    (
      {
        ResourcePath(),
        "..",
        "MinResources"
      }
    );

    m_minFontsDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Fonts" }
    );
    m_minMaterialsDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Materials" }
    );
    m_minMeshesDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Meshes" }
    );
    m_minShadersDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Shaders" }
    );
    m_minTexturesDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Textures" }
    );
    m_minAnimDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Meshes", "anim"}
    );

    // Create directories
    if
    (
      !std::filesystem::create_directory(minResourceDirectoryPath)
      && !std::filesystem::exists(minResourceDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: " + minResourceDirectoryPath
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        minResourceDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minFontsDirectoryPath)
      && !std::filesystem::exists(m_minFontsDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minFontsDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minFontsDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minMaterialsDirectoryPath)
      && !std::filesystem::exists(m_minMaterialsDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minMaterialsDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minMaterialsDirectoryPath.c_str()
      );
      return;
    }
    
    if
    (
      !std::filesystem::create_directory(m_minMeshesDirectoryPath)
      && !std::filesystem::exists(m_minMeshesDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minMeshesDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minMeshesDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minShadersDirectoryPath)
      && !std::filesystem::exists(m_minShadersDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minShadersDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minShadersDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minTexturesDirectoryPath)
      && !std::filesystem::exists(m_minTexturesDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minTexturesDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minTexturesDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minAnimDirectoryPath)
      && !std::filesystem::exists(m_minAnimDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minAnimDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "%s",
        m_minAnimDirectoryPath.c_str()
      );
      return;
    }
  }

  void FileManager::CopyFontResourcesToPack()
  {
    for (String path : m_fontResourcePaths)
    {
      // Get directories
      size_t index = path.find("Fonts");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"Fonts\" directory in font path."
        );
        return;
      }

      constexpr size_t length = sizeof("Fonts");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths({ m_minFontsDirectoryPath.string(), dirPath });
        if
        (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
        )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minFontsDirectoryPath.string();
      }

      // Copy
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }

  void FileManager::CopyMaterialResourcesToPack()
  {
    for (String path : m_materialResourcePaths)
    {
      // Get directories
      size_t index = path.find("Materials");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"Materials\" directory in material path."
        );
        return;
      }

      constexpr size_t length = sizeof("Materials");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths
        (
          { m_minMaterialsDirectoryPath.string(), dirPath }
        );
        if
        (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
        )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minMaterialsDirectoryPath.string();
      }

      // Copy file
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }

  void FileManager::CopyMeshResourcesToPack()
  {
    for (String path : m_meshResourcePaths)
    {
      // Get directories
      size_t index = path.find("Meshes");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"Meshes\" directory in mesh path."
        );
        return;
      }

      constexpr size_t length = sizeof("Meshes");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths({ m_minMeshesDirectoryPath.string(), dirPath });
        if
        (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
        )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minMeshesDirectoryPath.string();
      }

      // Copy
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }

  void FileManager::CopyShaderResourcesToPack()
  {
    for (String path : m_shaderResourcePaths)
    {
      // Get directories
      size_t index = path.find("Shaders");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"Shaders\" directory in shader path."
        );
        return;
      }

      constexpr size_t length = sizeof("Shaders");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths({ m_minShadersDirectoryPath.string(), dirPath });
        if
        (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
        )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minShadersDirectoryPath.string();
      }

      // Copy
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }

  void FileManager::CopyTextureResourcesToPack()
  {
    for (String path : m_textureResourcePaths)
    {
      // Get directories
      size_t index = path.find("Textures");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"Textures\" directory in texture path."
        );
        return;
      }

      constexpr size_t length = sizeof("Textures");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths({ m_minTexturesDirectoryPath.string(), dirPath });
        if
        (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
        )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minTexturesDirectoryPath.string();
      }

      // Copy
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }

  void FileManager::CopyAnimationResourcesToPack()
  {
    for (String path : m_animationResourcePaths)
    {
      // Get directories
      size_t index = path.find("anim");
      if (index == String::npos)
      {
        GetLogger()->WriteConsole
        (
          LogType::Error,
          "No \"anim\" directory in animation path."
        );
        return;
      }

      constexpr size_t length = sizeof("anim");
      String relativePath = path.substr(length + index);

      String dirPath;
      String name;
      String ext;
      DecomposePath(relativePath, &dirPath, &name, &ext);
      if (dirPath.compare(name + ext))  // There are sub directories
      {
        // Create directories
        dirPath = ConcatPaths({ m_minAnimDirectoryPath.string(), dirPath });
        if
          (
          !std::filesystem::create_directories(dirPath)
          && !std::filesystem::exists(dirPath)
          )
        {
          GetLogger()->Log
          (
            "Creating pack directory path has failed: " + dirPath
          );
          GetLogger()->WriteConsole
          (
            LogType::Error,
            "Packing failed while creating %s",
            dirPath.c_str()
          );
          return;
        }
      }
      else  // No sub diretories
      {
        dirPath = m_minAnimDirectoryPath.string();
      }

      // Copy
      std::filesystem::copy
      (
        path,
        dirPath,
        std::filesystem::copy_options::overwrite_existing
      );
    }
  }
}  // namespace ToolKit
