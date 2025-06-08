

#include "epi.h"
#include "r_backend.h"
#include "r_state.h"

static constexpr GLuint kGenTextureId       = 0x0000FFFF;
static constexpr GLuint kRenderStateInvalid = 0xFFFFFFFF;

struct MipLevel
{
    GLsizei width_;
    GLsizei height_;
    void   *pixels_;
};

struct TexInfo
{
    GLsizei width_;
    GLsizei height_;
    int64_t update_frame_;
};

constexpr int32_t kMaxClipPlane = 6;

class GodotRenderState : public RenderState
{
  public:
    GodotRenderState()
    {
        Reset();
    }

    void Reset()
    {
        line_width_ = 1.0f;

        for (int32_t i = 0; i < kMaxClipPlane; i++)
        {
            clip_planes_[i].enabled_ = false;
            clip_planes_[i].dirty_   = false;
        }

        scissor_.enabled_ = false;
        scissor_.dirty_   = false;
    }

    void Enable(GLenum cap, bool enabled = true)
    {
        int32_t index;

        switch (cap)
        {
        case GL_TEXTURE_2D:
            break;
        case GL_FOG:
            enable_fog_ = enabled;
            break;
        case GL_ALPHA_TEST:
            enable_alpha_test_ = enabled;
            break;
        case GL_BLEND:
            enable_blend_ = enabled;
            break;
        case GL_CULL_FACE:
            cull_enabled_ = enabled;
            break;
        case GL_SCISSOR_TEST:
            if (enabled != scissor_.enabled_)
            {
                scissor_.enabled_ = enabled;
                scissor_.dirty_   = true;
            }
            break;
        case GL_LIGHTING:
            break;
        case GL_COLOR_MATERIAL:
            break;
        case GL_DEPTH_TEST:
            enable_depth_test_ = enabled;
            break;
        case GL_STENCIL_TEST:
            break;
        case GL_LINE_SMOOTH:
            break;
        case GL_NORMALIZE:
            break;
        case GL_POLYGON_SMOOTH:
            break;

        case GL_CLIP_PLANE0:
        case GL_CLIP_PLANE1:
        case GL_CLIP_PLANE2:
        case GL_CLIP_PLANE3:
        case GL_CLIP_PLANE4:
        case GL_CLIP_PLANE5:
            index = cap - GL_CLIP_PLANE0;
            EPI_ASSERT(index < kMaxClipPlane);
            if (clip_planes_[index].enabled_ != enabled)
            {
                clip_planes_[index].enabled_ = enabled;
                clip_planes_[index].dirty_   = true;
            }
            break;
        default:
            FatalError("Unknown GL State %i", cap);
        }
    }

    void Disable(GLenum cap)
    {
        int32_t index;

        switch (cap)
        {
        case GL_TEXTURE_2D:
            break;
        case GL_FOG:
            enable_fog_ = false;
            break;
        case GL_ALPHA_TEST:
            enable_alpha_test_ = false;
            break;
        case GL_BLEND:
            enable_blend_ = false;
            break;
        case GL_CULL_FACE:
            cull_enabled_ = false;
            break;
        case GL_SCISSOR_TEST:
            if (scissor_.enabled_)
            {
                scissor_.enabled_ = false;
                scissor_.dirty_   = true;
            }
            break;
        case GL_LIGHTING:
            break;
        case GL_COLOR_MATERIAL:
            break;
        case GL_DEPTH_TEST:
            enable_depth_test_ = false;
            break;
        case GL_STENCIL_TEST:
            break;
        case GL_LINE_SMOOTH:
            break;
        case GL_NORMALIZE:
            break;
        case GL_POLYGON_SMOOTH:
            break;

        case GL_CLIP_PLANE0:
        case GL_CLIP_PLANE1:
        case GL_CLIP_PLANE2:
        case GL_CLIP_PLANE3:
        case GL_CLIP_PLANE4:
        case GL_CLIP_PLANE5:
            index = cap - GL_CLIP_PLANE0;
            EPI_ASSERT(index < kMaxClipPlane);
            if (clip_planes_[index].enabled_)
            {
                clip_planes_[index].enabled_ = false;
                clip_planes_[index].dirty_   = true;
            }
            break;
        default:
            FatalError("Unknown GL State %i", cap);
        }
    }

