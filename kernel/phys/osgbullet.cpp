#include "stdafx.h"

#include "../high_res_timer.h"
#include "BulletInterface.h"

#include "../creators.h"


class SampleRigidUpdater : public osgGA::GUIEventHandler
{
public:
    SampleRigidUpdater( osg::Group* root ) : _root(root) {}
    
    void addGround( const osg::Vec3& gravity )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(
            new osg::Box(osg::Vec3(0.0f, 0.0f,-0.5f), 100.0f, 100.0f, 1.0f)) );
        
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( geode.get() );
        _root->addChild( mt.get() );
        
        phys::sys()->createWorld( osg::Plane(0.0f, 0.0f, 1.0f, 0.0f), gravity );
    }
    
    void addPhysicsAirplane( osg::Node* node, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        
        osg::ComputeBoundsVisitor cbv;
        node->accept( cbv );
        const osg::BoundingBox& bb = cbv.getBoundingBox();

        // osg::Vec3 half_length ((bb.xMax() - bb.xMin()) /2.0f ,(bb.yMax() - bb.yMin())/2.0f,(bb.zMax() - bb.zMin())/2.0f);
        float xm = bb.xMax();
        float ym = bb.yMax();
        float zm = bb.zMax();
        
        osg::MatrixTransform* positioned = new osg::MatrixTransform(osg::Matrix::translate(osg::Vec3(0.0,0.0,-(bb.zMax() - bb.zMin())/2.0f)));
        positioned->addChild(node);

        osg::Vec3 half_length ( (bb.xMax() - bb.xMin())/2.0f,(bb.yMax() - bb.yMin()) /2.0f ,(bb.zMax() - bb.zMin())/2.0f );
        phys::sys()->createBox( id, half_length/*shape->getHalfLengths()*/, mass );
        addPhysicsData( id, positioned, pos, vel, mass );
    }

    void addPhysicsBox( osg::Box* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        phys::sys()->createBox( id, shape->getHalfLengths(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }
    
    void addPhysicsSphere( osg::Sphere* shape, const osg::Vec3& pos, const osg::Vec3& vel, double mass )
    {
        int id = _physicsNodes.size();
        phys::sys()->createSphere( id, shape->getRadius(), mass );
        addPhysicsData( id, shape, pos, vel, mass );
    }
    
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
    {
        osgViewer::View* view = static_cast<osgViewer::View*>( &aa );
        if ( !view || !_root ) return false;
        
        switch ( ea.getEventType() )
        {
        case osgGA::GUIEventAdapter::KEYUP:
            if ( ea.getKey()==osgGA::GUIEventAdapter::KEY_Return )
            {
                osg::Vec3 eye, center, up, dir;
                view->getCamera()->getViewMatrixAsLookAt( eye, center, up );
                dir = center - eye; dir.normalize();
                addPhysicsSphere( new osg::Sphere(osg::Vec3(), 0.5f), eye, dir * 60.0f, 2.0 );
            }
            break;
        case osgGA::GUIEventAdapter::FRAME:
            {
                double dt = _hr_timer.get_delta();
                phys::sys()->update( /*0.02*/dt );
                for ( NodeMap::iterator itr=_physicsNodes.begin();
                      itr!=_physicsNodes.end(); ++itr )
                {
                    osg::Matrix matrix = phys::sys()->getMatrix(itr->first);
                    itr->second->setMatrix( matrix );
                }
            }
            break;
        default: break;
        }
        return false;
    }
    
protected:
    void addPhysicsData( int id, osg::Shape* shape, const osg::Vec3& pos,
                         const osg::Vec3& vel, double mass )
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable( new osg::ShapeDrawable(shape) );
        
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( geode.get() );
        _root->addChild( mt.get() );
        
        phys::sys()->setMatrix( id, osg::Matrix::translate(pos) );
        phys::sys()->setVelocity( id, vel );
        _physicsNodes[id] = mt;
    }
    
    void addPhysicsData( int id, osg::Node* node, const osg::Vec3& pos,
        const osg::Vec3& vel, double mass )
    {
        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;
        mt->addChild( node/*.get()*/ );
        _root->addChild( mt.get() );

        phys::sys()->setMatrix( id, osg::Matrix::translate(pos) );
        phys::sys()->setVelocity( id, vel );
        _physicsNodes[id] = mt;
    }

    typedef std::map<int, osg::observer_ptr<osg::MatrixTransform> > NodeMap;
    NodeMap _physicsNodes;
    osg::observer_ptr<osg::Group> _root;
    high_res_timer                _hr_timer;
};


int main_bi( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    int mode = 0;
    if ( arguments.read("--rigid") ) mode = 0;
    //else if ( arguments.read("--cloth") ) mode = 1;
    
    osg::Node*  airplane = creators::loadAirplane("a_319");


    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osgGA::GUIEventHandler> updater;
    switch ( mode )
    {
    case 0:
        {
            SampleRigidUpdater* rigidUpdater = new SampleRigidUpdater( root.get() );
            rigidUpdater->addGround( osg::Vec3(0.0f, 0.0f,-9.8f) );
            for ( unsigned int i=0; i<10; ++i )
            {
                for ( unsigned int j=0; j<10; ++j )
                {
                    rigidUpdater->addPhysicsBox( new osg::Box(osg::Vec3(), 0.99f),
                        osg::Vec3((float)i, 0.0f, (float)j+0.5f), osg::Vec3(), 1.0f );
                }
            }

            rigidUpdater->addPhysicsAirplane( airplane,
                osg::Vec3(-25.0f, -25.0f, 50.0f), osg::Vec3(), 1.0f );

            updater = rigidUpdater;
        }
        break;
    case 1:
        break;
    default: break;
    }
    
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    if ( updater.valid() )
        viewer.addEventHandler( updater.get() );
    viewer.setSceneData( root.get() );
    return viewer.run();
}


AUTO_REG(main_bi)