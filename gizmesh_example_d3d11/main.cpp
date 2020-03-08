﻿#include "renderer.h"
#include "teapot.h"
#include "Win32Window.h"
#include "OrbitCamera.h"
#include <vector>
#include <gizmesh.h>

struct vertex
{
    std::array<float, 3> position;
    std::array<float, 3> normal;
    std::array<float, 4> color;
};
static_assert(sizeof(vertex) == 40, "vertex size");

enum class transform_mode
{
    translate,
    rotate,
    scale
};

#include <vector>
#include <ranges>
#include <iostream>

//////////////////////////
//   Main Application   //
//////////////////////////
int main(int argc, char *argv[])
{
    screenstate::Win32Window win(L"tinygizmo_example_class");
    auto hwnd = win.Create(L"tinygizmo_example_d3d11", 1280, 800);
    if (!hwnd)
    {
        return 1;
    }
    win.Show();

    // camera
    OrbitCamera camera(PerspectiveTypes::D3D);
    transform_mode mode = transform_mode::scale;
    bool is_local = true;

    //
    // scene
    //
    Renderer renderer;
    auto device = renderer.initialize(hwnd);
    if (!device)
    {
        return 3;
    }

    // create teapot
    auto teapot_mesh = renderer.createMesh();
    std::vector<vertex> vertices;
    for (int i = 0; i < 4974; i += 6)
    {
        vertex v{
            .position{
                teapot_vertices[i + 0],
                teapot_vertices[i + 1],
                teapot_vertices[i + 2]},
            .normal{
                teapot_vertices[i + 3],
                teapot_vertices[i + 4],
                teapot_vertices[i + 5]}};
        vertices.push_back(v);
    }
    teapot_mesh->uploadMesh(device,
                            vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(vertex)), sizeof(vertex),
                            teapot_triangles, sizeof(teapot_triangles), sizeof(teapot_triangles[0]),
                            false);

    // teapot a
    fpalg::TRS teapot_a;
    teapot_a.position = {-2, 0, 0};

    // teapot b
    fpalg::TRS teapot_b;
    teapot_b.position = {+2, 0, 0};

    // gizmo
    gizmesh::GizmoSystem system;
    auto gizmo_mesh = renderer.createMesh();

    //
    // main loop
    //
    screenstate::ScreenState state;
    std::bitset<128> lastState{};
    for (int i = 0; win.Update(&state); ++i)
    {
        // update camera
        camera.WindowInput(state);

        // gizmo new frame
        system.new_frame(
            camera.state.position, 
            camera.state.rotation,
            camera.state.ray_origin,
            camera.state.ray_direction,
            state.MouseLeftDown()
            );

        if (state.KeyCode['R'])
        {
            mode = transform_mode::rotate;
        }
        if (state.KeyCode['T'])
        {
            mode = transform_mode::translate;
        }
        if (state.KeyCode['S'])
        {
            mode = transform_mode::scale;
        }
        if (!lastState['Z'] && state.KeyCode['Z'])
        {
            is_local = !is_local;
        }
        lastState = state.KeyCode;

        using fpalg::operator*;
        auto viewProjection = camera.state.view * camera.state.projection;

        //
        // draw
        //
        auto context = renderer.beginFrame(state.Width, state.Height);
        teapot_mesh->draw(context, teapot_a.Matrix().data(), viewProjection.data(), camera.state.position.data());
        teapot_mesh->draw(context, teapot_b.Matrix().data(), viewProjection.data(), camera.state.position.data());

        {
            //
            // manipulate and update gizmo
            //
            switch (mode)
            {
            case transform_mode::translate:
                gizmesh::handle::translation(system, gizmesh::hash_fnv1a("first-example-gizmo"), teapot_a, is_local);
                gizmesh::handle::translation(system, gizmesh::hash_fnv1a("second-example-gizmo"), teapot_b, is_local);
                break;

            case transform_mode::rotate:
                gizmesh::handle::rotation(system, gizmesh::hash_fnv1a("first-example-gizmo"), teapot_a, is_local);
                gizmesh::handle::rotation(system, gizmesh::hash_fnv1a("second-example-gizmo"), teapot_b, is_local);
                break;

            case transform_mode::scale:
                gizmesh::handle::scale(system, gizmesh::hash_fnv1a("first-example-gizmo"), teapot_a, is_local);
                gizmesh::handle::scale(system, gizmesh::hash_fnv1a("second-example-gizmo"), teapot_b, is_local);
                break;
            }

            void *pVertices;
            uint32_t verticesBytes;
            uint32_t vertexStride;
            void *pIndices;
            uint32_t indicesBytes;
            uint32_t indexStride;
            system.render(
                &pVertices, &verticesBytes, &vertexStride,
                &pIndices, &indicesBytes, &indexStride);

            gizmo_mesh->uploadMesh(device,
                                   pVertices, verticesBytes, vertexStride,
                                   pIndices, indicesBytes, indexStride,
                                   true);
        }

        //
        // gizmo after xform user draw
        //
        renderer.clearDepth();
        static const float identity4x4[] = {
            1, 0, 0, 0, //
            0, 1, 0, 0, //
            0, 0, 1, 0, //
            0, 0, 0, 1, //
        };
        gizmo_mesh->draw(context, identity4x4, viewProjection.data(), camera.state.position.data());

        //
        // present
        //
        renderer.endFrame();
    }
    return EXIT_SUCCESS;
}
