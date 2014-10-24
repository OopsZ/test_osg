#include "stdafx.h"
#include "creators.h"
#include "shaders.h"

//#include "CommonFunctions"
#include "find_node_visitor.h"  
#include "find_tex_visitor.h"
#include "animation_handler.h"

static const std::string animationName("Default");

class ComputeTangentVisitor : public osg::NodeVisitor
{
public:
    void apply( osg::Node& node ) { traverse(node); }
    
    void apply( osg::Geode& node )
    {
        for ( unsigned int i=0; i<node.getNumDrawables(); ++i )
        {
            osg::Geometry* geom = dynamic_cast<osg::Geometry*>( node.getDrawable(i) );
            if ( geom ) generateTangentArray( geom );
        }
        traverse( node );
    }
    
    void generateTangentArray( osg::Geometry* geom )
    {
        osg::ref_ptr<osgUtil::TangentSpaceGenerator> tsg = new osgUtil::TangentSpaceGenerator;
        tsg->generate( geom, 1 );
        geom->setVertexAttribArray( 6, static_cast<osg::Array*>(tsg->getTangentArray()) );
        geom->setVertexAttribBinding( 6, osg::Geometry::BIND_PER_VERTEX );
        geom->setVertexAttribArray( 7, static_cast<osg::Array*>(tsg->getBinormalArray()) );
        geom->setVertexAttribBinding( 7, osg::Geometry::BIND_PER_VERTEX );
    }
};

osg::Geometry* createOctahedron( const osg::Vec3& center )
{
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(6);
    (*vertices)[0].set( 0.0f, 0.0f, 1.0f); (*vertices)[0] += center;
    (*vertices)[1].set(-0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[2].set( 0.5f,-0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[3].set( 0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[4].set(-0.5f, 0.5f, 0.0f); (*vertices)[0] += center;
    (*vertices)[5].set( 0.0f, 0.0f,-1.0f); (*vertices)[0] += center;

    osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(GL_TRIANGLES, 24);
    (*indices)[0] = 0; (*indices)[1] = 1; (*indices)[2] = 2;
    (*indices)[3] = 0; (*indices)[4] = 2; (*indices)[5] = 3;
    (*indices)[6] = 0; (*indices)[7] = 3; (*indices)[8] = 4;
    (*indices)[9] = 0; (*indices)[10]= 4; (*indices)[11]= 1;
    (*indices)[12]= 5; (*indices)[13]= 2; (*indices)[14]= 1;
    (*indices)[15]= 5; (*indices)[16]= 3; (*indices)[17]= 2;
    (*indices)[18]= 5; (*indices)[19]= 4; (*indices)[20]= 3;
    (*indices)[21]= 5; (*indices)[22]= 1; (*indices)[23]= 4;
    
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    texcoords->push_back( osg::Vec2(0.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.0f) );
    texcoords->push_back( osg::Vec2(0.5f, 1.0f) );
    texcoords->push_back( osg::Vec2(1.0f, 0.5f) );

    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;
    geom->setVertexArray( vertices.get() );
    geom->addPrimitiveSet( indices.get() );
    geom->setTexCoordArray( 0, texcoords.get() );

    // osgUtil::SmoothingVisitor::smooth( *geom );
    return geom.release();
}

osg::Node* CreateEarth()
{
    const double r_earth = 63.137;

    osg::ShapeDrawable *earth_sd = new osg::ShapeDrawable;
    osg::Sphere* earth_sphere = new osg::Sphere;
    earth_sphere->setName("EarthSphere");
    earth_sphere->setRadius(r_earth);
    earth_sd->setShape(earth_sphere);
    earth_sd->setColor(osg::Vec4(0, 0, 1.0, 1.0));

    osg::Geode* earth_geode = new osg::Geode;
    earth_geode->setName("EarthGeode");
    earth_geode->addDrawable(earth_sd);
     
    osg::ref_ptr<osg::Geode> oct = new osg::Geode;
    oct->addDrawable( createOctahedron(osg::Vec3()) );

    osg::Group* root = new osg::Group;
    //root->addChild(earth_geode);
    //root->addChild(osgDB::readNodeFile("cessna2.osgt")); 
    root->addChild(oct);
    return root ;
}

class LightChangeHandler : public osgGA::GUIEventHandler
{
public:
    LightChangeHandler(osg::LightSource *ls) : ls_(ls) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        if (!ea.getHandled() && ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Add)
            {
                auto a_ = ls_->getLight()->getAmbient();
                auto d_ = ls_->getLight()->getDiffuse();
                auto s_ = ls_->getLight()->getSpecular();

                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                {}
                else                                                                    // Decrement by one minute
                {
                    a_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f);  d_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f); s_ += osg::Vec4f(0.1f,0.1f,0.1f,0.1f); 
                }
                ls_->getLight()->setAmbient(a_);
                ls_->getLight()->setDiffuse(d_);
                ls_->getLight()->setSpecular(s_);

                return true;
            }

            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_KP_Subtract)
            {
                const osg::Vec4f zero(0.0f,0.0f,0.0f,0.0f);
                auto a_ = ls_->getLight()->getAmbient();
                auto d_ = ls_->getLight()->getDiffuse();
                auto s_ = ls_->getLight()->getSpecular();

                if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_SHIFT)          // Decrement by one hour
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_ALT)       // Decrement by one day
                {}
                else if (ea.getModKeyMask() & osgGA::GUIEventAdapter::MODKEY_CTRL)      // Decrement by one month
                {}
                else                                                                    // Decrement by one minute
                {
                  a_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f);  d_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f); s_ -= osg::Vec4f(0.1f,0.1f,0.1f,0.1f);
                  if(a_ < zero) a_ = zero; if(d_ < zero) d_ = zero; if(s_ < zero) s_ = zero;
                }
                ls_->getLight()->setAmbient(a_);
                ls_->getLight()->setDiffuse(d_);
                ls_->getLight()->setSpecular(s_);

                return true;
            }

        }

        return false;
    }

    virtual void getUsage(osg::ApplicationUsage& usage) const
    {
        usage.addKeyboardMouseBinding("Keypad +",       "Increment ambient diffuse specular by 0.1");
        usage.addKeyboardMouseBinding("Keypad -",       "Decrement ambient diffuse specular by 0.1");
    }

    osg::ref_ptr<osg::LightSource> ls_;
};

