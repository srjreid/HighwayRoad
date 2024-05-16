#version 410

in vec2 vPos;
in vec2 vUV;

out vec2 tc;

uniform ShaderUniformBlock {
  mat4 mvp;
  vec2 wrapCount;
  float scroll;
};

void main() {
  vec4 pos = mvp * vec4(vPos, 0.0, 1.0);
  tc = vec2(vUV.x * wrapCount.x, (vUV.y - scroll) * wrapCount.y);
  gl_Position = pos;
}
