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

#pragma once

#include <Prime/Config.h>
#if defined(PrimeTargetOpenGL)

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <Prime/Input/Touch.h>
#include <Prime/Graphics/opengl/OpenGLInc.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

#define PxOpenGLTouch OpenGLTouch::GetInstance()

////////////////////////////////////////////////////////////////////////////////
// Structs
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

typedef struct _OpenGLTouchInfo {
  f32 x;
  f32 y;
  bool button1;
  bool button2;
} OpenGLTouchInfo;

};

////////////////////////////////////////////////////////////////////////////////
// Classes
////////////////////////////////////////////////////////////////////////////////

namespace Prime {

class OpenGLTouch: public Touch {
public:

  static OpenGLTouch& GetInstance();

protected:

  OpenGLTouchInfo oldTouchInfo;
  bool oldTouchInfoKnown;

  bool ogl2CursorVisible;

public:

  OpenGLTouch();
  ~OpenGLTouch();

protected:

  void Init() override;
  void Shutdown() override;

public:

  void StartFrame() override;

  bool GetMainCursorPos(f32& x, f32& y) const override;
  bool IsButtonHeld(TouchButton button) const override;

  virtual void OnScroll(GLFWwindow* window, double x, double y);

protected:

  OpenGLTouchInfo GetOpenGLTouchInfo() const;

};

};

#endif
