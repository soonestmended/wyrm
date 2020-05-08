#include <iostream>

#include "common.hpp"
#include "Renderer.hpp"

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

    for (int j = 0; j < h; ++j) {
        printf("row: %4d", j);
        fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            Color samples[spp];
            for (int k = 0; k < spp; ++k) {
                Vec2 p((Real)i/(Real)w, (Real)j/(Real)h); // center of pixel
                p += (utils::rand01vec2() - Vec2(.5)) * pixelSize;
                r = camera.getRay(p - Vec2(.5));
                if (j == 259 && i == 109) {
                    samples[k] = tracer.lightAlongRay(r, false);
                }
                else {
                    samples[k] = tracer.lightAlongRay(r, false);
                }
            }
            // remove outliers -- calculate stdev
            Color mean{0};
            for (int k = 0; k < spp; k++) {
                mean += samples[k];
            }
            mean /= (Real) spp;
            Color stdev{0};
            for (int k = 0; k < spp; k++) {
                stdev += (samples[k] - mean) * (samples[k] - mean);
            }
            stdev = glm::sqrt(stdev / (Real) spp);
            int usedSamples = 0;
            
            for (int k = 0; k < spp; k++) {
                Color foo = Color::abs(samples[k] - mean);
                if (foo.allComponentsLessThanEqualTo(5 * stdev)) {
                    pixelColor += samples[k];
                    usedSamples++;
                }
                else {
                    rejectedSamples++;
                }
            }
            //cout << "Used samples: " << usedSamples << endl;
            /*
            if (j == 50) {
                cout << "Mean: " << mean << "\tStdev: " << stdev << endl;
                cout << "Samples used: " << usedSamples << endl;
            }
*/
            target(i, j) = utils::clamp(pixelColor / (Real) usedSamples, 0, 1);
            //if (utils::avg(pixelColor) > 10.0) cout << "col: " << i << "\tray: " << r << "\tcolor: " << pixelColor << endl;
        }
        printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Rejected " << rejectedSamples << " samples, " << 100. * (Real) rejectedSamples / (Real) (spp * w * h) << " pct. " << endl;
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

    for (int j = ip.y0; j < ip.y0+ip.h; ++j) {
        for (int i = ip.x0; i < ip.x0+ip.w; ++i) {
            Color pixelColor = Color::Black();
            Color samples[spp];
            for (int k = 0; k < spp; ++k) {
              Vec2 p((Real)i/(Real)target.width(), (Real)j/(Real)target.height()); // center of pixel
                p += (utils::rand01vec2() - Vec2(.5)) * pixelSize;
                r = camera.getRay(p - Vec2(.5));
                samples[k] = tracer.lightAlongRay(r, false);
            }
            // remove outliers -- calculate stdev
            Color mean{0};
            for (int k = 0; k < spp; k++) {
                mean += samples[k];
            }
            mean /= (Real) spp;
            Color stdev{0};
            for (int k = 0; k < spp; k++) {
                stdev += (samples[k] - mean) * (samples[k] - mean);
            }
            stdev = glm::sqrt(stdev / (Real) spp);
            int usedSamples = 0;
            
            for (int k = 0; k < spp; k++) {
                Color foo = Color::abs(samples[k] - mean);
                if (foo.allComponentsLessThanEqualTo(5 * stdev)) {
                    pixelColor += samples[k];
                    usedSamples++;
                }
                else {
                    rejectedSamples++;
                }
            }
            //cout << "Used samples: " << usedSamples << endl;
            /*
            if (j == 50) {
                cout << "Mean: " << mean << "\tStdev: " << stdev << endl;
                cout << "Samples used: " << usedSamples << endl;
            }
*/
            target(i, j) = utils::clamp(pixelColor / (Real) usedSamples, 0, 1);
            //if (utils::avg(pixelColor) > 10.0) cout << "col: " << i << "\tray: " << r << "\tcolor: " << pixelColor << endl;
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
    for (int j = 0; j < h; ++j) {
        //printf("row: %4d", j);
        //fflush(stdout);
        for (int i = 0; i < w; ++i) {
            Color pixelColor = Color::Black();
            Vec2 p((Real)109/(Real)w, (Real)259/(Real)h); // center of pixel
            p += (utils::rand01vec2() - Vec2(.5)) * pixelSize;
            r = camera.getRay(p - Vec2(.5));
            pixelColor = tracer.lightAlongRay(r);
            // 0.228245, 0.004493, 0.973594
            if (j == 259 && i == 109)
                cout << "Ray: " << r << "\tColor: " << pixelColor << endl;
        }
        //printf("\b\b\b\b\b\b\b\b\b");
    }
    cout << endl;
    cout << "Called render" << endl;
}
