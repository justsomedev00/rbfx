#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Scene/SceneResource.h>
#include <Urho3D/VisualTest/VisualTest.h>
#include <Urho3D/VisualTest/VisualTestCapture.h>
#include <Urho3D/VisualTest/VisualTestDefs.h>
#include <Urho3D/VisualTest/VisualTestEvents.h>
#include <Urho3D/VisualTest/VisualTestHarness.h>
#include <Urho3D/VisualTest/VisualTestImageCompare.h>


namespace Urho3D
{

VisualTest::VisualTest(Context* context) :
    Object(context)
{}


void VisualTest::RegisterObject(Context* context)
{
    context->AddFactoryReflection<VisualTest>();
}


const Variant& VisualTest::GetVariable(const ea::string& var) const
{
    auto it = variables_.find(var);
    return (it != variables_.end()) ? it->second : Variant::EMPTY;
}


void VisualTest::SetVariable(const ea::string& var, const Variant& value)
{
    variables_[var] = value;
}


bool VisualTest::HasCapturesRemaining() const
{
    if (!scene_) {
        return false;
    }

    ea::vector<VisualTestCapture*> captures;
    scene_->GetDerivedComponents(captures, true);

    for (VisualTestCapture* capture : captures) {
        if (!capture->IsDone()) {
            return true;
        }
    }

    return false;
}


void VisualTest::Start()
{
    state_ = VisualTestState::Starting;

    // Ensure name assigned for test
    if (name_.empty()) {
        Fail("No name assigned for test");
        return;
    }

    // Ensure output directory valid
    if (GetOutputPath().empty()) {
        Fail("No output path for test");
        return;
    }

    // Ensure output directory exists
    if (!EnsureOutputDirectoryExists()) {
        Fail("Couldn't create output directory");
        return;
    }

    // Ensure output directory is clean
    if (!CleanOutputDirectory()) {
        Fail("Couldn't clean output directory");
        return;
    }

    // Ensure valid scene loader
    if (!sceneLoader_) {
        Fail("No scene loader assigned for test");
        return;
    }

    // Try to load scene, scene loader may mark test
    // as failed, so need to check failure status
    scene_ = sceneLoader_(this);
    if (IsFailed()) {
        return;
    }

    // Ensure scene exists
    if (!scene_) {
        Fail("No scene loaded");
        return;
    }

    // Subscribe to capture and system events
    SubscribeToEvent(scene_, E_VISUALTESTCAPTUREFAILED, URHO3D_HANDLER(VisualTest, HandleCaptureFailed));
    SubscribeToEvent(scene_, E_VISUALTESTCAPTUREIMAGEREADY, URHO3D_HANDLER(VisualTest, HandleCaptureReady));
    SubscribeToEvent(scene_, E_SCENEUPDATE, URHO3D_HANDLER(VisualTest, HandleSceneUpdate));
    SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(VisualTest, HandleFrameEnd));

    // Enter startup
    state_ = VisualTestState::Running;

    // Inoke any custom startup code
    OnStartup();
}


void VisualTest::SetSceneLoader(const SceneLoader& sceneLoader)
{
    sceneLoader_ = sceneLoader;
}


void VisualTest::FailInternal(const ea::string& message)
{
    state_ = VisualTestState::Failed;
    OnFail(message);
    Finish();
}


void VisualTest::Finish()
{
    OnFinish();

    UnsubscribeFromEvent(scene_, E_SCENEUPDATE);
    UnsubscribeFromEvent(scene_, E_VISUALTESTCAPTUREFAILED);
    UnsubscribeFromEvent(scene_, E_VISUALTESTCAPTUREIMAGEREADY);
    scene_.Reset();
}


void VisualTest::OnStartup()
{}


void VisualTest::OnFinish()
{}


void VisualTest::OnSceneUpdate(float timeStep)
{}


void VisualTest::OnCaptureFailed(VisualTestCapture* capture, const ea::string& reason)
{
    Fail("Capture ({}) failed, {}", name_, capture->GetName(), reason);
}


void VisualTest::OnCaptureImageReady(VisualTestCapture* capture)
{
    SaveCaptureImageToFile(capture);
    CompareCaptureImage(capture);
}


