#include "ToolKit.h"

#include <algorithm>
#include <filesystem>
#include <string>

#include "DebugNew.h"

namespace ToolKit
{

  ULongID HandleManager::GetNextHandle()
  {
    return ++m_baseHandle;
  }

  void HandleManager::SetMaxHandle(ULongID val)
  {
    m_baseHandle = glm::max(m_baseHandle, val);
  }

  Main* Main::m_proxy = nullptr;

  Main::Main()
  {
    m_logger = std::make_shared<Logger>();

    m_renderer = std::make_shared<Renderer>();
    m_pluginManager = std::make_shared<PluginManager>();
    m_animationMan = std::make_shared<AnimationManager>();
    m_animationPlayer = std::make_shared<AnimationPlayer>();
    m_textureMan = std::make_shared<TextureManager>();
    m_meshMan = std::make_shared<MeshManager>();
    m_spriteSheetMan = std::make_shared<SpriteSheetManager>();
    m_audioMan = std::make_shared<AudioManager>();
    m_shaderMan = std::make_shared<ShaderManager>();
    m_materialManager = std::make_shared<MaterialManager>();
    m_sceneManager = std::make_shared<SceneManager>();
    m_uiManager = std::make_shared<UIManager>();
    m_skeletonManager = std::make_shared<SkeletonManager>();
    m_fileManager = std::make_shared<FileManager>();

    m_logger->Log("Main Constructed");
  }

  Main::~Main()
  {
    assert(m_initiated == false && "Uninitiate before deconstruct");
    m_logger->Log("Main Deconstructed");
  }

  void Main::Init()
  {
    // Start Timer.
    GetElapsedMilliSeconds();

    m_logger->Log("ToolKit Initialization");

    m_pluginManager->Init();
    m_animationMan->Init();
    m_textureMan->Init();
    m_meshMan->Init();
    m_spriteSheetMan->Init();
    m_audioMan->Init();
    m_shaderMan->Init();
    m_materialManager->Init();
    m_sceneManager->Init();
    m_skeletonManager->Init();

    m_initiated = true;
  }

  void Main::Uninit()
  {
    m_logger->Log("ToolKit Unitialization");

    m_animationPlayer->m_records.clear();
    m_animationMan->Uninit();
    m_textureMan->Uninit();
    m_meshMan->Uninit();
    m_spriteSheetMan->Uninit();
    m_audioMan->Uninit();
    m_shaderMan->Uninit();
    m_materialManager->Uninit();
    m_sceneManager->Uninit();
    m_skeletonManager->Uninit();

    // After all the resources, we can safely free modules.
    m_pluginManager->UnInit();

    m_initiated = false;
  }

  Main* Main::GetInstance()
  {
    assert(m_proxy);
    return m_proxy;
  }

  void Main::SetProxy(Main* proxy)
  {
    bool singular = m_proxy == nullptr || m_proxy == proxy;
    assert(singular && "You can only have one instance of the main");
    if (singular)
    {
      m_proxy = proxy;
    }
  }

  LoggerPtr GetLogger()
  {
    return Main::GetInstance()->m_logger;
  }

  RendererPtr GetRenderer()
  {
    return Main::GetInstance()->m_renderer;
  }

  AnimationManagerPtr GetAnimationManager()
  {
    return Main::GetInstance()->m_animationMan;
  }

  AnimationPlayerPtr GetAnimationPlayer()
  {
    return Main::GetInstance()->m_animationPlayer;
  }

  AudioManagerPtr GetAudioManager()
  {
    return Main::GetInstance()->m_audioMan;
  }

  MaterialManagerPtr GetMaterialManager()
  {
    return Main::GetInstance()->m_materialManager;
  }

  MeshManagerPtr GetMeshManager()
  {
    return Main::GetInstance()->m_meshMan;
  }

  ShaderManagerPtr GetShaderManager()
  {
    return Main::GetInstance()->m_shaderMan;
  }

  SpriteSheetManagerPtr GetSpriteSheetManager()
  {
    return Main::GetInstance()->m_spriteSheetMan;
  }