    void DepthMask(bool enable)
    {
        depth_mask_ = enable;
    }

    void DepthFunction(GLenum func)
    {
        depth_function_ = func;
    }

    void CullFace(GLenum mode)
    {
        cull_mode_ = mode;
    }

    void AlphaFunction(GLenum func, GLfloat ref)
    {
        EPI_UNUSED(func);

        alpha_test_ = ref;
    }

    void ActiveTexture(GLenum activeTexture)
    {
        EPI_UNUSED(activeTexture);
    }

    void Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
    {
        scissor_.x_      = x;
        scissor_.y_      = y;
        scissor_.width_  = width;
        scissor_.height_ = height;
    }

    void PolygonOffset(GLfloat factor, GLfloat units)
    {
        EPI_UNUSED(factor);
        EPI_UNUSED(units);
    }

    void Clear(GLbitfield mask)
    {
        if (mask & GL_DEPTH_BUFFER_BIT)
        {
        }
    }

    void ClearColor(RGBAColor color)
    {
        EPI_UNUSED(color);
    }

    void FogMode(GLint fogMode)
    {
        fog_mode_ = fogMode;
    }

    void FogColor(RGBAColor color)
    {
        fog_color_ = color;
    }

    void FogStart(GLfloat start)
    {
        fog_start_ = start;
    }

    void FogEnd(GLfloat end)
    {
        fog_end_ = end;
    }

    void FogDensity(GLfloat density)
    {
        fog_density_ = density;
    }

    void GLColor(RGBAColor color)
    {
        EPI_UNUSED(color);
    }

    void BlendFunction(GLenum sfactor, GLenum dfactor)
    {
        blend_source_factor_      = sfactor;
        blend_destination_factor_ = dfactor;
    }

    void TextureEnvironmentMode(GLint param)
    {
        EPI_UNUSED(param);
    }

    void TextureEnvironmentCombineRGB(GLint param)
    {
        EPI_UNUSED(param);
    }

    void TextureEnvironmentSource0RGB(GLint param)
    {
        EPI_UNUSED(param);
    }

    void MultiTexCoord(GLuint tex, const HMM_Vec2 *coords)
    {
        EPI_UNUSED(tex);
        EPI_UNUSED(coords);
    }

    void Hint(GLenum target, GLenum mode)
    {
        EPI_UNUSED(target);
        EPI_UNUSED(mode);
    }

    void LineWidth(float width)
    {
        line_width_ = width;
    }

    float GetLineWidth()
    {
        return line_width_;
    }

    void DeleteTexture(const GLuint *tex_id)
    {
        EPI_UNUSED(tex_id);
    }

    void FrontFace(GLenum wind)
    {
        EPI_UNUSED(wind);
    }

    void ShadeModel(GLenum model)
    {
        EPI_UNUSED(model);
    }

    void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
    {
        EPI_UNUSED(red);
        EPI_UNUSED(green);
        EPI_UNUSED(blue);
        EPI_UNUSED(alpha);
    }

    void BindTexture(GLuint textureid)
    {
        if (generating_texture_)
        {
            if (textureid != kGenTextureId)
            {
                FatalError("Cannot bind to another texture during texture creation");
            }
        }

        texture_bound_ = textureid ? textureid : kRenderStateInvalid;
    }

    void GenTextures(GLsizei n, GLuint *textures)
    {
        EPI_UNUSED(n);
        generating_texture_ = true;
        texture_level_      = 0;

        texture_wrap_s_ = GL_CLAMP;
        texture_wrap_t_ = GL_CLAMP;

        *textures = kGenTextureId;
    }

    void TextureMinFilter(GLint param)
    {
        texture_min_filter_ = param;
    }

    void TextureMagFilter(GLint param)
    {
        texture_mag_filter_ = param;
    }

    void TextureWrapS(GLint param)
    {
        texture_wrap_s_ = param;
    }

