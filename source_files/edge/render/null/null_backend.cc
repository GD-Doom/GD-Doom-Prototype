// clang-format off
#include "../../r_backend.h"
#include <epi.h>

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
      EPI_UNUSED(width);
      EPI_UNUSED(height);
    }

    void Flush(int32_t commands, int32_t vertices)
    {
      EPI_UNUSED(commands);
      EPI_UNUSED(vertices);
    }

    void SwapBuffers()
    {
    }

    void FinishFrame()
    {

    }

    void Resize(int32_t width, int32_t height)
    {
      EPI_UNUSED(width);
      EPI_UNUSED(height);
    }

    void Shutdown()
    {
    }


    void CaptureScreen(int32_t width, int32_t height, int32_t stride, uint8_t *dest)
    {
      EPI_UNUSED(width);
      EPI_UNUSED(height);
      EPI_UNUSED(stride);
      EPI_UNUSED(dest);
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
      EPI_UNUSED(color);
    }

    int32_t GetHUDLayer()
    {
        return kRenderLayerHUD;
    }

    void SetupMatrices(RenderLayer layer, bool context_change = false)
    {
      EPI_UNUSED(layer);
      EPI_UNUSED(context_change);
    }

    void FlushContext()
    {
    }

    virtual void SetRenderLayer(RenderLayer layer, bool clear_depth = false)
    {
      EPI_UNUSED(layer);
      EPI_UNUSED(clear_depth);
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
        EPI_UNUSED(stats);
    }

  private:
};

static NullRenderBackend null_render_backend;
RenderBackend            *render_backend = &null_render_backend;
