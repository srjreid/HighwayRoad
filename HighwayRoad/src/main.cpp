/*
Prime Engine

MIT License

Copyright (c) 2024 Sean Reid (email@seanreid.ca)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Engine.h>
#include <Prime/Graphics/Graphics.h>
#include <Prime/Input/Keyboard.h>
#include <Prime/Input/Touch.h>
#include <Prime/Font/Font.h>
#include <Prime/Model/Model.h>
#include <Prime/Imagemap/Imagemap.h>

using namespace Prime;

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

typedef struct {
  Model* model;     // the model object to draw
  f32 scale;        // the scale of the model object
  f32 x;            // how far away object is from road side (right = positive, left = negative), normalized to road texture width
  f32 z;            // how far along the road the object is, normalized to texture height
  f32 angle;        // the angle of the building, rotated in the up/y-axis
} HighwayObject;

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////

#define NearZ                   0.1f
#define FarZ                    1000.0f
#define ViewHeight              0.4f
#define ViewAzimuthStart        0.0f
#define ViewAltitudeStart       0.0f
#define ViewSensitivity         0.1f
#define RoadRepetitionCount     100.0f
#define MoveSpeed               2.0f
#define MoveSpeedScaleFast      5.0f
#define FocusObjectTime         0.7f
#define FocusObjectOffsetPos    (-2.0f)

#define RhinoScale              0.3f
#define TreeScale               0.015f
#define BuildingScale           0.1f

////////////////////////////////////////////////////////////////////////////////
// Entry
////////////////////////////////////////////////////////////////////////////////

int main(int argc, const char* const* argv) {
#if defined(_DEBUG) && defined(PrimeTargetWindows)
  // If debugging in Windows, enable memory leak detection at end of program execution.
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

  // Init engine.
  Engine& engine = PxEngine;

  // Load font.
  refptr font = new Font();

  GetContent("data/Font/NotoSansCJKtc-Regular.otf", [=](Content* content) {
    font->SetContent(content);
  });

  // Load shaders.
  refptr texProgram = DeviceProgram::Create("data/Shader/Tex/Tex.vsh", "data/Shader/Tex/Tex.fsh");
  refptr scrollTexProgram = DeviceProgram::Create("data/Shader/Tex/ScrollTex.vsh", "data/Shader/Tex/ScrollTex.fsh");
  refptr modelProgram = DeviceProgram::Create("data/Shader/Model/Model.vsh", "data/Shader/Model/Model.fsh");
  refptr modelAnimProgram = DeviceProgram::Create("data/Shader/Model/ModelAnim.vsh", "data/Shader/Model/ModelAnim.fsh");

  // Load assets.
  refptr road = new Imagemap();
  GetContent("data/Asset/Road.png", [=](Content* content) {
    road->SetContent(content);

    auto roadContent = road->GetImagemapContent();
    auto tex = roadContent->GetTex();
    if(tex) {
      tex->SetWrapModeX(WrapModeRepeat);
      tex->SetWrapModeY(WrapModeRepeat);
    }
  });

  refptr grass = new Imagemap();
  GetContent("data/Asset/Grass.png", [=](Content* content) {
    grass->SetContent(content);

    auto grassContent = grass->GetImagemapContent();
    auto tex = grassContent->GetTex();
    if(tex) {
      tex->SetWrapModeX(WrapModeRepeat);
      tex->SetWrapModeY(WrapModeRepeat);
    }
  });

  refptr tree = new Model();
  GetContent("data/Asset/Tree.obj", [=](Content* content) {
    tree->SetContent(content);

    GetContent("data/Asset/TreeTexture.png", [=](Content* content) {
      auto tex = content->GetAs<ImagemapContent>()->GetTex();
      if(tex) {
        tree->ApplyTextureOverride("", tex);
      }
    });
  });

  refptr buildingBasic = new Model();
  GetContent("data/Asset/Building/Basic/Model.fbx", [=](Content* content) {
    buildingBasic->SetContent(content);

    GetContent("data/Asset/Building/Basic/Texture.png", [=](Content* content) {
      auto tex = content->GetAs<ImagemapContent>()->GetTex();
      if(tex) {
        buildingBasic->ApplyTextureOverride("", tex);
      }
    });
  });

  refptr buildingFlower = new Model();
  GetContent("data/Asset/Building/Flower/Model.fbx", [=](Content* content) {
    buildingFlower->SetContent(content);

    GetContent("data/Asset/Building/Flower/Texture.png", [=](Content* content) {
      auto tex = content->GetAs<ImagemapContent>()->GetTex();
      if(tex) {
        buildingFlower->ApplyTextureOverride("", tex);
      }
    });
  });

  refptr buildingGrafitti = new Model();
  GetContent("data/Asset/Building/Grafitti/Model.fbx", [=](Content* content) {
    buildingGrafitti->SetContent(content);

    GetContent("data/Asset/Building/Grafitti/Texture.png", [=](Content* content) {
      auto tex = content->GetAs<ImagemapContent>()->GetTex();
      if(tex) {
        buildingGrafitti->ApplyTextureOverride("", tex);
      }
    });
  });

  refptr rhino = new Model();
  GetContent("data/Asset/Rhino.glb", [=](Content* content) {
    rhino->SetContent(content);
  });

  // Define a list of highway models.
  Model* models[] = {
    tree,
    buildingBasic,
    buildingFlower,
    buildingGrafitti,
    rhino,
  };

  // Define a list of highway objects and positions.
  const HighwayObject objects[] = {
    {buildingGrafitti, BuildingScale, -1.0f, 5.0f, 0.0f},
    {buildingFlower, BuildingScale, 0.8f, 7.0f, 90.0f},
    {tree, TreeScale, 1.0f, 10.0f, 0.0f},
    {rhino, RhinoScale, 1.0f, 20.0f, 0.0f},
  };

  size_t objectCount = sizeof(objects) / sizeof(objects[0]);

  bool lastTouchButtonHeld = false;
  f32 touchPressX = 0.0f;
  f32 touchPressY = 0.0f;
  f32 viewAzimuth = ViewAzimuthStart;
  f32 viewAltitude = ViewAltitudeStart;
  f32 viewAzimuthPressed = 0.0f;
  f32 viewAltitudePressed = 0.0f;
  f32 roadPos = 0.0f;
  size_t focusObject = 0;
  f32 focusObjectT = -1.0f;
  f32 focusObjectPosStart = 0.0f;

  ////////////////////////////////////////
  // Main Loop
  ////////////////////////////////////////

  Graphics& g = PxGraphics;
  Keyboard& kb = PxKeyboard;
  Touch& touch = PxTouch;

  g.ShowScreen();
  g.clearScreenColor = Color(0.0f, 0.0f, 0.1f);

  engine.Start();
  while(engine.IsRunning()) {
    f32 dt = engine.StartFrame();

    // Process camera view direction.
    f32 cursorX, cursorY;
    touch.GetMainCursorPos(cursorX, cursorY);
    bool touchButtonHeld = touch.IsButtonHeld(TouchButton1);
    bool touchButtonPressed = !lastTouchButtonHeld && touchButtonHeld;
    lastTouchButtonHeld = touchButtonHeld;

    if(touchButtonPressed) {
      touchPressX = cursorX;
      touchPressY = cursorY;
      viewAzimuthPressed = viewAzimuth;
      viewAltitudePressed = viewAltitude;
    }
    else if(touchButtonHeld) {
      f32 dx = cursorX - touchPressX;
      f32 dy = cursorY - touchPressY;
      viewAzimuth = viewAzimuthPressed + dx * ViewSensitivity;
      viewAltitude = viewAltitudePressed + dy * ViewSensitivity;
    }

    // Process keyboard input.
    f32 moveSpeedScale;
    if(kb.IsKeyHeld(KeyLShift) || kb.IsKeyHeld(KeyRShift)) {
      moveSpeedScale = MoveSpeedScaleFast;
    }
    else {
      moveSpeedScale = 1.0f;
    }

    if(kb.IsKeyPressed(KeyEscape)) {
      roadPos = 0.0f;
    }

    if(objectCount > 0) {
      if(kb.IsKeyPressed(',')) {
        if(focusObject == 0) {
          focusObject = objectCount - 1;
        }
        else {
          focusObject--;
        }
        focusObjectT = 0.0f;
        focusObjectPosStart = roadPos;
      }

      if(kb.IsKeyPressed('.')) {
        if(focusObject >= objectCount - 1) {
          focusObject = 0;
        }
        else {
          focusObject++;
        }
        focusObjectT = 0.0f;
        focusObjectPosStart = roadPos;
      }
    }

    if(kb.IsKeyHeld(KeyUp) || kb.IsKeyHeld('W') || kb.IsKeyHeld(' ')) {
      roadPos += MoveSpeed * moveSpeedScale * dt;
    }
    else if(kb.IsKeyHeld(KeyDown) || kb.IsKeyHeld('S')) {
      roadPos -= MoveSpeed * moveSpeedScale * dt;
    }

    // Update the focus position if it is active.
    if(focusObjectT >= 0.0f) {
      focusObjectT += dt;
      if(focusObjectT > 1.0f) {
        focusObjectT = 1.0f;
      }

      auto& obj = objects[focusObject];

      f32 t = focusObjectT;
      roadPos = GetLerp(focusObjectPosStart, obj.z + FocusObjectOffsetPos, t * t * (3.0f - 2.0f * t));

      if(focusObjectT >= 1.0f) {
        focusObjectT = -1.0f;
      }
    }

    // Draw the scene.
    f32 screenW = g.GetScreenW();
    f32 screenH = g.GetScreenH();
    f32 aspect = screenW / screenH;

    g.ClearScreen();

    g.projection.Push().LoadPerspective(60.0f, aspect, NearZ, FarZ);
    g.view.Push().LoadIdentity()
      .Rotate(viewAltitude, 1.0f, 0.0f, 0.0f)
      .Rotate(viewAzimuth, 0.0f, 1.0f, 0.0f)
      .Translate(0.0f, -ViewHeight, 0.0f);

    scrollTexProgram->SetVariable("scroll", roadPos / RoadRepetitionCount);

    if(grass) {
      auto grassContent = grass->GetImagemapContent();
      if(grassContent) {
        f32 grassW = (f32) grassContent->GetRectW();
        f32 grassH = (f32) grassContent->GetRectH();

        scrollTexProgram->SetVariable("wrapCount", Vec2(RoadRepetitionCount, RoadRepetitionCount));

        g.program.Push() = scrollTexProgram;
        g.model.Push().LoadIdentity()
          .Scale(1.0f / grassH)    // normalize the grass texture size 
          .Rotate(90.0f, -1.0f, 0.0f, 0.0f)
          .Scale(RoadRepetitionCount, RoadRepetitionCount, 1.0f)
          .Translate(-grassW * 0.5f, -grassH * 0.5f);

        grass->Draw();

        g.model.Pop();
        g.program.Pop();
      }
    }

    g.ClearDepth();

    if(road) {
      auto roadContent = road->GetImagemapContent();
      if(roadContent) {
        f32 roadW = (f32) roadContent->GetRectW();
        f32 roadH = (f32) roadContent->GetRectH();

        scrollTexProgram->SetVariable("wrapCount", Vec2(1.0f, RoadRepetitionCount));

        g.program.Push() = scrollTexProgram;
        g.model.Push().LoadIdentity()
          .Scale(1.0f / roadH)    // normalize the road texture size
          .Rotate(90.0f, -1.0f, 0.0f, 0.0f)
          .Scale(1.0f, RoadRepetitionCount, 1.0f)
          .Translate(-roadW * 0.5f, -roadH * 0.5f);

        road->Draw();

        g.model.Pop();
        g.program.Pop();
      }
    }

    for(auto model: models) {
      model->Calc(dt);
    }

    for(auto& obj: objects) {
      auto modelContent = obj.model->GetModelContent();
      if(modelContent) {
        g.program.Push() = modelContent->GetActionCount() > 0 ? modelAnimProgram : modelProgram;
        g.model.Push().LoadIdentity()
          .Translate(obj.x, 0.0f, roadPos - obj.z)
          .Rotate(obj.angle, 0.0f, 1.0f, 0.0f)
          .Scale(obj.scale);

        obj.model->Draw();

        g.model.Pop();
        g.program.Pop();
      }
    }

    g.view.Pop();
    g.projection.Pop();

    engine.EndFrame();
  }

  return 0;
}
