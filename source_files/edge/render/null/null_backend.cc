// clang-format off
#include "../../r_backend.h"

class NullRenderBackend : public RenderBackend
{
  public:
    void SetupMatrices2D()
    {
    }

    void SetupWorldMatrices2D()
    {
    }

    void SetupMatrices3D()
    {
    }

    void StartFrame(int32_t width, int32_t height)
    {
    }

    void Flush(int32_t commands, int32_t vertices)
    {
    }

    void SwapBuffers()
    {
    }

    void FinishFrame()
    {

    }

    void Resize(int32_t width, int32_t height)
    {
    }

    void Shutdown()
    {
    }


    void CaptureScreen(int32_t width, int32_t height, int32_t stride, uint8_t *dest)
    {

    }

    void Init()
    {
        max_texture_size_ = 4096;
        RenderBackend::Init();
    }

    // FIXME: go away!
    void GetPassInfo(PassInfo &info)
    {
        info.width_  = 1280;
        info.height_ = 720;
    }

    void SetClearColor(RGBAColor color)
    {
    }

    int32_t GetHUDLayer()
    {
        return kRenderLayerHUD;
    }

    void SetupMatrices(RenderLayer layer, bool context_change = false)
    {
    }

    void FlushContext()
    {
    }

    virtual void SetRenderLayer(RenderLayer layer, bool clear_depth = false)
    {
    }

    RenderLayer GetRenderLayer()
    {
        return kRenderLayerHUD;
    }

    void BeginWorldRender()
    {
    }

    void FinishWorldRender()
    {
    }

    void GetFrameStats(FrameStats &stats)
    {
    }

  private:
};

static NullRenderBackend null_render_backend;
RenderBackend            *render_backend = &null_render_backend;
