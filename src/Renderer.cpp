#include <iostream>

#include "common.hpp"
#include "Renderer.hpp"
#include "SampleGenerator.hpp"
#include <chrono>
#include <thread>
#include <random>

using namespace std;

void QuickRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray
    SampleGenerator sg{ Vec2(0), Vec2(1), w * h };
    sg.generate();
    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Vec2 p((Real)i/(Real)w-.5, (Real)j/(Real)h-.5);
            Ray r = camera.getRay(p);
            target(i, j) = utils::clamp(tracer.lightAlongRay(r, &sg), 0, 1);
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}

void MultisampleRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    Vec2 pixelSize{1./(Real) w, 1./(Real)h};
    int rejectedSamples = 0;

    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            Vec2 pixelCenter {(Real)i/(Real)w, (Real)j/(Real)h};
            StratifiedSampleGenerator ssg(pixelCenter - pixelSize, pixelCenter + pixelSize, spp);
            ssg.generate();

            SampleGenerator sg(Vec2(0), Vec2(1), spp*3);
            sg.generate();

            vector <Color> samples;
            samples.reserve(spp);

            for (int k = 0; k < spp; ++k) {
              r = camera.getRay(ssg.next());
              samples[k] = tracer.lightAlongRay(r, &sg, false);
            }
            for (auto& c : samples)
              pixelColor += c;
            
            // remove outliers -- calculate stdev
            target(i, j) = utils::clamp(pixelColor / (Real) spp, 0, 1);
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
}


void MultiThreadRenderer::render() {
  int paneWidth = target.width() < 64 ? target.width() : 64;
  int paneHeight = target.height() < 64 ? target.height() : 64;
  int x0 = 0, y0 = 0;
  for (; y0 < target.height()-paneHeight; y0+=paneHeight) {
    for (; x0 < target.width()-paneWidth; x0+=paneWidth) {
      panes.push_back(ImagePane{x0, y0, paneWidth, paneHeight});
    }
    panes.push_back(ImagePane{x0, y0, target.width() - x0, paneHeight});
    x0 = 0;
  }
  for (; x0 < target.width()-paneWidth; x0+=paneWidth) {
    panes.push_back(ImagePane{x0, y0, paneWidth, target.height() - y0});
  }
  panes.push_back(ImagePane{x0, y0, target.width()-x0, target.height()-y0});
  /*  for (auto ip : panes) {
    cout << "x0: " << ip.x0 << " y0: " << ip.y0 << " w: " << ip.w << " h: " << ip.h << endl;
    }*/
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();  

  vector <std::thread> threads;
  for (int i = 0; i < numThreads; ++i) {
    threads.push_back(std::thread(&MultiThreadRenderer::threadFunc, this));
    // Create a cpu_set_t object representing a set of CPUs. Clear it and mark
    // only CPU i as set.
    /*
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset);
    int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
      std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
    */
    threads[i].detach();
  }

  
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();  
  cout << endl << "Thread creation: ";
  std::chrono::milliseconds rt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  cout << utils::format_duration(rt_ms) << endl;

  //  for (auto& thr : threads) 
  //  thr.join();
  {
    std::unique_lock <std::mutex> lk(cv_m);
    cv.wait(lk, [this]{
                  int jobsRemaining = panes.size() - nextJob.load();
                  if (jobsRemaining >= 0) {
                    cout << "\r|";
                    int pctDone = (int) (50. * (panes.size() - jobsRemaining) / (Real) panes.size());
                    for (int i = 0; i < pctDone-1; ++i)
                      cout << "-";
                    cout << ">";
                    for (int i = pctDone; i < 50; ++i)
                      cout << " ";
                    cout << "|" << std::flush;
                  }
                  return threadsRemaining == 0;
                });
  }
  cout << "Count: " << count << endl;
}


void MultiThreadRenderer::renderPane(const ImagePane& ip) {
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    Vec2 pixelSize{1./(Real) target.width(), 1./(Real)target.height()};
    Vec2 pixelCenter;
    int rejectedSamples = 0;
    vector <Vec2> subPixels;
    Color pixelColor;
    Real ans = 0;
    int result = 0;
    for (int j = ip.y0; j < ip.y0+ip.h; ++j) {
        for (int i = ip.x0; i < ip.x0+ip.w; ++i) {
          
          pixelColor = Color::Black();
          pixelCenter = Vec2{(Real)i/(Real)target.width()-.5, (Real)(target.height()-j-1)/(Real)target.height()-.5};
          StratifiedSampleGenerator ssg{pixelCenter - .5*pixelSize, pixelCenter + .5*pixelSize, spp};
          ssg.generate();

          SampleGenerator sg(Vec2(0), Vec2(1), spp);
          sg.generate();
          //          vector <Color> samples;
          //samples.reserve(spp);
          for (int k = 0; k < spp; ++k) {
            r = camera.getRay(ssg.next());
            
            Color pc;
            pc = tracer.lightAlongRay(r, &sg, false);
            pixelColor += utils::clamp(pc, 0, 13);
            //Color pc = Color::Blue();
            //samples.push_back(pc);
            //            pixelColor += Color {(Real) rand() / (Real) RAND_MAX};
            //            pixelColor += Color{(Real)mt_rand() / (Real)RAND_MAX};
            
          }
          
          //          for (auto& c : samples)
          
          target(i, j) = utils::clamp(pixelColor / (Real) spp, 0, 1);
          
        }
    }
    //cout << endl;
    //cout << "Pane finished. Rejected " << rejectedSamples << " samples, " << 100. * (Real) rejectedSamples / (Real) (spp * ip.w * ip.h) << " pct. " << endl;
}

void MultiThreadRenderer::threadFunc() {
  // this function atomically checks nextJob. If it's less than the number of jobs, we advance the
  // job pointer and do the job. Otherwise, terminate.
  int job;
  while (true) {
    job = nextJob.fetch_add(1);
    if (job < panes.size()) {
      renderPane(panes[job]);
      cv.notify_all();
    }
    else {
      break;
    }
  }
  //cout << "Reporting done." << endl;
  threadsRemaining--;
  cv.notify_all();
}

void DebugRenderer::render() {
    int w = 512;
    int h = 512;
    
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    Vec2 pixelSize{1./(Real) w, 1./(Real)h};
    Vec2 p((Real)100/(Real)w, (Real)259/(Real)h); // center of pixel
    r = camera.getRay(p - Vec2(.5));
    SampleGenerator sg{ Vec2(0), Vec2(1), 10 };
    sg.generate();
    Color pixelColor = tracer.lightAlongRay(r, &sg, true);
    // 0.228245, 0.004493, 0.973594
    cout << "Pixel color: " << pixelColor << endl;
    //printf("\b\b\b\b\b\b\b\b\b");
    
    cout << endl;
    cout << "Called render" << endl;
}
