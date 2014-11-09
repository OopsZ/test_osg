#include "stdafx.h"
#include "find_node_visitor.h" 
#include "info_visitor.h"
#include "find_tex_visitor.h"
#include "creators.h"
#include "sv/FogLayer.h"

osg::Image* createSpotLight( const osg::Vec4& centerColor, const osg::Vec4& bgColor, unsigned int size, float power )
{
    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage( size, size, 1, GL_RGBA, GL_UNSIGNED_BYTE );

    float mid = (float(size)-1) * 0.5f;
    float div = 2.0f / float(size);
    for( unsigned int r=0; r<size; ++r )
    {
        unsigned char* ptr = image->data(0, r, 0);
        for( unsigned int c=0; c<size; ++c )
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float r = powf(1.0f - sqrtf(dx*dx+dy*dy), power);
            if ( r<0.0f ) r = 0.0f;

            osg::Vec4 color = centerColor*r + bgColor*(1.0f - r);
            *ptr++ = (unsigned char)((color[0]) * 255.0f);
            *ptr++ = (unsigned char)((color[1]) * 255.0f);
            *ptr++ = (unsigned char)((color[2]) * 255.0f);
            *ptr++ = (unsigned char)((color[3]) * 255.0f);
        }
    }
    return image.release();
}


class TexChangeHandler : public osgGA::GUIEventHandler
{
public:
    TexChangeHandler(osg::Node * root,osg::Texture *texture) 
      :_texture    (texture)
      ,_root       (root)
      {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Add || ea.getKey()== osgGA::GUIEventAdapter::KEY_KP_Subtract)
            { 
                if ( _root )
                { 
                    ReplaceTextureVisitor ftv( _texture.get() );
                    _root->accept( ftv );
                    _texture =  ftv.getOldTexture();
                }
                return true;
            }
        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Change texture");
        usage.addKeyboardMouseBinding("Keypad -",       "Change texture");

    }

    osg::ref_ptr<osg::Texture> _texture;
    osg::ref_ptr<osg::Node> _root;
};



class FogHandler : public osgGA::GUIEventHandler
{
public:
	typedef std::function<void(osg::Vec4f)> on_fog_change_f;
public:
    FogHandler(const on_fog_change_f& f_fog_changer) 
      :_f_fog_changer    (f_fog_changer)
	   , _intensivity(0.1)
      {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {            
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_F3 )
            { 
				_intensivity += 0.1;
                if ( _f_fog_changer )
                { 
					_f_fog_changer(osg::Vec4f(1.0,1.0,1.0,_intensivity));
                }
                return true;
            } else
            if (ea.getKey()== osgGA::GUIEventAdapter::KEY_F4)
            { 
				_intensivity -= 0.1;
                if ( _f_fog_changer )
                { 
					_f_fog_changer(osg::Vec4f(1.0,1.0,1.0,_intensivity));
                }
                return true;
            }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("F3",       "+ fog");
        usage.addKeyboardMouseBinding("F4",       "- fog");

    }

    on_fog_change_f _f_fog_changer;
	float           _intensivity; 
    
};

int main_tex_test( int argc, char** argv )
{

    osgViewer::Viewer viewer;

    osg::ref_ptr<osg::MatrixTransform> root = new osg::MatrixTransform;
    //root->setMatrix(osg::Matrix::rotate(osg::inDegrees(-90.0f),1.0f,0.0f,0.0f));
#if 0
    //int tex_width = 2048, tex_height = 2048;
    //osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    //texture->setTextureSize( tex_width, tex_height );
    //texture->setInternalFormat( GL_RGBA );
    //texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    //texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
#endif
#if 0
    osg::Vec4 centerColor( 1.0f, 1.0f, 0.0f, 1.0f );
    osg::Vec4 bgColor( 0.0f, 0.0f, 0.0f, 1.0f );

    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.0f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.5f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 4.0f) );
    imageSequence->addImage( createSpotLight(centerColor, bgColor, 2048, 3.5f) );

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
    texture->setImage( imageSequence.get() );
#endif

    osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D(osgDB::readImageFile("a_319_aeroflot.png"/*"Fieldstone.jpg"*/));

    creators::nodes_array_t plane = creators::loadAirplaneParts();
    auto airplane = plane[1];

    // osg::Node*  airplane = // creators::loadAirplane();
    
    InfoVisitor infoVisitor;
    airplane->accept( infoVisitor );

    findNodeVisitor findNode("Body_",findNodeVisitor::not_exact); 
    airplane->accept(findNode);
    auto a_node =  findNode.getFirst();
    
    findNodeByType<osg::Geode> findGeode; 
    a_node->accept(findGeode);
    auto g_node =  findGeode.getFirst();
    
    //auto dr = g_node->asGeode()->getDrawable(0);
    //
    //auto dr_num =  g_node->asGeode()->getNumDrawables();

    // g_node->setNodeMask(0);

    //osg::StateSet* stateset = g_node->asGeode()->getDrawable(0)->getOrCreateStateSet();
    // � ��� ��� � ��� ���������� �� ����� ����
    // osg::Array* a = g_node->asGeode()->getDrawable(0)->asGeometry()->getTexCoordArray(1);

    //stateset->setTextureAttributeAndModes( 0, /*texture.get()*/new osg::Texture2D());
    //g_node->setStateSet(stateset);
    
    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum( 0 );
    light->setAmbient(osg::Vec4(0.2, 0.2, 0.2, 1));
    light->setDiffuse(osg::Vec4(0.8, 0.8, 0.8, 1));
    light->setPosition( osg::Vec4(100.0f, 10.0f, 10.0f, 1.0f) );

    osg::ref_ptr<osg::LightSource> source = new osg::LightSource;
    source->setLight( light ); 
    
    osg::ref_ptr<osgFX::BumpMapping> effet = new osgFX::BumpMapping();
    effet->setLightNumber(0);
    effet->setOverrideDiffuseTexture(texture.get());
    effet->setOverrideNormalMapTexture(new osg::Texture2D(osgDB::readImageFile("a_319_n.png")));
    effet->addChild(airplane);
    // effet->prepareChildren();
    effet->setEnabled(false);

         osg::ref_ptr<FogLayer> skyFogLayer = new FogLayer(root->asGroup());
         root->addChild(skyFogLayer.get());
         skyFogLayer->setFogParams(osg::Vec3f(1.5,1.5,1.5),0.1);    // (����� �������� � 0.1 �� �������� 1.0)
         float coeff = skyFogLayer->getFogExp2Coef();
    
    osg::StateSet * pCommonStateSet = airplane->getOrCreateStateSet();
    pCommonStateSet->setNestRenderBins(false);
    pCommonStateSet->setRenderBinDetails(/*RENDER_BIN_SOLID_MODELS*/0, "RenderBin");

    // effet->setUpDemo();
    // effet->setEnabled(false);
    root->addChild(source);
    root->addChild(effet);

    //FindTextureVisitor ftv( texture.get() );
    //if ( root ) root->accept( ftv );
    
    // osgDB::writeNodeFile(*root,"tex_test_blank.osgt");
	
	// Set the clear color to black
    viewer.getCamera()->setClearColor(osg::Vec4(1.0,0,0,1));

    viewer.addEventHandler( new TexChangeHandler( root.get(), texture.get() ) );
	viewer.addEventHandler( new FogHandler([&](osg::Vec4f v){skyFogLayer->setFogParams(osg::Vec3f(1.5,1.5,1.5),v.w());} ));
    // Add some useful handlers to see stats, wireframe and onscreen help
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgGA::StateSetManipulator(root->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.setSceneData( root.get() );
    return viewer.run();
}