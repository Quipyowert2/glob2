#include "RenderThread.h"
#include "SDLGraphicContext.h"
#include "Toolkit.h"
#include "GlobalContainer.h"
#include <iostream>
#include <boost/thread.hpp>
using namespace GAGCore;
enum RenderState {
	TITLESCREEN,
	IN_MENU,
	IN_GAME,
	MAPEDIT
};
class RenderThreadImpl
{
	friend class RenderThread;
public:
RenderThreadImpl() :gfx(nullptr), keepRunning(true), state(TITLESCREEN), gui(nullptr), screen(nullptr)
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
			switch (state.load())
			{
			case TITLESCREEN:
			case IN_MENU:
				if (!screen)
					break;
				screen.load()->dispatchPaint();
				break;
			case IN_GAME:
				if (!gui)
					break;
				{
				GameGUI* g = gui.load();
				// Engine.cpp:511
				g->drawAll(g->localTeamNo);
				}
				break;
			case MAPEDIT:
				assert(!"Unimplemented");
				break;
			default:
				assert(!"Unknown render state");
				break;
			}
			//queueCond.wait(lock);
		}
		queue.front()();
		queue.pop_front();
	}
	return true;
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

	std::atomic<RenderState> state;
	std::atomic<GameGUI*> gui;
	std::atomic<Screen*> screen;

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
void RenderThread::setScreen(Screen* s)
{
	impl->state = IN_MENU;
	impl->screen = s;
}
void RenderThread::setGui(GameGUI* g)
{
	impl->state = IN_GAME;
	impl->gui = g;
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
