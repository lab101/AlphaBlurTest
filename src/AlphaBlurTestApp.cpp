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
    ci::gl::GlslProgRef mGlsl2;

    ci::gl::TextureRef  mTexture;


  public:
	void setup() override;
	void draw() override;
    void renderBlur(ci::ColorA backgroundColor);
};

void AlphaBlurTestApp::setup()
{

    mFboFirstPass = gl::Fbo::create(150,150);
    mFboSecondPass = gl::Fbo::create(150,150);
    
    try{
        mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("shaders/blur.vert"))
                                     .fragment(loadAsset("shaders/blur.frag")));
        mGlsl2 = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("shaders/blur.vert"))
                                      .fragment(loadAsset("shaders/blur2.frag")));
    }
    catch (gl::GlslProgCompileExc ex) {
        std::cout << ex.what() << std::endl;
    }
    

    
    mTexture = gl::Texture::create(loadImage(loadAsset("UI/backButtonBlue.png")));
}



void AlphaBlurTestApp::renderBlur(ci::ColorA backgroundColor)
{
    float blurAmount = getMousePos().x / 200000.0f;

    gl::pushMatrices();
    gl::setMatricesWindow(mFboFirstPass->getSize());
    gl::ScopedViewport vp( ci::ivec2( 0 ), mFboFirstPass->getSize() );

    // first pass
    {
        gl::ScopedGlslProg scpProg(mGlsl);

        gl::ScopedFramebuffer scpFrame(mFboFirstPass);
        gl::clear(backgroundColor,true);

        mGlsl->uniform("sampleOffset", blurAmount * vec2(1,0));
        gl::enableBlending();
        gl::context()->pushBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        
        gl::ScopedTextureBind tex0(mTexture, (uint8_t) 0);
        gl::drawSolidRect(mTexture->getBounds());
        
        gl::context()->popBlendFuncSeparate();
    }
    
    // second pass
    {
        gl::ScopedGlslProg scpProg(mGlsl2);

        gl::ScopedFramebuffer scpFrame(mFboSecondPass);
        gl::clear(backgroundColor);

        mGlsl2->uniform("sampleOffset", blurAmount * vec2(0,1));

        gl::ScopedTextureBind tex0(mFboFirstPass->getColorTexture(), 0);
        gl::drawSolidRect(Rectf(0,0,mFboFirstPass->getWidth(),mFboFirstPass->getHeight()));
    }
    
    gl::popMatrices();

    
    
}

void AlphaBlurTestApp::draw()
{
    gl::clear( Color( 0.85, 0.85, 0.85),true );
  
    gl::enableAlphaBlending();
    ci::gl::draw(mTexture,vec2(300,20));

    {
        // render with transparent background
        renderBlur(ColorA( 0, 0, 0, 0.0000 ));
        gl::ScopedBlendPremult scpBlend2;
        ci::gl::draw(mFboSecondPass->getColorTexture(),vec2(0,220));
    }
    
}

CINDER_APP( AlphaBlurTestApp, RendererGl )
