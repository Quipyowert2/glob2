#ifndef RENDERTHREAD_H
#define RENDERTHREAD_H
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
private:
	RenderThreadImpl* impl;
};
#endif
