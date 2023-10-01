#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/RenderPipeline/RenderPipeline.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/SceneResource.h>
#include <Urho3D/VisualTest/VisualTest.h>
#include <Urho3D/VisualTest/VisualTestCapture.h>
#include <Urho3D/VisualTest/VisualTestHarness.h>

namespace Urho3D
{

// The test resources
static const ea::string RESOURCE_SCENE("VisualTests/Scenes/RealtimeLights.scene");
static const ea::string RESOURCE_LIGHTSHAPE("VisualTests/Textures/DebugUVTiles.png");


static SharedPtr<Scene> SetupRealtimeLightsScene(VisualTest* test)
{
    ResourceCache* cache = test->GetSubsystem<ResourceCache>();

    // Load scene
    SharedPtr<Scene> scene;
    auto sceneResource = cache->GetTempResource<SceneResource>(RESOURCE_SCENE);
    if (!sceneResource || !sceneResource->GetScene()) {
        test->Fail("Couldn't load scene ({})", RESOURCE_SCENE);
        return nullptr;
    }
    scene = sceneResource->GetScene();

    // Setup light properties
    bool castShadows = test->GetSwitch("Shadows");
    bool vsmShadows = test->GetSwitch("VSM");
    int pcfKernel = 1;
    if (test->GetSwitch("PCF3")) {
        pcfKernel = 3;
    }
    if (test->GetSwitch("PCF5")) {
        pcfKernel = 5;
    }

    // Setup light shape texture
    Texture2D* lightShapeTexture = nullptr;
    if (test->GetSwitch("LightShape")) {
        lightShapeTexture = cache->GetResource<Texture2D>(RESOURCE_LIGHTSHAPE);
        if (!lightShapeTexture) {
            test->Fail("Couldn't load light shape texture ({})", RESOURCE_LIGHTSHAPE);
            return nullptr;
        }
    }


    // Setup lights in the scene
    Node* lightsRoot = scene->GetChild("Lights");
    if (!lightsRoot) {
        test->Fail("Couldn't find 'Lights' node in scene");
        return nullptr;
    }

    ea::vector<Node*> lightNodes;
    lightsRoot->GetChildrenWithComponent<Light>(lightNodes);
    for (Node* lightNode : lightNodes) {
        bool lightOn = test->GetSwitch("AllLights") || test->GetSwitch(lightNode->GetName());
        Light* lightObject = lightNode->GetComponent<Light>();
        if (!lightObject) {
            test->Fail("Missing light ({}) in scene", lightNode->GetName());
            return nullptr;
        }
        lightNode->SetEnabledRecursive(lightOn);
        lightObject->SetCastShadows(castShadows);
        lightObject->SetShapeTexture(lightShapeTexture);
    }

    // setup pipeline options
    auto* pipeline = scene->GetComponent<RenderPipeline>();
    RenderPipelineSettings pipelineSettings = pipeline->GetSettings();
    pipelineSettings.sceneProcessor_.enableShadows_ = true;
    pipelineSettings.sceneProcessor_.pcfKernelSize_ = pcfKernel;
    pipelineSettings.shadowMapAllocator_.enableVarianceShadowMaps_ = vsmShadows;
    pipeline->SetSettings(pipelineSettings);

    // return the scene
    return scene;
}


static VisualTest* DefineTest(VisualTestHarness* harness, const ea::string& light, const ea::string& variant, const StringVector& features)
{
    ea::string name = Format("RealtimeLights/{}_{}", light, variant);

    VisualTest* test = harness->CreateTest();
    test->SetName(name);
    test->SetSwitch(light);
    for (auto& feature : features) {
        test->SetSwitch(feature);
    }
    test->SetSceneLoader(SetupRealtimeLightsScene);
    return test;
}


static void AddLightTests(VisualTestHarness* harness, const ea::string& light)
{
    DefineTest(harness, light, "Base", {});
    DefineTest(harness, light, "Shadows", {"Shadows" });
    DefineTest(harness, light, "Shadows_PCF3", { "Shadows", "PCF3" });
    DefineTest(harness, light, "Shadows_PCF5", { "Shadows", "PCF5" });
    DefineTest(harness, light, "Shadows_VSM", {"Shadows", "VSM"});
    DefineTest(harness, light, "LightShape", { "Shadows", "LightShape" });
}


void CreateRealtimeLightTests(VisualTestHarness* harness)
{
    AddLightTests(harness, "GlobalLight");
    AddLightTests(harness, "PointLight");
    AddLightTests(harness, "SpotLight");
    AddLightTests(harness, "AllLights");
}


}
