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
#include <Prime/Font/Font.h>
#include <Prime/Model/Model.h>

using namespace Prime;

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
  refptr modelProgram = DeviceProgram::Create("data/Shader/Model/Model.vsh", "data/Shader/Model/Model.fsh");
  refptr modelAnimProgram = DeviceProgram::Create("data/Shader/Model/ModelAnim.vsh", "data/Shader/Model/ModelAnim.fsh");

  // Load assets.
  refptr rhino = new Model();
  GetContent("data/Asset/Rhino.glb", [=](Content* content) {
    rhino->SetContent(content);
  });

  ////////////////////////////////////////
  // Main Loop
  ////////////////////////////////////////

  Graphics& g = PxGraphics;

  g.ShowScreen();
  g.clearScreenColor = Color(0.0f, 0.0f, 0.1f);

  engine.Start();
  while(engine.IsRunning()) {
    f32 dt = engine.StartFrame();
    f32 screenW = g.GetScreenW();
    f32 screenH = g.GetScreenH();
    f32 aspect = screenW / screenH;

    g.ClearScreen();

    if(rhino) {
      rhino->Calc(dt);

      const Vec3& vertexMin = rhino->GetVertexMin();
      const Vec3& vertexMax = rhino->GetVertexMax();

      f32 sizeX = vertexMax.x - vertexMin.x;
      f32 sizeY = vertexMax.y - vertexMin.y;
      f32 sizeZ = vertexMax.z - vertexMin.z;
      f32 size = max(max(sizeX, sizeY), sizeZ);

      g.program.Push() = modelAnimProgram;
      g.projection.Push().LoadPerspective(60.0f, aspect, size * 0.1f, size * 20.0f);
      g.view.Push().LoadTranslation(0.0f, -size * 0.5f, -size * 2.0f)
        .Rotate(25.0f, 1.0f, 0.0f, 0.0f)
        .Rotate(35.0f, 0.0f, 1.0f, 0.0f);

      rhino->Draw();

      g.view.Pop();
      g.projection.Pop();
      g.program.Pop();
    }

    engine.EndFrame();
  }

  return 0;
}