    void TextureWrapT(GLint param)
    {
        texture_wrap_t_ = param;
    }

    void FinishTextures(GLsizei n, GLuint *textures)
    {
        EPI_UNUSED(n);
        EPI_UNUSED(textures);
    }

    void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border,
                    GLenum format, GLenum type, const void *pixels, RenderUsage usage = kRenderUsageImmutable)
    {
        EPI_UNUSED(target);
        EPI_UNUSED(level);
        EPI_UNUSED(internalformat);
        EPI_UNUSED(width);
        EPI_UNUSED(height);
        EPI_UNUSED(border);
        EPI_UNUSED(format);
        EPI_UNUSED(type);
        EPI_UNUSED(pixels);
        EPI_UNUSED(usage);
    }

    void PixelStorei(GLenum pname, GLint param)
    {
        EPI_UNUSED(pname);
        EPI_UNUSED(param);
    }

    void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
    {
        EPI_UNUSED(x);
        EPI_UNUSED(y);
        EPI_UNUSED(width);
        EPI_UNUSED(height);
        EPI_UNUSED(format);
        EPI_UNUSED(type);
        EPI_UNUSED(pixels);
    }

    void PixelZoom(GLfloat xfactor, GLfloat yfactor)
    {
        EPI_UNUSED(xfactor);
        EPI_UNUSED(yfactor);
    }

    void Flush()
    {
    }

    void OnContextSwitch()
    {
        for (int32_t i = 0; i < kMaxClipPlane; i++)
        {
            if (clip_planes_[i].enabled_)
            {
                clip_planes_[i].dirty_ = true;
            }
        }

        if (scissor_.enabled_)
        {
            scissor_.dirty_ = true;
        }
    }

    void ClipPlane(GLenum plane, GLdouble *equation)
    {
        int32_t index = plane - GL_CLIP_PLANE0;
        EPI_ASSERT(index < kMaxClipPlane);

        EClipPlane *clip = &clip_planes_[index];

        clip->equation_[0] = equation[0];
        clip->equation_[1] = equation[1];
        clip->equation_[2] = equation[2];
        clip->equation_[3] = equation[3];
    }

    void SetPipeline(uint32_t flags)
    {

        EPI_UNUSED(flags);
    }

    // state
    bool   enable_depth_test_;
    GLenum depth_function_;
    bool   depth_mask_;

    bool      enable_fog_;
    GLint     fog_mode_;
    GLfloat   fog_start_;
    GLfloat   fog_end_;
    GLfloat   fog_density_;
    RGBAColor fog_color_;

    bool   cull_enabled_ = false;
    GLenum cull_mode_    = GL_BACK;

    bool   enable_blend_;
    GLenum blend_source_factor_;
    GLenum blend_destination_factor_;

    bool    enable_alpha_test_ = false;
    GLfloat alpha_test_;

    // texture creation
    bool                                    generating_texture_ = false;
    GLint                                   texture_level_;
    std::vector<MipLevel>                   mip_levels_;
    std::unordered_map<uint32_t, TexInfo *> tex_infos_;

    GLuint texture_bound_ = kRenderStateInvalid;

    GLint texture_min_filter_;
    GLint texture_mag_filter_;
    GLint texture_wrap_s_;
    GLint texture_wrap_t_;

    float line_width_;

    struct EClipPlane
    {
        bool  enabled_;
        bool  dirty_;
        float equation_[4];
    };

    EClipPlane clip_planes_[kMaxClipPlane];

    struct EScissor
    {
        bool    enabled_;
        bool    dirty_;
        int32_t x_;
        int32_t y_;
        int32_t width_;
        int32_t height_;
    };

    EScissor scissor_;
};

static GodotRenderState godot_render_state;
RenderState            *render_state = &godot_render_state;

void SetupSkyMatrices(void)
{
}

void RendererRevertSkyMatrices(void)
{
    /*
    sgl_matrix_mode_projection();
    sgl_pop_matrix();

    sgl_matrix_mode_modelview();
    sgl_pop_matrix();
    */
}