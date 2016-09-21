#include "stdafx.h"
#ifndef Q_MOC_RUN
#include <SPK.h>
#include <SPK_GL.h>
#endif
#include "SparkDrawable.h"
#include <algorithm>
#include <iostream>

typedef std::map<std::string, SparkDrawable::ImageAttribute> TextureObjMap;
typedef std::vector<SPK::System*>             ParticleSystemList;


struct SparkDrawable::Private
{
       Private(SparkDrawable* parent)
       : _baseSystemCreator(NULL)
       , _baseSystemID(SPK::NO_ID)
       , _protoSystem(NULL)
       , _parent(parent)
       {}

    SPK::System* createParticleSystem( const SPK::Vector3D& pos, const SPK::Vector3D& rot, float angle );

    TextureObjMap           _textureObjMap;
    ParticleSystemList      _particleSystems;
    CreateBaseSystemFunc    _baseSystemCreator;
    mutable SPK::SPK_ID     _baseSystemID;

    mutable SPK::System*    _protoSystem;

    SparkDrawable*          _parent;
};

class SparkDrawable::DeferredSystemHandler : public osg::Drawable::UpdateCallback
{
public:
    virtual void update( osg::NodeVisitor* nv, osg::Drawable* drawable );

    struct PosAndRotate
    {
        SPK::Vector3D position;
        SPK::Vector3D rotationAxis;
        float rotationAngle;
    };

    std::vector<PosAndRotate> _newSystemsToAdd;
};


struct SparkDrawable::SortParticlesOperator
{
    SortParticlesOperator( const osg::Vec3d& eye )
    { _eye.x = eye.x(); _eye.y = eye.y(); _eye.z = eye.z(); }

    virtual bool operator()( SPK::System* lhs, SPK::System* rhs )
    {
        return SPK::getSqrDist(lhs->getWorldTransformPos(), _eye) >
            SPK::getSqrDist(rhs->getWorldTransformPos(), _eye);
    }

    SPK::Vector3D _eye;
};


bool SparkDrawable::isValid() const 
{
    return getBaseSystemID()!=SPK::NO_ID && p_->_protoSystem!=NULL; 
}

void SparkDrawable::setBaseSystemID( SPK::SPK_ID id ) { p_->_baseSystemID = id; }
SPK::SPK_ID SparkDrawable::getBaseSystemID() const { return p_->_baseSystemID; }

void SparkDrawable::setSortParticles( bool b ) { _sortParticles = b; }
bool SparkDrawable::getSortParticles() const { return _sortParticles; }

void SparkDrawable::setAutoUpdateBound( bool b ) { _autoUpdateBound = b; }
bool SparkDrawable::getAutoUpdateBound() const { return _autoUpdateBound; }

SPK::System* SparkDrawable::getProtoSystem() { return p_->_protoSystem; }
const SPK::System* SparkDrawable::getProtoSystem() const { return p_->_protoSystem; }

SPK::System* SparkDrawable::getParticleSystem( unsigned int i ) { return p_->_particleSystems[i]; }
const SPK::System* SparkDrawable::getParticleSystem( unsigned int i ) const { return p_->_particleSystems[i]; }
unsigned int SparkDrawable::getNumParticleSystems() const { return p_->_particleSystems.size(); }

void SparkDrawable::addExternalParticleSystem( SPK::System* system )
{
    if (system)
        p_->_particleSystems.push_back(system);
}

void SparkDrawable::setBaseSystemCreator( CreateBaseSystemFunc func, bool useProtoSystem )
{
    p_->_baseSystemCreator = func;
    _useProtoSystem = useProtoSystem;
    _dirty = true;
}


/* SparkDrawable::DeferredSystemHandler */

