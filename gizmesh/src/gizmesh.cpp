#include "gizmesh.h"
#include "geometry_mesh.h"

#include "impl.h"
#include <assert.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace gizmesh {

GizmoSystem::GizmoSystem() : m_impl(new gizmo_system_impl) {}

GizmoSystem::~GizmoSystem() { delete m_impl; }

void GizmoSystem::begin(const std::array<float, 3> &camera_position,
                        const std::array<float, 4> &camera_rotation,
                        const std::array<float, 3> &ray_origin,
                        const std::array<float, 3> &ray_direction,
                        bool button) {
  m_impl->update(
      {camera_position, camera_rotation, ray_origin, ray_direction, button});
}

GizmoSystem::Buffer GizmoSystem::end() {
  auto &r = m_impl->render();
  return {
      (uint8_t *)r.vertices.data(),
      static_cast<uint32_t>(r.vertices.size() * sizeof(r.vertices[0])),
      static_cast<uint32_t>(sizeof(r.vertices[0])),
      (uint8_t *)r.triangles.data(),
      static_cast<uint32_t>(r.triangles.size() * sizeof(r.triangles[0])),
      static_cast<uint32_t>(sizeof(r.triangles[0])),
  };
}

// 32 bit FNV Hash
uint32_t hash_fnv1a(const std::string &str) {
  static const uint32_t fnv1aBase32 = 0x811C9DC5u;
  static const uint32_t fnv1aPrime32 = 0x01000193u;

  uint32_t result = fnv1aBase32;

  for (auto &c : str) {
    result ^= static_cast<uint32_t>(c);
    result *= fnv1aPrime32;
  }
  return result;
}

} // namespace gizmesh
