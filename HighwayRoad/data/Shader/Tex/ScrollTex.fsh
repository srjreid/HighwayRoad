#version 410

in vec2 tc;

out vec4 color;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec2 wrapCount;
  float scroll;
};

uniform sampler2D tex;

void main() {
  color = texture2D(tex, tc);
}
