#pragma once

#include <filesystem>

#include "zip.h"
#include "unzip.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>
#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    ~FileManager();

    XmlFile GetXmlFile(const String& path);
    void PackResources(const String& path);

   private:
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack();
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAnimationPaths(const String& path);
    void GetScenePaths(const String& path);

    String GetRelativeResourcesPath(const String& path);
    XmlFile ReadFileFromZip
    (
      zipFile zfile,
      const char* relativePath,
      const char* path
    );

   private:
    StringSet allPaths;
    zipFile zfile;

    struct _streambuf : std::streambuf
    {
      _streambuf(char* begin, char* end)
      {
        this->setg(begin, begin, end);
      }
    };
  };
}  // namespace ToolKit