void SparkDrawable::DeferredSystemHandler::update( osg::NodeVisitor* nv, osg::Drawable* drawable )
{
    SparkDrawable* spark = static_cast<SparkDrawable*>(drawable);
    if ( !spark || !spark->isValid() ) return;
    
    if ( _newSystemsToAdd.size()>0 )
    {
        for ( unsigned int i=0; i<_newSystemsToAdd.size(); ++i )
        {
            const PosAndRotate& pr = _newSystemsToAdd[i];
            spark->p_->createParticleSystem( pr.position, pr.rotationAxis, pr.rotationAngle );
        }
        _newSystemsToAdd.clear();
    }
}

/* SparkDrawable */

SparkDrawable::SparkDrawable()
    :   _lastTime(-1.0)
    , _sortParticles(false)
    , _useProtoSystem(true)
    , _autoUpdateBound(true)
    , _dirty(true)
    , p_ (new Private(this))
{
    _activeContextID = 0;
    setUpdateCallback( new DeferredSystemHandler );
    setSupportsDisplayList( false );
    setDataVariance( osg::Object::DYNAMIC );
}
    
SparkDrawable::SparkDrawable( const SparkDrawable& copy,const osg::CopyOp& copyop )
    : osg::Drawable(copy, copyop)
    , _activeContextID(copy._activeContextID)
    , _lastTime(copy._lastTime), _sortParticles(copy._sortParticles)
    , _useProtoSystem(copy._useProtoSystem), _autoUpdateBound(copy._autoUpdateBound)
    , _dirty(copy._dirty)
    , p_ (copy.p_)
{}

SparkDrawable::~SparkDrawable()
{
    for ( ParticleSystemList::const_iterator itr=p_->_particleSystems.begin();
          itr!=p_->_particleSystems.end(); ++itr )
    {
        destroyParticleSystem( *itr, false );
    }
    if ( p_->_protoSystem )
        destroyParticleSystem( p_->_protoSystem, false );

    delete p_;
}

void SparkDrawable::destroyParticleSystem( SPK::System* system, bool removeFromList )
{
    if (system) SPK_Destroy(system);
    if (removeFromList)
    {
        unsigned int i = 0, size = p_->_particleSystems.size();
        for (; i<size; ++i) { if (p_->_particleSystems[i]==system) break; }
        if (i<size) p_->_particleSystems.erase(p_->_particleSystems.begin() + i);
    }
}

unsigned int SparkDrawable::getNumParticles() const
{
    unsigned int count = 0;
    if ( _useProtoSystem && p_->_protoSystem )
        count += p_->_protoSystem->getNbParticles();
    for ( ParticleSystemList::const_iterator itr=p_->_particleSystems.begin();
          itr!=p_->_particleSystems.end(); ++itr )
    {
        count += (*itr)->getNbParticles();
    }
    return count;
}

void SparkDrawable::setGlobalTransformMatrix( const osg::Matrix& matrix, bool useOffset )
{
    osg::Vec3d trans = matrix.getTrans();
    osg::Quat quat = matrix.getRotate();
    osg::Vec3d axis; double angle = 0.0f;
    quat.getRotate( angle, axis );
    
    SPK::Vector3D pos(trans.x(), trans.y(), trans.z());
    SPK::Vector3D rot(axis.x(), axis.y(), axis.z());
    if ( _useProtoSystem && p_->_protoSystem )
        setTransformMatrix( p_->_protoSystem, pos, rot, angle, useOffset );
    for ( ParticleSystemList::const_iterator itr=p_->_particleSystems.begin();
          itr!=p_->_particleSystems.end(); ++itr )
    {
        setTransformMatrix( *itr, pos, rot, angle, useOffset );
    }
}

void SparkDrawable::setTransformMatrix( SPK::System* system, const SPK::Vector3D& pos, const SPK::Vector3D& rot,
                                        float angle, bool useOffset )
{
    if ( useOffset )
    {
        system->setTransformPosition( pos + system->getLocalTransformPos() );
        system->setTransformOrientation( rot, angle );  // FIXME: how to get rotation offset?
    }
    else
    {
        system->setTransformPosition( pos );
        system->setTransformOrientation( rot, angle );
    }
    system->updateTransform();
}

