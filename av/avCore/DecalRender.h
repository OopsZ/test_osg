#pragma once


namespace avCore
{

    typedef osg::Texture2D               ITexture;
    typedef osg::ref_ptr<osg::Texture2D> ITexturePtr;

    struct DecalRenderer : osg::Group
    {
        virtual void SetVisible( bool vis ) = 0;
        virtual void Clear() =0;
        virtual void AddPolyline( std::vector<cg::point_2f> const & positions, cg::colorf const & col, float w )=0;

    private:
        class _private;
        std::shared_ptr<_private>  _impl; 
    };

    typedef osg::ref_ptr<DecalRenderer> IDecalRendererPtr;


    // create method
    IDecalRendererPtr createDecalRenderer(osg::Group * sceneRoot);

}