int main_bump_map( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    osg::DisplaySettings::instance()->setNumMultiSamples( 8 );
    
    osg::ref_ptr<osg::Group> root (new osg::Group);

    osg::ref_ptr<osg::StateSet> lightSS (root->getOrCreateStateSet());
    //osg::ref_ptr<osg::LightSource> lightSource1 = new osg::LightSource;
    osg::ref_ptr<osg::LightSource> lightSource2 = new osg::LightSource;
    // create a local light.
    osg::Vec4f lightPosition2 (osg::Vec4f(-200.0,-100.0,-300.0,0.0f));
    osg::ref_ptr<osg::Light> myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(lightPosition2);

#if 1
    myLight2->setAmbient(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    myLight2->setSpecular(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
#else
    myLight2->setAmbient(osg::Vec4(0.5f,0.5f,0.5f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.9f,0.9f,0.85f,1.0f));
    myLight2->setSpecular(osg::Vec4(0.2f,0.2f,0.2f,1.0f));
    myLight2->setConstantAttenuation(1.0f);
#endif

    lightSource2->setLight(myLight2.get());
    lightSource2->setLocalStateSetModes(osg::StateAttribute::ON); 
    lightSource2->setStateSetModes(*lightSS,osg::StateAttribute::ON);


/////////////////////////////////////////////////////////////////////////    
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles( arguments );
    if ( !model ) 
        model = creators::loadAirplane();// CreateEarth(); //osgDB::readNodeFile("skydome.osgt");//  //osgDB::readNodeFile("spaceship.osgt"); // 
   
    ComputeTangentVisitor ctv;
    ctv.setTraversalMode( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
    model->accept( ctv );
    
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::VERTEX,   shaders::plane_mat::get_shader(shaders::VS)) );
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, shaders::plane_mat::get_shader(shaders::FS)) );
    program->addBindAttribLocation( "tangent", 6 );
    program->addBindAttribLocation( "binormal", 7 );

#if 0
    osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    colorTex->setImage( osgDB::readImageFile("Images/whitemetal_diffuse.jpg") );
    
    osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    normalTex->setImage( osgDB::readImageFile("Images/whitemetal_normal.jpg") );
#else
    
    //osg::ref_ptr<osg::Texture2D> colorTex = new osg::Texture2D;
    //colorTex->setImage( osgDB::readImageFile("a_319_airfrance.dds"/*,new osgDB::Options("dds_flip")*/) );//

    FindTextureVisitor ft("a_319");
    model->accept( ft );
    osg::ref_ptr<osg::Texture2D> colorTex =  new osg::Texture2D(ft.getTexture()->getImage(0));

    osg::ref_ptr<osg::Texture2D> normalTex = new osg::Texture2D;
    normalTex->setImage( osgDB::readImageFile("a_319_n.dds") );  
#endif

    osg::StateSet* stateset = model->getOrCreateStateSet();
    stateset->addUniform( new osg::Uniform("colorTex", 0) );
    stateset->addUniform( new osg::Uniform("normalTex", 1) );
    stateset->setAttributeAndModes( program.get() );

    osg::StateAttribute::GLModeValue value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
    stateset->setTextureAttributeAndModes( 0, colorTex.get(), value );
    stateset->setTextureAttributeAndModes( 1, normalTex.get(), value );



#if 0  // ����� ��������� � ���������, �� �������� ������ � ���������� �������
    stateset->setMode( GL_BLEND,osg::StateAttribute::ON );

    //stateset->setAttributeAndModes( new osg::CullFace() );
    //stateset->setAttributeAndModes( new osg::Depth( osg::Depth::LESS, 0.f, 1.f, true ) );

    stateset->setAttributeAndModes( new osg::AlphaFunc( osg::AlphaFunc::GREATER, 0.0f ) );
#endif

    osgDB::writeNodeFile(*model,"bump_mapping_test.osgt");
    
    root->addChild(model.get());
	root->addChild(lightSource2.get());

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( root.get() );

    // Use a default camera manipulator
    osgGA::TrackballManipulator* manip = new osgGA::TrackballManipulator;
    viewer.setCameraManipulator(manip);
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::HelpHandler);
    viewer.addEventHandler(new LightChangeHandler(lightSource2.get()));

    viewer.addEventHandler(new AnimationHandler(model,animationName
            ,nullptr
            ,nullptr
            ,nullptr
    ));


    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );


    return viewer.run();
}