unsigned int SparkDrawable::addParticleSystem( const osg::Vec3& p, const osg::Quat& r )
{
    DeferredSystemHandler* updater = dynamic_cast<DeferredSystemHandler*>( getUpdateCallback() );
    if ( updater )
    {
        osg::Vec3 axis; double angle = 0.0f;
        r.getRotate( angle, axis );
        
        DeferredSystemHandler::PosAndRotate pr;
        pr.position = SPK::Vector3D( p.x(), p.y(), p.z() );
        pr.rotationAxis = SPK::Vector3D( axis.x(), axis.y(), axis.z() );
        pr.rotationAngle = angle;
        updater->_newSystemsToAdd.push_back( pr );
    }
    return p_->_particleSystems.size() + updater->_newSystemsToAdd.size() - 1;
}

void SparkDrawable::addImage( const std::string& name, osg::Image* image, GLuint type, GLuint clamp )
{
    if ( image )
    {
        ImageAttribute attr;
        attr.image = image;
        attr.type = type;
        attr.clamp = clamp;
        p_->_textureObjMap[name] = attr;
    }
}

bool SparkDrawable::update( double currentTime, const osg::Vec3d& eye )
{
    bool active = false;
    if ( _lastTime>0.0 )
    {
        if ( _sortParticles )
            std::sort( p_->_particleSystems.begin(), p_->_particleSystems.end(), SortParticlesOperator(eye) );
        
        double deltaTime = currentTime - _lastTime;
        SPK::Vector3D eyePos(eye.x(), eye.y(), eye.z());
        if ( _useProtoSystem && p_->_protoSystem )
        {
            p_->_protoSystem->setCameraPosition( eyePos );
            active = p_->_protoSystem->update(deltaTime);
        }
        
        ParticleSystemList::iterator itr = p_->_particleSystems.begin();
        while( itr!=p_->_particleSystems.end() )
        {
            (*itr)->setCameraPosition( eyePos );
            if ( !(*itr)->update(deltaTime) )
            {
                destroyParticleSystem( *itr, false );
                itr = p_->_particleSystems.erase( itr );
            }
            else
            {
                active = true;
                ++itr;
            }
        }
        
        if ( _autoUpdateBound )
            dirtyBound();  // Update the particle bound for near/far computing and culling
    }
    else
        active = true;
    
    _lastTime = currentTime;
    return active;
}

