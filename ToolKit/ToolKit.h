#pragma once

/**
* @file ToolKit.h Header for Main, the manager class to access all the 
* functionalities of the ToolKit framework.
*/

#include "Animation.h"
#include "Audio.h"
#include "Directional.h"
#include "Drawable.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Primative.h"
#include "RenderState.h"
#include "Renderer.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "StateMachine.h"
#include "Surface.h"
#include "Texture.h"
#include "Scene.h"
#include "PluginManager.h"
#include "Events.h"
#include "UIManager.h"
#include "Scene.h"
#include "Renderer.h"
#include "Material.h"
#include "Types.h"
#include "Skeleton.h"
#include "FileManager.h"

/**
* Base name space for all the ToolKit functionalities.
*/
namespace ToolKit
{

  /**
  * A class that Provides a unique handle when needed.
  */
  class TK_API HandleManager
  {
   public:
    /**
    * Creates a handle that has not been allocated within the current runtime.
    * Increments the m_baseHandle by one.
    * @return A unique handle for the current runtime.
    */
    ULongID GetNextHandle();

    /**
    * This function allows to set the m_baseHandle to maximum of
    * val and m_baseHandle. That is m_baseHandle = max(m_baseHandle, val).
    * @param val The value to compare with current handle.
    */
    void SetMaxHandle(ULongID val);

   private:
    ULongID m_baseHandle = 1000;  //!< Starting value of the handles.
  };

  class TK_API Main
  {
   public:
    Main();
    virtual ~Main();

    Main(Main const&) = delete;
    void operator=(Main const&) = delete;

    virtual void Init();
    virtual void Uninit();
    static Main* GetInstance();
    static void SetProxy(Main* proxy);

   public:
    AnimationManagerPtr m_animationMan;
    AnimationPlayerPtr m_animationPlayer;
    AudioManagerPtr m_audioMan;
    MaterialManagerPtr m_materialManager;
    MeshManagerPtr m_meshMan;
    ShaderManagerPtr m_shaderMan;
    SpriteSheetManagerPtr m_spriteSheetMan;
    TextureManagerPtr m_textureMan;
    SceneManagerPtr m_sceneManager;
    PluginManagerPtr m_pluginManager;
    RendererPtr m_renderer;
    LoggerPtr m_logger;
    UIManagerPtr m_uiManager;
    SkeletonManagerPtr m_skeletonManager;
    HandleManager m_handleManager;
    FileManagerPtr m_fileManager;

    bool m_initiated = false;
    String m_resourceRoot;
    EventPool m_eventPool;

   private:
    static Main* m_proxy;
  };

  // Accessors.
  TK_API LoggerPtr GetLogger();
  TK_API RendererPtr GetRenderer();
  TK_API AnimationManagerPtr GetAnimationManager();
  TK_API AnimationPlayerPtr GetAnimationPlayer();
  TK_API AudioManagerPtr GetAudioManager();
  TK_API MaterialManagerPtr GetMaterialManager();
  TK_API MeshManagerPtr GetMeshManager();
  TK_API ShaderManagerPtr GetShaderManager();
  TK_API SpriteSheetManagerPtr GetSpriteSheetManager();
  TK_API TextureManagerPtr GetTextureManager();
  TK_API SceneManagerPtr GetSceneManager();
  TK_API PluginManagerPtr GetPluginManager();
  TK_API ResourceManagerPtr GetResourceManager(ResourceType type);
  TK_API UIManagerPtr GetUIManager();
  TK_API HandleManager* GetHandleManager();
  TK_API SkeletonManagerPtr GetSkeletonManager();
  TK_API FileManagerPtr GetFileManager();

  TK_API String DefaultPath();
  TK_API String DefaultAbsolutePath();
  TK_API String ResourcePath(bool def = false);
  TK_API String TexturePath(const String& file, bool def = false);
  TK_API String MeshPath(const String& file, bool def = false);
  TK_API String FontPath(const String& file, bool def = false);
  TK_API String SpritePath(const String& file, bool def = false);
  TK_API String AudioPath(const String& file, bool def = false);
  TK_API String AnimationPath(const String& file, bool def = false);
  TK_API String SkeletonPath(const String& file, bool def = false);
  TK_API String ShaderPath(const String& file, bool def = false);
  TK_API String MaterialPath(const String& file, bool def = false);
  TK_API String ScenePath(const String& file, bool def = false);
  TK_API String PrefabPath(const String& file, bool def = false);

}  // namespace ToolKit
