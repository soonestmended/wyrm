#pragma once


#include "Camera.hpp"
#include "Image.hpp"
#include "PBRTParser.hpp"
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Tracer.hpp"

class InteractiveSession {
	//{ c, s, pt, foo, o };
public:
	InteractiveSession(Camera& _c, Scene& _s, Tracer& _t, Image& _i, Options& _o) :
		camera(_c), scene(_s), tracer(_t), image(_i), options(_o) {}

	void go();
	void initRenderTexture(int width, int height);
	void updateFrame();
	void updateTextureIfNecessary();
	bool frameNeedsUpdate = false, rendererNeedsUpdate = false;

	unsigned int sketchFramebuffer, sketchTexture, depthrenderbuffer;
	
	Camera& camera;
	Scene& scene;
	Tracer& tracer;
	Image& image;
	Options& options;
	std::shared_ptr <MultiThreadRenderer> mtr = nullptr;
};