#if OSG_MIN_VERSION_REQUIRED(3,3,2)
osg::BoundingBox SparkDrawable::computeBoundingBox() const
#else
osg::BoundingBox SparkDrawable::computeBound() const
#endif
{
    osg::BoundingBox bb;
    SPK::Vector3D min, max;
    if ( _useProtoSystem && p_->_protoSystem )
    {
        if ( p_->_protoSystem->isAABBComputingEnabled() )
        {
            p_->_protoSystem->computeAABB();
            min = p_->_protoSystem->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = p_->_protoSystem->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    
    for ( ParticleSystemList::const_iterator itr=p_->_particleSystems.begin();
          itr!=p_->_particleSystems.end(); ++itr )
    {
        SPK::System* system = *itr;
        if ( system->isAABBComputingEnabled() )
        {
            system->computeAABB();
            min = system->getAABBMin(); bb.expandBy( osg::Vec3(min.x, min.y, min.z) );
            max = system->getAABBMax(); bb.expandBy( osg::Vec3(max.x, max.y, max.z) );
        }
    }
    return bb;
}

void SparkDrawable::drawImplementation( osg::RenderInfo& renderInfo ) const
{
    unsigned int contextID = renderInfo.getContextID();
    if ( _dirty )
    {
        if ( p_->_baseSystemCreator )
        {
            TextureIDMap textureIDMap;
            for ( TextureObjMap::const_iterator itr=p_->_textureObjMap.begin();
                  itr!=p_->_textureObjMap.end(); ++itr )
            {
                const ImageAttribute& attr = itr->second;
                textureIDMap[itr->first] =
                    compileInternalTexture(attr.image.get(), attr.type, attr.clamp);
            }
            p_->_baseSystemID = (*p_->_baseSystemCreator)( textureIDMap, 800, 600 );
            p_->_protoSystem = SPK_Get( SPK::System, p_->_baseSystemID );
        }
        
        _activeContextID = contextID;
        _dirty = false;
    }
    
    osg::State* state = renderInfo.getState();
    state->disableAllVertexArrays();
    
    auto tex_unit = state->getActiveTextureUnit();

    // Make sure the client unit and active unit are unified
    state->setClientActiveTextureUnit( 0 );
    state->setActiveTextureUnit( 0 );


    SPK::GL::GLRenderer::saveGLStates();
    if ( _useProtoSystem && p_->_protoSystem )
        p_->_protoSystem->render();
    for ( ParticleSystemList::const_iterator itr=p_->_particleSystems.begin();
          itr!=p_->_particleSystems.end(); ++itr )
    {
        (*itr)->render();
    }
    SPK::GL::GLRenderer::restoreGLStates();

    state->setActiveTextureUnit( tex_unit );
}

SPK::System* SparkDrawable::Private::createParticleSystem( const SPK::Vector3D& pos, const SPK::Vector3D& rot, float angle )
{
    SPK::System* system = SPK_Copy( SPK::System, _baseSystemID );
    if ( !system ) return NULL;
    else _parent->setTransformMatrix( system, pos, rot, angle );
    
    _particleSystems.push_back( system );
    return system;
}

GLuint SparkDrawable::compileInternalTexture( osg::Image* image, GLuint type, GLuint clamp ) const
{
    GLuint index;
    glGenTextures( 1, &index );
    glBindTexture( GL_TEXTURE_2D, index );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clamp );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    unsigned int numCurrent = osg::Image::computeNumComponents( image->getPixelFormat() );
    unsigned int numRequired = osg::Image::computeNumComponents( type );
    if ( numCurrent!=numRequired && image->getDataType()==GL_UNSIGNED_BYTE )
        convertData( image, type, numCurrent, numRequired );
    
    /*if ( mipmap )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        gluBuild2DMipmaps( GL_TEXTURE_2D, type, image->s(), image->t(),
                           type, GL_UNSIGNED_BYTE, image->data() );
    }
    else*/
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        glTexImage2D( GL_TEXTURE_2D, 0, type, image->s(), image->t(), 0,
                      type, GL_UNSIGNED_BYTE, image->data() );
    }
    return index;
}

void SparkDrawable::convertData( osg::Image* image, GLuint type,
                                 unsigned int numCurrent, unsigned int numRequired ) const
{
    int newRowWidth = osg::Image::computeRowWidthInBytes(image->s(), type, GL_UNSIGNED_BYTE, 1);
    unsigned char* newData = new unsigned char[newRowWidth * image->t() * image->r()];
    
    for ( int t=0; t<image->t(); ++t )
    {
        unsigned char* source = image->data(0, t);
        unsigned char* dest = newData + t * newRowWidth;
        for ( int s=0; s<image->s(); ++s )
        {
            if ( numRequired==1 )  // RGB/RGBA -> ALPHA
            {
                *dest++ = *source;
                source += numCurrent;
            }
            else
            {
                OSG_WARN << image->getFileName() << ": no conversation from "
                         << numCurrent << " elements to " << numRequired << " elements" << std::endl;
            }
        }
    }
    image->setImage( image->s(), image->t(), image->r(), numRequired, type,
                     GL_UNSIGNED_BYTE, newData, osg::Image::USE_NEW_DELETE );
}
