// Copyright 2021 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

// Note that this version of the tutorial directly links to
// libanari_library_example, which exports a function to directly
// construct the "example" devic. This lets you avoid the need to load a
// module first (which opens a shared library at runtime). Thus the only
// difference between this and 'anariTutorial.c' is how the device is created.

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
// anari
#include "anari/devices/example.h"
// stb_image
#include "stb_image_write.h"

const char *g_moduleType = "example";

void statusFunc(void *userData,
    ANARIDevice device,
    ANARIObject source,
    ANARIDataType sourceType,
    ANARIStatusSeverity severity,
    ANARIStatusCode code,
    const char *message)
{
  (void)userData; // unused
  if (severity == ANARI_SEVERITY_FATAL_ERROR) {
    fprintf(stderr, "[FATAL] %s\n", message);
  } else if (severity == ANARI_SEVERITY_ERROR) {
    fprintf(stderr, "[ERROR] %s\n", message);
  } else if (severity == ANARI_SEVERITY_WARNING) {
    fprintf(stderr, "[WARN ] %s\n", message);
  } else if (severity == ANARI_SEVERITY_PERFORMANCE_WARNING) {
    fprintf(stderr, "[PERF ] %s\n", message);
  } else if (severity == ANARI_SEVERITY_INFO) {
    fprintf(stderr, "[INFO] %s\n", message);
  }
}

