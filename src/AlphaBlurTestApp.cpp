#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class AlphaBlurTestApp : public App {
    
    ci::gl::FboRef      mFboFirstPass;
    ci::gl::FboRef      mFboSecondPass;
    ci::gl::GlslProgRef mGlsl;
    
    ci::gl::TextureRef  mTexture;


  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void draw() override;
    void renderBlur(ci::ColorA backgroundColor);
};

void AlphaBlurTestApp::setup()
{
    ci::gl::Fbo::Format format;
    format.setSamples(0);
    format.setColorTextureFormat(ci::gl::Texture2d::Format().internalFormat(GL_RGBA32F));
    format.enableDepthBuffer(false);
    
    mFboFirstPass = gl::Fbo::create(200,200, format);
    mFboSecondPass = gl::Fbo::create(200,200, format);
    
    try{
        mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("shaders/blur.vert"))
                                     .fragment(loadAsset("shaders/blur.frag")));
    }
    catch (gl::GlslProgCompileExc ex) {
        std::cout << ex.what() << std::endl;
    }

    
    mTexture = gl::Texture::create(loadImage(loadAsset("UI/backButtonBlue.png")));
}

void AlphaBlurTestApp::mouseDown( MouseEvent event )
{
}

void AlphaBlurTestApp::renderBlur(ci::ColorA backgroundColor)
{
    float blurAmount = getMousePos().x / 200000.0f;

    gl::ScopedGlslProg scpProg(mGlsl);
    gl::pushMatrices();
    gl::setMatricesWindow(mFboFirstPass->getSize());
    gl::ScopedViewport vp( ci::ivec2( 0 ), mFboFirstPass->getSize() );

    // first pass
    {
        gl::ScopedFramebuffer scpFrame(mFboFirstPass);
        gl::clear(backgroundColor);

        mGlsl->uniform("sampleOffset", blurAmount * vec2(1,0));
        
        gl::ScopedTextureBind tex0(mTexture, 0);
        gl::drawSolidRect(Rectf(20,20,20 + mTexture->getWidth(),20 + mTexture->getHeight()));
    }
    
    // second pass
    {
        gl::ScopedFramebuffer scpFrame(mFboSecondPass);
        gl::clear(backgroundColor);
        
        mGlsl->uniform("sampleOffset", blurAmount * vec2(0,1));
        
        gl::ScopedTextureBind tex0(mFboFirstPass->getColorTexture(), 0);
        gl::drawSolidRect(Rectf(0,0,mTexture->getWidth(),mTexture->getHeight()));
    }
    
    gl::popMatrices();

    
    
}

void AlphaBlurTestApp::draw()
{
    gl::clear( Color( 0.83, 0.83, 0.85 ) );
    
    renderBlur(ColorA( 1, 1, 1,1 ));
    ci::gl::draw(mFboSecondPass->getColorTexture(),vec2(0,20));
    renderBlur(ColorA( 1, 1, 1, 0 ));
    ci::gl::draw(mFboSecondPass->getColorTexture(),vec2(0,220));

    
    ci::gl::draw(mTexture,vec2(300,20));
}

CINDER_APP( AlphaBlurTestApp, RendererGl )