bool VisualTest::EnsureOutputDirectoryExists()
{
    ea::string path = GetOutputPath();

    FileSystem* fs = GetSubsystem<FileSystem>();
    if (!fs->DirExists(path)) {
        fs->CreateDirsRecursive(path);
    }
    if (fs->DirExists(path)) {
        return true;
    }
    return false;
}


bool VisualTest::CleanOutputDirectory()
{
    ea::string path = GetOutputPath();

    FileSystem* fs = GetSubsystem<FileSystem>();
    if (!fs->DirExists(path)) {
        return true;
    }

    StringVector files;
    fs->ScanDir(files, path, "", SCAN_FILES | SCAN_RECURSIVE);
    for (auto& file : files) {
        if (!fs->Delete(GetOutputPath(file))) {
            return false;
        }
    }

    return true;
}


ea::string VisualTest::GetOutputPath(const ea::string& subpath)
{
    auto* harness = GetSubsystem<VisualTestHarness>();
    if (subpath.empty()) {
        return harness->GetOutputPath(this);
    }
    return harness->GetOutputPath(this, subpath);
}


ea::string VisualTest::GetGoldenPath(const ea::string& subpath)
{
    auto* harness = GetSubsystem<VisualTestHarness>();
    return harness->GetGoldenPath(this, subpath);
}


SharedPtr<Image> VisualTest::GetGoldenImage(const ea::string& imageFileName)
{
    ResourceCache* resources = GetSubsystem<ResourceCache>();
    return resources->GetTempResource<Image>(GetGoldenPath(imageFileName));
}


void VisualTest::CompareCaptureImage(VisualTestCapture* capture)
{
    auto* harness = GetSubsystem<VisualTestHarness>();

    ea::string imageFile = capture->GetImageFileName();

    SharedPtr<Image> captureImage = capture->GetImage();
    SharedPtr<Image> goldenImage = GetGoldenImage(imageFile);

    if (!goldenImage) {
        harness->Error("Capture ({}/{}) doesn't have a golden image for comparison", GetName(), imageFile);
        return;
    }

    VisualTestImageCompareOptions options;
    auto result = CompareImages(options, captureImage, goldenImage);
    if (result.pixelMatchPercent_ < 1.0f) {
        Fail("Image ({}) did not match golden data", imageFile);
    }
}


void VisualTest::SaveCaptureImageToFile(VisualTestCapture* capture)
{
    auto* harness = GetSubsystem<VisualTestHarness>();

    ea::string filePath = GetOutputPath(capture->GetImageFileName());

    if (!capture->GetImage()) {
        harness->Error("No image produced for file ({})", filePath);
        return;
    }

    FileIdentifier fileInfo("file", filePath);
    if (!capture->GetImage()->SaveFile(fileInfo)) {
        harness->Error("Image write ({}) failed ", filePath);
        return;
    }
}


void VisualTest::OnFail(const ea::string& reason)
{
    GetSubsystem<VisualTestHarness>()->Error("{} Failed: {}", name_, reason);
}


void VisualTest::CheckComplete()
{
    if (IsState(VisualTestState::Running)) {
        if (!HasCapturesRemaining()) {
            state_ = VisualTestState::Passed;
            Finish();
        }
    }
}


void VisualTest::HandleCaptureFailed(StringHash eventType, VariantMap& eventData)
{
    using namespace VisualTestCaptureFailed;
    auto* capture = static_cast<VisualTestCapture*>(eventData[P_CAPTURE].GetPtr());
    OnCaptureFailed(capture, eventData[P_REASON].GetString());
}


void VisualTest::HandleCaptureReady(StringHash eventType, VariantMap& eventData)
{
    using namespace VisualTestCaptureImageReady;
    auto* capture = static_cast<VisualTestCapture*>(eventData[P_CAPTURE].GetPtr());
    OnCaptureImageReady(capture);
}


void VisualTest::HandleSceneUpdate(StringHash eventType, VariantMap& eventData)
{
    using namespace SceneUpdate;
    OnSceneUpdate(eventData[P_TIMESTEP].GetFloat());
}


void VisualTest::HandleFrameEnd(StringHash eventType, VariantMap& eventData)
{
    if (IsComplete()) {
        return;
    }
    CheckComplete();
}


void RegisterVisualTestLibrary(Context* context)
{
    VisualTest::RegisterObject(context);
    VisualTestCapture::RegisterObject(context);
}

}
