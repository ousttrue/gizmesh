#pragma once
// interface for gizmesh example
#include <memory>
#include <stdint.h>

class Model {
  class ModelImpl *m_impl = nullptr;

public:
  Model(ModelImpl *impl);
  ~Model();
  void uploadMesh(void *device, const void *vertices, uint32_t verticesSize,
                  uint32_t vertexStride, const void *indices,
                  uint32_t indicesSize, uint32_t indexSize, bool is_dynamic);
  void draw(void *context, const float *model, const float *vp,
            const float *eye);
};

class Renderer {
  class RendererImpl *m_impl = nullptr;

public:
  Renderer();
  ~Renderer();
  void *initialize(void *hwnd);
  std::shared_ptr<Model> createMesh();
  void *beginFrame(int width, int height);
  void endFrame();
  void clearDepth();
};
