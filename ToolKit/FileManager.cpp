
#include "FileManager.h"

#include <string.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>

#include "ToolKit.h"

namespace ToolKit
{
  FileManager::~FileManager()
  {
    // Close zip file
    unzClose(zfile);
  }

  XmlFile FileManager::GetXmlFile(const String& path)
  {
    String pakPath = ConcatPaths({ ResourcePath(), "..", "MinResources.pak" });

    // Get relative path from Resources directory
    String relativePath = GetRelativeResourcesPath(path);
    if (relativePath.empty())
    {
      return nullptr;
    }
    const char* relativePathC = relativePath.c_str();

    static bool openZip = true;

    if (openZip)
    {
      zfile = unzOpen(pakPath.c_str());
    }

    if (zfile)
    {
      openZip = false;
      XmlFile file = ReadFileFromZip(zfile, relativePathC, path.c_str());

      return file;
    }
    else
    {
      // Zip pak not found, read from file at default path
      XmlFile file = XmlFile(path.c_str());
      return file;
    }
  }

  void FileManager::PackResources(const String& path)
  {
    // Load all scenes once in order to fill resource managers
    LoadAllScenes(path);

    // Get all paths of resources
    GetAllUsedResourcePaths(path);

    // Zip used resources
    if (!ZipPack())
    {
      // Error
      GetLogger()->WriteConsole(LogType::Error, "Error zipping.");
    }
    else
    {
      GetLogger()->WriteConsole(LogType::Memo, "Resources packed.");
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

      allPaths.insert(absolutePath);
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

      allPaths.insert(absolutePath);
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

      allPaths.insert(absolutePath);
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

      allPaths.insert(absolutePath);
    }

    // Animations
    static String animPath = ConcatPaths({ ResourcePath(), "Meshes", "anim" });
    GetAnimationPaths(animPath);

    // Scenes
    GetScenePaths(path);
  }

  bool FileManager::ZipPack()
  {
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
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Resource is not under resources path: %s",
        filename
      );
      return false;
    }

    ret = zipOpenNewFileInZip64
    (
      zfile,
      filenameStr.c_str(),
      NULL,
      NULL,
      0,
      NULL,
      0,
      NULL,
      Z_DEFLATED,
      Z_DEFAULT_COMPRESSION,
      (flen > 0xffffffff) ? 1 : 0
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    char* fileData = reinterpret_cast<char*>
    (
      malloc((flen + 1) * static_cast<unsigned int>(sizeof(char)))
    );
    red = fread(fileData, flen, 1, f);
    ret = zipWriteInFileInZip
    (
      zfile,
      fileData,
      static_cast<unsigned int>(red * flen)
    );
    if (ret != ZIP_OK)
    {
      fclose(f);
      zipCloseFileInZip(zfile);
      return false;
    }

    free(fileData);
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

      allPaths.insert(entry.path().string());
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

      allPaths.insert(entry.path().string());
    }
  }

  String FileManager::GetRelativeResourcesPath(const String& path)
  {
    String filenameStr = String(path);
    size_t index = filenameStr.find("Resources");
    if (index != String::npos)
    {
      constexpr int length = sizeof("Resources");
      filenameStr = filenameStr.substr(index + length);
      return filenameStr;
    }
    else
    {
      GetLogger()->WriteConsole
      (
        LogType::Error,
        "Resource is not under resources path: %s",
        path
      );
      return "";
    }
  }

  XmlFile FileManager::ReadFileFromZip
  (
    zipFile zfile,
    const char* relativePath,
    const char* path
  )
  {
    if (unzGoToFirstFile(zfile) == UNZ_OK)
    {
      // Iterate over file info's
      do
      {
        if (unzOpenCurrentFile(zfile) == UNZ_OK)
        {
          unz_file_info fileInfo;
          memset(&fileInfo, 0, sizeof(unz_file_info));

          if
          (
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              NULL,
              0,
              NULL,
              0,
              NULL,
              0
            ) == UNZ_OK
          )
          {
            // Get file info
            char* filename = reinterpret_cast<char*>
            (
              malloc(fileInfo.size_filename + 1)
            );
            unzGetCurrentFileInfo
            (
              zfile,
              &fileInfo,
              filename,
              fileInfo.size_filename + 1,
              NULL,
              0,
              NULL,
              0
            );
            filename[fileInfo.size_filename] = '\0';

            if (strcmp(filename, relativePath))
            {
              free(filename);
              unzCloseCurrentFile(zfile);
              continue;
            }

            // Read file
            unsigned int filesize =
            static_cast<unsigned int>(fileInfo.uncompressed_size);
            char* fileBuffer = reinterpret_cast<char*>(malloc(filesize));
            int readBytes = unzReadCurrentFile(zfile, fileBuffer, filesize);
            if (readBytes < 0)
            {
              GetLogger()->Log
              (
                "Error reading compressed file: " + String(filename)
              );
              GetLogger()->WriteConsole
              (
                LogType::Error,
                "Error reading compressed file: %s",
                filename
              );
            }

            // Create XmlFile object
            _streambuf sbuf(fileBuffer, fileBuffer + filesize);
            std::basic_istream<char> stream(&sbuf);
            XmlFile file(stream);

            free(fileBuffer);
            free(filename);

            return file;
          }

          unzCloseCurrentFile(zfile);
        }
      } while (unzGoToNextFile(zfile) == UNZ_OK);
    }

    return XmlFile(path);
  }
}  // namespace ToolKit
