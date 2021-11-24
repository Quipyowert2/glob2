#include "RenderThread.h"
#include "SDLGraphicContext.h"
#include "Toolkit.h"
#include "GlobalContainer.h"
#include <iostream>
#include <boost/thread.hpp>
using namespace GAGCore;
class RenderThreadImpl
{
public:
RenderThreadImpl() :gfx(nullptr) {}
int operator()()
{
	std::cout << "RenderThreadImpl::operator() " << std::endl;
	{
		boost::lock_guard<boost::mutex> lock(gfxMutex);
		// create graphic context
		gfx = Toolkit::initGraphic(globalContainer->settings.screenWidth, globalContainer->settings.screenHeight, globalContainer->settings.screenFlags, "Globulation 2", "glob 2");
		gfx->setMinRes(640, 480);
		//gfx->setQuality((settings.optionFlags & OPTION_LOW_SPEED_GFX) != 0 ? GraphicContext::LOW_QUALITY : GraphicContext::HIGH_QUALITY);
	}
	gfxInitialized.notify_all();
}
GAGCore::GraphicContext* getGfx()
{
	boost::unique_lock<boost::mutex> lock(gfxMutex);
	while (gfx == nullptr) {
		gfxInitialized.wait(lock);
	}
	return gfx;
}
private:
	GAGCore::GraphicContext* gfx;
	boost::mutex gfxMutex;
	boost::condition_variable gfxInitialized;
};
RenderThread::RenderThread()
	:impl(new RenderThreadImpl)
{
	std::cout << "RenderThread constructor" << std::endl;
	boost::thread thread(boost::ref(*impl));
}
GAGCore::GraphicContext* RenderThread::getGfx()
{
	return impl->getGfx();
}
RenderThread::~RenderThread()
{
	delete impl;
}
