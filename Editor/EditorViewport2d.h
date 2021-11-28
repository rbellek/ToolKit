#pragma once

#include "ToolKit.h"
#include "Viewport.h"
#include "UI.h"
#include "EditorViewport.h"

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  namespace Editor
  {

    class EditorViewport2d : public EditorViewport
    {
    public:
      EditorViewport2d(XmlNode* node);
      EditorViewport2d(float width, float height);
      virtual ~EditorViewport2d();

      // Window Overrides.
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void Update(float deltaTime) override;
      virtual void OnResize(float width, float height) override;

    protected:
      void UpdateContentArea();
      void UpdateWindow();
      void DrawCommands();
      void HandleDrop();
      void DrawOverlays();

    private:
      void Init2dCam();
      void GetGlobalCanvasSize();
      void PanZoom(float deltaTime);
      void AdjustZoom(float z);

    public:
      Vec2 m_canvasSize;
    
    private:
      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;
      Vec2 m_scroll;
    };

  }

}
