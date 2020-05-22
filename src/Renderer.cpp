#include <iostream>

#include "common.hpp"
#include "Renderer.hpp"
#include "SampleGenerator.hpp"
#include <chrono>
#include <thread>

using namespace std;

void QuickRenderer::render() {
    int w = target.width();
    int h = target.height();
    
    // one ray for each pixel in the result image
    // one sample along each ray

    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Vec2 p((Real)i/(Real)w-.5, (Real)j/(Real)h-.5);
            Ray r = camera.getRay(p);
            target(i, j) = utils::clamp(tracer.lightAlongRay(r), 0, 1);
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
    StratifiedSampleGenerator ssg;
    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            Vec2 pixelCenter {(Real)i/(Real)w, (Real)j/(Real)h};
            vector <Vec2> subPixels = ssg.generate(pixelCenter - pixelSize, pixelCenter + pixelSize, spp);
            Color samples[subPixels.size()];

            for (int k = 0; k < subPixels.size(); ++k) {
              r = camera.getRay(subPixels[k]);
              samples[k] = tracer.lightAlongRay(r, false);
            }
            for (auto& c : samples)
              pixelColor += c;
            
            // remove outliers -- calculate stdev
            target(i, j) = utils::clamp(pixelColor / (Real) subPixels.size(), 0, 1);

        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
}

std::string format_duration( std::chrono::milliseconds ms ) {
    using namespace std::chrono;
    auto secs = duration_cast<seconds>(ms);
    ms -= duration_cast<milliseconds>(secs);
    auto mins = duration_cast<minutes>(secs);
    secs -= duration_cast<seconds>(mins);
    auto hour = duration_cast<hours>(mins);
    mins -= duration_cast<minutes>(hour);

    std::stringstream ss;
    ss << hour.count() << "h " << mins.count() << "m " << secs.count() << "s."; // << ms.count() << " Milliseconds";
    return ss.str();
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
  for (int i = 0; i < numThreads; ++i) {
    std::thread(&MultiThreadRenderer::threadFunc, this).detach();
  }

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
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();  
  cout << endl << "Render complete." << endl;
  std::chrono::milliseconds rt_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  cout << format_duration(rt_ms) << endl;
}

void MultiThreadRenderer::renderPane(const ImagePane& ip) {
    // one ray for each pixel in the result image
    // one sample along each ray
    Ray r;
    Vec2 pixelSize{1./(Real) target.width(), 1./(Real)target.height()};
    int rejectedSamples = 0;
    StratifiedSampleGenerator ssg;
    for (int j = ip.y0; j < ip.y0+ip.h; ++j) {
        for (int i = ip.x0; i < ip.x0+ip.w; ++i) {
            Color pixelColor = Color::Black();
            Vec2 pixelCenter {(Real)i/(Real)target.width()-.5, (Real)j/(Real)target.height()-.5};
            vector <Vec2> subPixels = ssg.generate(pixelCenter - .5*pixelSize, pixelCenter + .5*pixelSize, spp);
            vector <Color> samples;
            samples.reserve(subPixels.size());
            for (int k = 0; k < subPixels.size(); ++k) {
              r = camera.getRay(subPixels[k]);
              Color pc = tracer.lightAlongRay(r, false);

              if ((i == 10 || i == 480) && j == 250 && k == 0) {
                cout << "i: " << i << "   j: " << j << endl;
                cout << Vec3(subPixels[k], 0) << endl;
                cout << r << endl;
                cout << "color: " << pc << endl;
              }
              samples.push_back(pc);
              //r = camera.getRay(pixelCenter);
            }
            //utils::winsorize(samples, .95);
            for (auto& c : samples)
              pixelColor += utils::clamp(c, 0, 13);
                        
            target(i, j) = utils::clamp(pixelColor / (Real) subPixels.size(), 0, 1);
            //            if (target(i, j).g > .2) cout << "(i, j): (" << i << ", " << j << ")\tray: " << r << "\tcolor: " << target(i,j) << endl;
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
    Color pixelColor = tracer.lightAlongRay(r, true);
    // 0.228245, 0.004493, 0.973594
    cout << "Pixel color: " << pixelColor << endl;
    //printf("\b\b\b\b\b\b\b\b\b");
    
    cout << endl;
    cout << "Called render" << endl;
}
