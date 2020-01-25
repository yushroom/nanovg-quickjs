//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <stdio.h>
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "nanovg.h"
#include "nanovg_mtl.h"
#include "demo.h"
#include "perf.h"

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include <nanovg_qjs.h>


void errorcb(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}

int blowup = 0;
int screenshot = 0;
int premult = 0;

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, TRUE);
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		blowup = !blowup;
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
		screenshot = 1;
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
		premult = !premult;
}

extern NVGcontext* g_NVGcontext;

int main()
{
	GLFWwindow* window;
	DemoData data;
	NVGcontext* vg = NULL;
	GPUtimer gpuTimer;
	PerfGraph fps, cpuGraph, gpuGraph;
	double prevt = 0, cpuTime = 0;

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}

	initGraph(&fps, GRAPH_RENDER_FPS, "Frame Time");
	initGraph(&cpuGraph, GRAPH_RENDER_MS, "CPU Time");
	initGraph(&gpuGraph, GRAPH_RENDER_MS, "GPU Time");

	glfwSetErrorCallback(errorcb);

#ifdef DEMO_MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);
#endif
	// Create window with graphics context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(1000, 600, "NanoVG", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	
	id <MTLDevice> device = MTLCreateSystemDefaultDevice();;

	NSWindow *nswin = glfwGetCocoaWindow(window);
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
//	layer.displaySyncEnabled = false;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;
	
	glfwSetKeyCallback(window, key);

	vg = nvgCreateMTL((__bridge void*)layer, NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return -1;
	}
	g_NVGcontext = vg;

	if (loadDemoData(vg, &data) == -1)
		return -1;

	initGPUTimer(&gpuTimer);

	glfwSetTime(0);
	prevt = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		@autoreleasepool
        {
			double mx, my, t, dt;
			int winWidth, winHeight;
			int fbWidth, fbHeight;
			float pxRatio;
			float gpuTimes[3];
			int i, n;

			t = glfwGetTime();
			dt = t - prevt;
			prevt = t;

			startGPUTimer(&gpuTimer);

			glfwGetCursorPos(window, &mx, &my);
			glfwGetWindowSize(window, &winWidth, &winHeight);
			glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
			// Calculate pixel ration for hi-dpi devices.
			pxRatio = (float)fbWidth / (float)winWidth;

			// Update and render
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);
			layer.drawableSize = CGSizeMake(width, height);
			
			NVGcolor clear_color = {.r=0.3f, .g=0.3f, .b=0.32f, .a=1.00f};
			mnvgClearWithColor(vg, clear_color);
			
			nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

			renderDemo(vg, mx,my, winWidth,winHeight, t, blowup, &data);
//			RunFile("/Users/yushroom/program/qjs/nanovg-quickjs/src/nvg.js");

			renderGraph(vg, 5,5, &fps);
			renderGraph(vg, 5+200+5,5, &cpuGraph);
			if (gpuTimer.supported)
				renderGraph(vg, 5+200+5+200+5,5, &gpuGraph);

			nvgEndFrame(vg);

			// Measure the CPU time taken excluding swap buffers (as the swap may wait for GPU)
			cpuTime = glfwGetTime() - t;

			updateGraph(&fps, dt);
			updateGraph(&cpuGraph, cpuTime);

			// We may get multiple results.
			n = stopGPUTimer(&gpuTimer, gpuTimes, 3);
			for (i = 0; i < n; i++)
				updateGraph(&gpuGraph, gpuTimes[i]);
			
			if (screenshot) {
				screenshot = 0;
	//			saveScreenShot(fbWidth, fbHeight, premult, "dump.png");
			}
			
			glfwPollEvents();
		}
	}

	freeDemoData(vg, &data);

	nvgDeleteMTL(vg);

	printf("Average Frame Time: %.2f ms\n", getGraphAverage(&fps) * 1000.0f);
	printf("          CPU Time: %.2f ms\n", getGraphAverage(&cpuGraph) * 1000.0f);
	printf("          GPU Time: %.2f ms\n", getGraphAverage(&gpuGraph) * 1000.0f);

	glfwTerminate();
	return 0;
}