int main(int argc, const char **argv)
{
  stbi_flip_vertically_on_write(1);

  // image size
  int imgSize[2] = {1024 /*width*/, 768 /*height*/};

  // camera
  float cam_pos[] = {0.f, 0.f, 0.f};
  float cam_up[] = {0.f, 1.f, 0.f};
  float cam_view[] = {0.1f, 0.f, 1.f};

  // triangle mesh array
  float vertex[] = {-1.0f,
      -1.0f,
      3.0f,
      -1.0f,
      1.0f,
      3.0f,
      1.0f,
      -1.0f,
      3.0f,
      0.1f,
      0.1f,
      0.3f};
  float color[] = {0.9f,
      0.5f,
      0.5f,
      1.0f,
      0.8f,
      0.8f,
      0.8f,
      1.0f,
      0.8f,
      0.8f,
      0.8f,
      1.0f,
      0.5f,
      0.9f,
      0.5f,
      1.0f};
  int32_t index[] = {0, 1, 2, 1, 2, 3};

  printf("initialize ANARI...");

  // Here we use the direct function which creates the reference device instead
  // of loading it as a module at runtime.
  ANARIDevice dev = anariNewExampleDevice();

  if (!dev) {
    printf("\n\nERROR: could not load default device in module %s\n",
        g_moduleType);
    return 1;
  }

  ANARIStatusCallback statusFuncPtr = &statusFunc;
  anariSetParameter(
      dev, dev, "statusCallback", ANARI_STATUS_CALLBACK, &statusFuncPtr);

  // commit device
  anariCommit(dev, dev);

  printf("done!\n");
  printf("setting up camera...");

  // create and setup camera
  ANARICamera camera = anariNewCamera(dev, "perspective");
  float aspect = imgSize[0] / (float)imgSize[1];
  anariSetParameter(dev, camera, "aspect", ANARI_FLOAT32, &aspect);
  anariSetParameter(dev, camera, "position", ANARI_FLOAT32_VEC3, cam_pos);
  anariSetParameter(dev, camera, "direction", ANARI_FLOAT32_VEC3, cam_view);
  anariSetParameter(dev, camera, "up", ANARI_FLOAT32_VEC3, cam_up);
  anariCommit(dev, camera); // commit each object to indicate mods are done

  printf("done!\n");
  printf("setting up scene...");

  // The world to be populated with renderable objects
  ANARIWorld world = anariNewWorld(dev);

  // create and setup surface and mesh
  //   A mesh requires an index, plus arrays of: locations & colors
  ANARIGeometry mesh = anariNewGeometry(dev, "triangle");

  // Set the vertex locations
  ANARIArray1D array =
      anariNewArray1D(dev, vertex, 0, 0, ANARI_FLOAT32_VEC3, 4, 0);
  anariCommit(dev, array);
  anariSetParameter(dev, mesh, "vertex.position", ANARI_ARRAY1D, &array);
  anariRelease(dev, array); // we are done using this handle

  // Set the vertex colors
  array = anariNewArray1D(dev, color, 0, 0, ANARI_FLOAT32_VEC4, 4, 0);
  anariCommit(dev, array);
  anariSetParameter(dev, mesh, "vertex.color", ANARI_ARRAY1D, &array);
  anariRelease(dev, array);

  // Set the index
  array = anariNewArray1D(dev, index, 0, 0, ANARI_UINT32_VEC3, 2, 0);
  anariCommit(dev, array);
  anariSetParameter(dev, mesh, "primitive.index", ANARI_ARRAY1D, &array);
  anariRelease(dev, array);

  // Affect all the mesh values
  anariCommit(dev, mesh);

  // Set the material rendering parameters
  ANARIMaterial mat = anariNewMaterial(dev, "matte");
  anariCommit(dev, mat);

  // put the mesh into a surface
  ANARISurface surface = anariNewSurface(dev);
  anariSetParameter(dev, surface, "geometry", ANARI_GEOMETRY, &mesh);
  anariSetParameter(dev, surface, "material", ANARI_MATERIAL, &mat);
  anariCommit(dev, surface);
  anariRelease(dev, mesh);
  anariRelease(dev, mat);

  // put the geometry into an instance (give the geometry a world transform)
  ANARIInstance instance = anariNewInstance(dev);
  array = anariNewArray1D(dev, &surface, 0, 0, ANARI_SURFACE, 1, 0);
  anariCommit(dev, array);
  anariSetParameter(dev, world, "surface", ANARI_ARRAY1D, &array);
  anariRelease(dev, surface);
  anariRelease(dev, array);

  // create and setup light for Ambient Occlusion
  ANARILight light = anariNewLight(dev, "ambient");
  anariCommit(dev, light);
  array = anariNewArray1D(dev, &light, 0, 0, ANARI_LIGHT, 1, 0);
  anariCommit(dev, array);
  anariSetParameter(dev, world, "light", ANARI_ARRAY1D, &array);
  anariRelease(dev, light);
  anariRelease(dev, array);

  anariCommit(dev, world);

  printf("done!\n");

  // print out world bounds
  float worldBounds[6];
  if (anariGetProperty(dev,
          world,
          "bounds",
          ANARI_FLOAT32_BOX3,
          worldBounds,
          sizeof(worldBounds),
          ANARI_WAIT)) {
    printf("\nworld bounds: ({%f, %f, %f}, {%f, %f, %f}\n\n",
        worldBounds[0],
        worldBounds[1],
        worldBounds[2],
        worldBounds[3],
        worldBounds[4],
        worldBounds[5]);
  } else {
    printf("\nworld bounds not returned\n\n");
  }

  printf("setting up renderer...");

  // create renderer
  ANARIRenderer renderer = anariNewRenderer(dev, "default");

  // complete setup of renderer
  float bgColor[4] = {1.f, 1.f, 1.f, 1.f}; // white
  anariSetParameter(
      dev, renderer, "backgroundColor", ANARI_FLOAT32_VEC4, bgColor);
  anariCommit(dev, renderer);

  // create and setup frame
  ANARIFrame frame = anariNewFrame(dev);
  anariSetParameter(dev, frame, "size", ANARI_UINT32_VEC2, imgSize);
  ANARIDataType fbFormat = ANARI_UFIXED8_RGBA_SRGB;
  anariSetParameter(dev, frame, "color", ANARI_DATA_TYPE, &fbFormat);

  anariSetParameter(dev, frame, "renderer", ANARI_RENDERER, &renderer);
  anariSetParameter(dev, frame, "camera", ANARI_CAMERA, &camera);
  anariSetParameter(dev, frame, "world", ANARI_WORLD, &world);

  anariCommit(dev, frame);

  printf("rendering initial frame to firstFrame.png...");

  // render one frame
  anariRenderFrame(dev, frame);
  anariFrameReady(dev, frame, ANARI_WAIT);

  // access frame and write its content as PNG file
  const uint32_t *fb = (uint32_t *)anariMapFrame(dev, frame, "color");
  stbi_write_png(
      "firstFrame.png", imgSize[0], imgSize[1], 4, fb, 4 * imgSize[0]);
  anariUnmapFrame(dev, frame, "color");

  printf("done!\n");
  printf("rendering 10 accumulated frames to accumulatedFrame.png...");

  // render 10 more frames, which are accumulated to result in a better
  //   converged image
  for (int frames = 0; frames < 10; frames++) {
    anariRenderFrame(dev, frame);
    anariFrameReady(dev, frame, ANARI_WAIT);
  }

  fb = (uint32_t *)anariMapFrame(dev, frame, "color");
  stbi_write_png(
      "accumulatedFrame.png", imgSize[0], imgSize[1], 4, fb, 4 * imgSize[0]);
  anariUnmapFrame(dev, frame, "color");

  printf("done!\n");
  printf("\ncleaning up objects...");

  // final cleanups
  anariRelease(dev, renderer);
  anariRelease(dev, camera);
  anariRelease(dev, frame);
  anariRelease(dev, world);

  anariRelease(dev, dev);

  printf("done!\n");

  return 0;
}
