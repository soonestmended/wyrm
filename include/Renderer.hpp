#pragma once

#include "Camera.hpp"
#include "Image.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

#include <atomic>
#include <condition_variable>
#include <thread>
#include <vector>

class Renderer {
public:    
    Renderer(const Camera& c, const Scene& s, const Tracer& t) : camera (c), scene (s), tracer (t) {}
    virtual void render() = 0;

protected: 
    const Camera& camera;
    const Scene& scene;
    const Tracer& tracer;
};

class DebugRenderer : public Renderer {
public:
    DebugRenderer(const Camera& c, const Scene& s, const Tracer& t) : 
        Renderer(c, s, t) {}
    void render();
};

class QuickRenderer : public Renderer {
public:
    QuickRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &target_) : 
        Renderer(c, s, t),
        target (target_) {}
    void render();

private:
    Image &target;
};

class MultisampleRenderer : public Renderer {
public:
    MultisampleRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &_target, int _spp) : 
        Renderer(c, s, t),
        target (_target),
        spp (_spp) {}
    void render();

protected:
    Image &target;
    int spp; // samples per pixel
};

class MultiThreadRenderer : public MultisampleRenderer {
public:
    MultiThreadRenderer(const Camera& c, const Scene& s, const Tracer& t, Image &_target, int _spp, int _threads) :
      MultisampleRenderer(c, s, t, _target, _spp), numThreads(_threads), threadsRemaining(_threads) {}
    void render();

protected:
  struct ImagePane {
    int x0, y0, w, h;
  };

  std::vector <std::thread> threads;
  void threadFunc();
  void renderPane(const ImagePane& ip);
  int numThreads;
  std::atomic <int> nextJob = 0, count = 0, threadsRemaining;
  std::vector <ImagePane> panes;
  std::condition_variable cv;
  std::mutex cv_m;
};

