#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <boost/thread.hpp>
#include "GameGUI.h"
class RenderThreadImpl;
namespace GAGCore
{
	class GraphicContext;
}
class RenderThread
{
public:
	RenderThread();
	~RenderThread();
	GAGCore::GraphicContext* getGfx();
	void setScreen(Screen* s);
	void setGui(GameGUI* g);
	void removeScreen(Screen* s);
	void removeGui(GameGUI* g);
	//void setMapEdit();
	void pushOrder(std::function<void()> f);
	boost::thread::id getId();
private:
	RenderThreadImpl* impl;
};
#endif
