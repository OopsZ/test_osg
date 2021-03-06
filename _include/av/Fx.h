#pragma once

//
//  FoamSreamFx interface
//

// base info
struct particles_effect_info
{
	virtual float getMaxParticleLifetime() const = 0;
	virtual bool  isQueueEmpty() const = 0;
};

// FoamStream
struct FoamStreamSfxNode : particles_effect_info
{
	virtual void  setIntensity( float inten ) = 0;
	virtual float getIntensity() const = 0;

#if 0
	virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldDir() const = 0;
#endif

	virtual void setEmitterWorldVelocity( cg::point_3f const & velocity ) = 0;
	virtual cg::point_3f const & getEmitterWorldVelocity() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};

//
//  SmokeFx interface
//

// Smoke
struct SmokeSfxNode : particles_effect_info
{
	virtual void  setIntensity( float inten ) = 0;
	virtual float getIntensity() const = 0;

#if 0
	virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldDir() const = 0;
#endif
	
	virtual void setEmitWorldPos( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldPos() const = 0;

	virtual void setEmitterWorldVelocity( cg::point_3f const & velocity ) = 0;
	virtual cg::point_3f const & getEmitterWorldVelocity() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};

// Fire
struct FireSfxNode : particles_effect_info
{
    virtual void  setIntensity( float inten ) = 0;
    virtual float getIntensity() const = 0;
    
    virtual void  setBaseAlpha( float alpha )= 0;
    virtual float getBaseAlpha() const = 0;

#if 0
    virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
    virtual cg::point_3f const & getEmitWorldDir() const = 0;
#endif

    virtual void setEmitWorldPos( cg::point_3f const & dir ) = 0;
    virtual cg::point_3f const & getEmitWorldPos() const = 0;

    virtual void setEmitterWorldVelocity( cg::point_3f const & velocity ) = 0;
    virtual cg::point_3f const & getEmitterWorldVelocity() const = 0;

    virtual void  setFactor( float factor ) = 0;
    virtual float getFactor() const = 0;

    virtual void  setBreakDistance( float break_sfx_distance ) = 0 ;
    virtual float getBreakDistance() const = 0 ;

};

// Sparks
struct SparksSfxNode : particles_effect_info
{
	virtual void setContactFlag( bool flag ) = 0;
	virtual bool getContactFlag() const = 0;


	virtual void setEmitterWorldVelocity( cg::point_3f const & velocity ) = 0;
	virtual cg::point_3f const & getEmitterWorldVelocity() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};

// Friction dust
struct FrictionDustSfxNode : particles_effect_info
{
	virtual void setContactFlag( bool flag ) = 0;
	virtual bool getContactFlag() const = 0;

	virtual void setEmitterWorldVelocity( cg::point_3f const & velocity ) = 0;
	virtual cg::point_3f const & getEmitterWorldVelocity() const = 0;
};

// Landing dust
struct LandingDustSfxNode : particles_effect_info
{
	// inject particles
	virtual void makeContactDust( float timestamp, cg::point_3f const & contact_pos, cg::point_3f const & contact_speed ) = 0;
};