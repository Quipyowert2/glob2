#include "RenderThread.h"
#include "SDLGraphicContext.h"
#include "Toolkit.h"
#include "GlobalContainer.h"
#include <iostream>
#include <boost/thread.hpp>
using namespace GAGCore;
class RenderThreadImpl
{
	friend class RenderThread;
public:
RenderThreadImpl() :gfx(nullptr), keepRunning(true)
{
	thread = boost::thread(boost::ref(*this));
	id = thread.get_id();
}
~RenderThreadImpl()
{
	keepRunning = false;
	thread.join();
}
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

	//main loop
	while (keepRunning)
	{
		boost::unique_lock<boost::mutex> lock(queueMutex);
		while (queue.empty())
		{
			queueCond.wait(lock);
		}
		queue.front()();
		queue.pop_front();
	}
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

	std::deque<std::function<void()> > queue;
	boost::mutex queueMutex;
	boost::condition_variable queueCond;

	boost::thread::id id;
	boost::thread thread;
	std::atomic<bool> keepRunning;
};
RenderThread::RenderThread()
	:impl(new RenderThreadImpl)
{
	std::cout << "RenderThread constructor" << std::endl;
}
GAGCore::GraphicContext* RenderThread::getGfx()
{
	return impl->getGfx();
}
// Pass a parameter-less lambda
void RenderThread::pushOrder(std::function<void()> f)
{
	{
		boost::lock_guard<boost::mutex> lock(impl->queueMutex);
		impl->queue.push_back(f);
	}
	impl->queueCond.notify_all();
}
boost::thread::id RenderThread::getId()
{
	return impl->id;
}
RenderThread::~RenderThread()
{
	delete impl;
}