  TextureManagerPtr GetTextureManager()
  {
    return Main::GetInstance()->m_textureMan;
  }

  SceneManagerPtr GetSceneManager()
  {
    return Main::GetInstance()->m_sceneManager;
  }

  PluginManagerPtr GetPluginManager()
  {
    return Main::GetInstance()->m_pluginManager;
  }

  ResourceManagerPtr GetResourceManager(ResourceType type)
  {
    switch (type)
    {
    case ResourceType::Animation:
      return GetAnimationManager();
    case ResourceType::Audio:
      return GetAudioManager();
    case ResourceType::Material:
      return GetMaterialManager();
    case ResourceType::SkinMesh:
    case ResourceType::Mesh:
      return GetMeshManager();
    case ResourceType::Shader:
      return GetShaderManager();
    case ResourceType::SpriteSheet:
      return GetSpriteSheetManager();
    case ResourceType::Texture:
    case ResourceType::CubeMap:
    case ResourceType::RenderTarget:
      return GetTextureManager();
    case ResourceType::Scene:
      return GetSceneManager();
    case ResourceType::Base:
    default:
      assert(false);
      break;
    }

    return nullptr;
  }

  UIManagerPtr GetUIManager()
  {
    return Main::GetInstance()->m_uiManager;
  }

  HandleManager* GetHandleManager()
  {
    return &Main::GetInstance()->m_handleManager;
  }

  TK_API SkeletonManagerPtr GetSkeletonManager()
  {
    return Main::GetInstance()->m_skeletonManager;
  }

  TK_API FileManagerPtr GetFileManager()
  {
    return Main::GetInstance()->m_fileManager;
  }

  String DefaultAbsolutePath()
  {
    static String cur = std::filesystem::current_path().string();
    static StringArray splits;
    Split(cur, GetPathSeparatorAsStr(), splits);
    splits.erase(splits.end() - 1);
    splits.push_back("Resources");
    splits.push_back("Engine");
    static String res = ConcatPaths(splits);

    return res;
  }

  String DefaultPath()
  {
    static String res = ConcatPaths({ ".", "..", "Resources", "Engine" });

    return res;
  }

  String ResourcePath(bool def)
  {
    if (!def)
    {
      String& path = Main::GetInstance()->m_resourceRoot;
      if (!path.empty())
      {
        return path;
      }
    }

    return DefaultPath();
  }

  /* 
  * When dynamically created resources refer to default assets,
  * they got saved with an altered relative path which starts with ToolKit.
  * Check Util.h GetRelativeResourcePath() for more.
  * So here, we try to detect defaul assets.
  */
  bool CheckForRelative(const String& file)
  {
    return file.find("ToolKit") != String::npos;
  }

  String ProcessPath(const String& file, const String& prefix, bool def)
  {
    if (CheckForRelative(file))
    {
      constexpr int length = sizeof("ToolKit");
      String modified = file.substr(length);
      String path = ConcatPaths({ ResourcePath(true), prefix, modified });
      return path;
    }

    String path = ConcatPaths({ ResourcePath(def), prefix, file });
    return path;
  }

  String TexturePath(const String& file, bool def)
  {
    return ProcessPath(file, "Textures", def);
  }

  String MeshPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String FontPath(const String& file, bool def)
  {
    return ProcessPath(file, "Fonts", def);
  }

  String SpritePath(const String& file, bool def)
  {
    return ProcessPath(file, "Sprites", def);
  }

  String AudioPath(const String& file, bool def)
  {
    return ProcessPath(file, "Audio", def);
  }

  String AnimationPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String SkeletonPath(const String& file, bool def)
  {
    return ProcessPath(file, "Meshes", def);
  }

  String ShaderPath(const String& file, bool def)
  {
    return ProcessPath(file, "Shaders", def);
  }

  String MaterialPath(const String& file, bool def)
  {
    return ProcessPath(file, "Materials", def);
  }

  String ScenePath(const String& file, bool def)
  {
    return ProcessPath(file, "Scenes", def);
  }

  String PrefabPath(const String& file, bool def)
  {
    return ProcessPath(file, "Prefabs", def);
  }

}  //  namespace ToolKit
