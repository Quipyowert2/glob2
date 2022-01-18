#include "RenderThread.h"
#include "SDLGraphicContext.h"
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
RenderThreadImpl() :gfx(nullptr), keepRunning(true), states{TITLESCREEN}
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

	//main loop
	while (keepRunning)
	{
		while (queue.empty())
		{
			RenderState state;
			{
				boost::lock_guard<boost::mutex> lock(stateMutex);
				state = states.back();
			}
			switch (state)
			{
			case TITLESCREEN:
			case IN_MENU:
				if (screens.empty())
					break;
				{
				Screen* s = screens.back();
				std::cout << "RenderThread calling Screen::dispatchPaint(): Screen is " << s << std::endl;
				s->dispatchPaint();
				}
				break;
			case IN_GAME:
				if (guis.empty())
					break;
				{
				GameGUI* g = guis.back();
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
		boost::unique_lock<boost::mutex> lock(queueMutex);
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

	boost::mutex stateMutex;
	std::vector<RenderState> states;
	std::vector<GameGUI*> guis;
	std::vector<Screen*> screens;

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
	boost::lock_guard<boost::mutex> lock(impl->stateMutex);
	impl->states.push_back(IN_MENU);
	impl->screens.push_back(s);
}
void RenderThread::setGui(GameGUI* g)
{
	boost::lock_guard<boost::mutex> lock(impl->stateMutex);
	impl->states.push_back(IN_GAME);
	impl->guis.push_back(g);
}
void RenderThread::removeScreen(Screen* s)
{
	boost::lock_guard<boost::mutex> lock(impl->stateMutex);
	assert(s == impl->screens[impl->screens.size()-1]);
	impl->screens.pop_back();
	impl->states.pop_back();
}
void RenderThread::removeGui(GameGUI* g)
{
	boost::lock_guard<boost::mutex> lock(impl->stateMutex);
	assert(g == impl->guis[impl->guis.size()-1]);
	impl->guis.pop_back();
	impl->states.pop_back();
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
