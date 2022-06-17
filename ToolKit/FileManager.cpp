
#include "FileManager.h"

#include <string>
#include <unordered_map>
#include <memory>

#include "ToolKit.h"

namespace ToolKit
{
  void FileManager::PackResources(const String& path)
  {
    LoadAllScenes(path);
    GetAllUsedResourcePaths(path);
    //CreatePackResources(path);
    
    if (!CompressPack())
    {
      // Error
      GetLogger()->WriteConsole(LogType::Error, "Error zipping.");
    }
    else
    {
      GetLogger()->WriteConsole(LogType::Warning, "Resources packed.");
    }
  }

  void FileManager::LoadAllScenes(const String& path)
  {
    // Resources in the scene files
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go scenes in directories
        LoadAllScenes(entry.path().string());
        continue;
      }

      // Load all scenes
      String pt = entry.path().string();
      ScenePtr scene = GetSceneManager()->Create<Scene>(pt);
      scene->Load();
      scene->Init();
    }
  }

  void FileManager::GetAllUsedResourcePaths(const String& path)
  {
    std::unordered_map<String, ResourcePtr> mp;

    // No manager for fonts

    // Material
    mp = GetMaterialManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;

      // Skip default.material
      if (!it->second->m_loaded)
      {
        continue;
      }

      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Materials");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath});
      }

      m_materialResourcePaths.insert(absolutePath);
    }

    // Meshes
    mp = GetMeshManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Meshes");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      m_meshResourcePaths.insert(absolutePath);
    }

    // Shaders
    mp = GetShaderManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Shaders");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      m_shaderResourcePaths.insert(absolutePath);
    }

    // Textures
    mp = GetTextureManager()->m_storage;
    for (auto it = mp.begin(); it != mp.end(); it++)
    {
      String absolutePath = it->first;
      // If the path is relative, make it absolute
      if (absolutePath[0] == '.')
      {
        size_t index = absolutePath.find("Textures");
        absolutePath = absolutePath.substr(index);
        absolutePath = ConcatPaths({ DefaultAbsolutePath(), absolutePath });
      }

      m_textureResourcePaths.insert(absolutePath);
    }

    // Animations
    static String animPath = ConcatPaths({ ResourcePath(), "Meshes", "anim" });
    GetAnimationPaths(animPath);

    // Scenes
    GetScenePaths(path);
  }

  void FileManager::CreatePackResources(const String& path)
  {
    // Create directories
    CreatePackDirectories();

    // Copy the files into the directories
    CopyFontResourcesToPack();
    CopyMaterialResourcesToPack();
    CopyMeshResourcesToPack();
    CopyShaderResourcesToPack();
    CopyTextureResourcesToPack();
    CopyAnimationResourcesToPack(ConcatPaths({ path, "..", "Meshes", "anim" }));
    CopySceneResourcesToPack(path);

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
    m_minSceneDirectoryPath = ConcatPaths
    (
      { minResourceDirectoryPath, "Scenes" }
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
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
        "Error when creating directory: %s",
        m_minAnimDirectoryPath.c_str()
      );
      return;
    }

    if
    (
      !std::filesystem::create_directory(m_minSceneDirectoryPath)
      && !std::filesystem::exists(m_minSceneDirectoryPath)
    )
    {
      GetLogger()->Log
      (
        "Creating pack directory path has failed: "
        + m_minSceneDirectoryPath.string()
      );
      GetLogger()->WriteConsole
      (
        LogType::Warning,
        "Error when creating directory: %s",
        m_minSceneDirectoryPath.c_str()
      );
      return;
    }
  }

  bool FileManager::CompressPack()
  {
    UniqueStringArray allPaths;
    allPaths.insert(m_fontResourcePaths.begin(), m_fontResourcePaths.end());
    allPaths.insert(m_materialResourcePaths.begin(), m_materialResourcePaths.end());
    allPaths.insert(m_meshResourcePaths.begin(), m_meshResourcePaths.end());
    allPaths.insert(m_shaderResourcePaths.begin(), m_shaderResourcePaths.end());
    allPaths.insert(m_textureResourcePaths.begin(), m_textureResourcePaths.end());
    allPaths.insert(m_animResourcePaths.begin(), m_animResourcePaths.end());
    allPaths.insert(m_sceneResourcePaths.begin(), m_sceneResourcePaths.end());

    String zipName = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });
    zipFile zFile = zipOpen64(zipName.c_str(), 0);

    if (zFile == NULL)
    {
      printf("Error opening zip.\n");
      zipClose(zFile, NULL);
      return false;
    }

    // Add files to zip
    for (String path : allPaths)
    {
      if (!AddFileToZip(zFile, path.c_str()))
      {
        GetLogger()->WriteConsole(LogType::Error, "Error adding file to zip.");
        return false;
      }
    }

    zipClose(zFile, NULL);

    return true;
  }

  bool FileManager::AddFileToZip(zipFile zfile, const char* filename)
  {
    FILE* f;
    int ret;
    size_t red;
    size_t flen;

    if (zfile == NULL || filename == NULL)
    {
      return false;
    }

    f = fopen(filename, "rb");
    if (f == NULL)
    {
      return false;
    }

    fseek(f, 0, SEEK_END);
    flen = ftell(f);
    rewind(f);

    String filenameStr = String(filename);
    size_t index = filenameStr.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      filenameStr = filenameStr.substr(index + length);
    }
    else
    {
      GetLogger()->WriteConsole(LogType::Error, "Resource is not under resources path: %s", filename);
      return false;
    }

    ret = zipOpenNewFileInZip64(zfile, filenameStr.c_str(), NULL, NULL, 0, NULL, 0, NULL,
      Z_DEFLATED, Z_DEFAULT_COMPRESSION, (flen > 0xffffffff) ? 1 : 0);
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    char* file_data = (char*)malloc((flen + 1) * sizeof(char));
    red = fread(file_data, flen, 1, f);
    ret = zipWriteInFileInZip(zfile, file_data, red * flen);
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    free(file_data);
    fclose(f);
    zipCloseFileInZip(zfile);

    return true;
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

      m_animResourcePaths.insert(entry.path().string());
    }
  }

  void FileManager::GetScenePaths(const String& path)
  {
    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
      if (entry.is_directory())
      {
        // Go scenes in directories
        GetScenePaths(entry.path().string());
        continue;
      }

      m_sceneResourcePaths.insert(entry.path().string());
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

  void FileManager::CopyAnimationResourcesToPack(const String& path)
  {
    String animPath = ConcatPaths({ ResourcePath(), "Meshes", "anim" });
    std::filesystem::copy
    (
      animPath,
      m_minAnimDirectoryPath,
      std::filesystem::copy_options::overwrite_existing |
      std::filesystem::copy_options::recursive
    );
  }

  void FileManager::CopySceneResourcesToPack(const String& path)
  {
    std::filesystem::copy
    (
      path,
      m_minSceneDirectoryPath,
      std::filesystem::copy_options::overwrite_existing |
      std::filesystem::copy_options::recursive
    );
  }
}  // namespace ToolKit
