#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <boost/thread.hpp>
class RenderThreadImpl;
class GameGUI;
namespace GAGCore
{
	class GraphicContext;
}
namespace GAGGUI
{
	class Screen;
}
class RenderThread
{
public:
	RenderThread();
	~RenderThread();
	GAGCore::GraphicContext* getGfx();
	void setScreen(GAGGUI::Screen* s);
	void setGui(GameGUI* g);
	void removeScreen(GAGGUI::Screen* s);
	void removeGui(GameGUI* g);
	//void setMapEdit();
	void pushOrder(std::function<void()> f);
	boost::thread::id getId();
private:
	RenderThreadImpl* impl;
};
#endif
