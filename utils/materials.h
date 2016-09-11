#pragma once 

#include "visitors/materials_visitor.h"
#include "utils/cpp_utils/singleton.h"

namespace mat
{

    struct reader
    {
        reader();

        reader(std::string full_path);
        mat_profiles_t  read (std::string full_path);
        mat_profiles_t  get () {return mats_;}
	private:
		void         init_();

    private:
        mat_profiles_t               mats_;
		std::set<std::string>  valid_mats_;
    };



}

namespace creators 
{
    class programsHolder_base {
    public:
        struct program_t
        {
            osg::ref_ptr<osg::Program> program;
        };
    };

    class texturesHolder_base {
    public:
        virtual osg::ref_ptr<osg::TextureCubeMap>   getEnvTexture()      = 0;
        virtual osg::ref_ptr<osg::Texture2D>        getDecalTexture()    = 0;
        virtual osg::ref_ptr<osg::Texture2D>        getLightMapTexture() = 0;
		virtual osg::ref_ptr<osg::Texture2D>        getReflTexture()     = 0;
    };    

    texturesHolder_base&                            getTextureHolder();

    programsHolder_base::program_t                  createProgram(const std::string& mat_name, const std::string& preprocessorDefinitions = std::string());

    void createMaterial     (osg::Node* node , osg::StateSet* stateset,const std::string& model_name,const std::string& mat_suffix,const std::string& mat_name,const mat::mat_profiles_t& m);
    void createMaterialLite (osg::Node* node , osg::StateSet* stateset,const std::string& model_name,const std::string& mat_name,const mat::mat_profiles_t& m);
    void computeAttributes  (osg::Node* node , std::string mat_name);
}


