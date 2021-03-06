#pragma once

#include <osg/Drawable>
#include <OpenThreads/Mutex>

namespace avGui 
{
    class CEGUIEventHandler : public osgGA::GUIEventHandler
    {
    public:
        CEGUIEventHandler( osg::Camera* camera ) : _camera(camera) {}

        virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );


    protected:
        CEGUI::MouseButton convertMouseButton( int button );

        osg::observer_ptr<osg::Camera> _camera;
    };



class CEGUIDrawable : public osg::Drawable
{
public:
    CEGUIDrawable();
    CEGUIDrawable( const CEGUIDrawable& copy,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY );
    META_Object( osg, CEGUIDrawable );
    
    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
    
    bool handleClose( const CEGUI::EventArgs& e );

    DECLARE_EVENT(ready_for_init  , ());

protected:
    virtual ~CEGUIDrawable() {}
    
    mutable double _lastSimulationTime;
    mutable unsigned int _activeContextID;
    static  OpenThreads::Mutex  _mutex;
    static   bool    _initialized;
};

}
