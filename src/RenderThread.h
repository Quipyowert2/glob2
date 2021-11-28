#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
#include <boost/thread.hpp>
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
	void pushOrder(std::function<void()> f);
	boost::thread::id getId();
private:
	RenderThreadImpl* impl;
};
#endif
