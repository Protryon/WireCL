#version 430 core
attribute vec2 texCoord;
out vec2 uv;
attribute int layer;
out int l2;
void main(void) {
  uv = texCoord;
  l2 = layer;
}