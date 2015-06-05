#include "stdafx.h"

#include "Scene.h"
#include "Lights.h"

#include "LightManager.h"

#include "utils/callbacks.h"



namespace avScene
{


osg::ref_ptr<LightManager> LightManager::g_Instance;


enum LightManagerMessage
{
    CreateDynamicLight,
    DeleteDynamicLight,
    UpdateDynamicLight,
};

typedef std::map<std::string, LightManagerMessage> LightManagerMessageMap;
typedef LightManagerMessageMap::value_type LightManagerMessageMapValue;

static const LightManagerMessageMapValue g_MessageTable[] =
{
    LightManagerMessageMapValue("CreateDynamicLight", CreateDynamicLight),
    LightManagerMessageMapValue("DeleteDynamicLight", DeleteDynamicLight),
    LightManagerMessageMapValue("UpdateDynamicLight", UpdateDynamicLight),
};
static const unsigned g_nMessageTableSize = sizeof(g_MessageTable) / sizeof(g_MessageTable[0]);
static const LightManagerMessageMap g_MessageMap(&g_MessageTable[0], &g_MessageTable[0] + g_nMessageTableSize);


struct LightManager::Light
{
    osg::MatrixTransform * transform;

    cg::range_2f spotFalloff;
    cg::range_2f distanceFalloff;
    cg::colorf color;

    cg::point_3f position;
    cg::vector_3f direction;

    bool active;

    inline Light() : transform(NULL), active(false) { }
};

LightManager::LightManager()
{
    setDataVariance(DYNAMIC);
    setUpdateCallback(utils::makeNodeCallback(this, &LightManager::update));
}

LightManager::~LightManager()
{
}

void LightManager::Create()
{
    avAssert(!g_Instance.valid());
    g_Instance = new LightManager();
}

void LightManager::Release()
{
    avAssert(g_Instance.valid());
    g_Instance = NULL;
}

LightManager * LightManager::GetInstance()
{
    return g_Instance;
}


#if 0
void LightManager::OnEvent( const char * name, svCore::MessageManager::MessageStream & stream )
{
    avAssert(name != NULL);

    const LightManagerMessageMap::const_iterator it = g_MessageMap.find(name);

    if (it == g_MessageMap.end())
    {
        svError("Unknown event");
        return;
    }

    const LightManagerMessage message = it->second;

    switch (message)
    {
    case CreateDynamicLight:
        {
            const unsigned lightID = stream.get<svCore::uint32>();

            Light & light = m_LightsMap[lightID];

            const unsigned objectID = stream.get<svCore::uint32>();
            svDynamicObject::VisualDynamicObject * dynamicObject = svCore::ObjectManager::GetInstance()->GetObjectByID<svDynamicObject::VisualDynamicObject>(svCore::ObjectBase::OBJECT_DYNAMIC, objectID);
            avAssert(dynamicObject != NULL);
            light.transform = dynamicObject->GetVisualModel();

            const float spotFalloff0 = osg::DegreesToRadians(stream.get<float>());
            const float spotFalloff1 = osg::DegreesToRadians(stream.get<float>());
            light.spotFalloff = cg::range_2f(spotFalloff0, spotFalloff1);

            const float distanceFalloff0 = stream.get<float>();
            const float distanceFalloff1 = stream.get<float>();
            light.distanceFalloff = cg::range_2f(distanceFalloff0, distanceFalloff1);

            light.color.r = stream.get<float>();
            light.color.g = stream.get<float>();
            light.color.b = stream.get<float>();
        }
        break;

    case DeleteDynamicLight:
        {
            const unsigned lightID = stream.get<svCore::uint32>();

            LightsMap::const_iterator it = m_LightsMap.find(lightID);
            avAssert(it != m_LightsMap.end());

            m_LightsMap.erase(it);
        }
        break;

    case UpdateDynamicLight:
        {
            const unsigned lightID = stream.get<svCore::uint32>();
            avAssert(m_LightsMap.find(lightID) != m_LightsMap.end());

            Light & light = m_LightsMap[lightID];

            light.position.x = stream.get<double>();
            light.position.y = stream.get<double>();
            light.position.z = stream.get<double>();
            std::swap(light.position.x, light.position.y);

            const float heading = osg::DegreesToRadians(stream.get<float>());
            const float pitch = osg::DegreesToRadians(stream.get<float>());
            light.direction = cg::vector_3(cos(pitch) * sin(heading), cos(pitch) * cos(heading), sin(pitch));

            light.active = stream.get<bool>();
        }
        break;

    default:
        svError("message not handled");
        break;
    }
}
#endif

void  LightManager::addLight(uint32_t id, osg::MatrixTransform* mt )
{
    const unsigned lightID = id;
    
    //if(m_LightsMap.find(lightID) == m_LightsMap.end())
    //   m_LightsMap.insert(std::make_pair(lightID,Light()));
    
    Light & light = m_LightsMap[lightID];

    // FIXME
    // light.transform = dynamicObject->GetVisualModel();
    light.transform = mt;

    const float spotFalloff0 = osg::DegreesToRadians(0.f);
    const float spotFalloff1 = osg::DegreesToRadians(359.f);
    light.spotFalloff = cg::range_2f(spotFalloff0, spotFalloff1);

    const float distanceFalloff0 = 0;
    const float distanceFalloff1 = 50000;
    light.distanceFalloff = cg::range_2f(distanceFalloff0, distanceFalloff1);

    light.color.r = 255;
    light.color.g = 0;
    light.color.b = 0;

    light.direction = as_vector(cg::point_3f(-100.f,-100.f,-100.f));
    light.position = cg::point_3f(0,0,0);

    light.active = true;

}


void LightManager::update( osg::NodeVisitor * nv )
{
    avScene::Scene * scene = GetScene();

    if (scene == NULL)
        return;

    avScene::Lights * lights = scene->getLights();

    if (lights == NULL)
        return;

    for (LightsMap::const_iterator it = m_LightsMap.cbegin(); it != m_LightsMap.cend(); ++it)
    {
        const Light & light = it->second;

        if (!light.active)
            continue;


      
#if 0
        osg::Matrix matrix = light.transform->getMatrix() * svCore::GetCoordinateSystem()->GetLCS2LTPMatrix();
        matrix4d transform = matrix4d(matrix.ptr(), matrix::unscaled).transpose();
#else
        osg::Matrix matrix = light.transform->getMatrix() ;
        cg::transform_4 transform = from_osg_transform(matrix);
#endif

        const cg::point_3f position = transform.treat_point(light.position );
        const cg::vector_3 direction = transform.treat_vector(light.direction);

        lights->AddLight(avScene::GlobalInfluence, avScene::ConicalLight,
            position, direction, light.distanceFalloff, light.spotFalloff, 
            light.color, 0.60f, 0.35f);
    }

}


} // namespace svScene
