#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/VirtualFileSystem.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/VisualTest/VisualTestHarness.h>


namespace Urho3D
{


VisualTestHarness::VisualTestHarness(Context* context) :
    Object(context)
{
    context_->RegisterSubsystem(this);

    SetDefaultEngineParameters();
}


bool VisualTestHarness::Initialize()
{
    if (!SetupEngine()) {
        return false;
    }

    if (!SetupPaths()) {
        return false;
    }

    return true;
}

bool VisualTestHarness::SetupEngine()
{
    engine_ = MakeShared<Engine>(context_);

    if (!engine_->Initialize(engineParameters_, {})) {
        Error("Engine initialization failed");
        return false;
    }

    Input* input = GetSubsystem<Input>();
    input->SetMouseMode(MM_ABSOLUTE);
    input->SetMouseVisible(true);
    input->SetEnabled(false);

    return true;
}


bool VisualTestHarness::SetupPaths()
{
    FileSystem* fs = GetSubsystem<FileSystem>();
    if (outputRoot_.empty()) {
        outputRoot_ = fs->GetCurrentDir() + "/Output";
    }
    if (goldenRoot_.empty()) {
        goldenRoot_ = fs->GetCurrentDir() + "/Golden";
    }

    if (!fs->DirExists(outputRoot_)) {
        if (!fs->CreateDirsRecursive(outputRoot_)) {
            Error("Couldn't create output directory {}", outputRoot_);
            return false;
        }
    }

    // If the golden directory doesn't exist then emit an error but continue with
    // the tests as this could be a valid case of just trying to generate test output
    // but generally is unexpected in most circumstances
    if (!fs->DirExists(goldenRoot_)) {
        Error("Specified golden root ({}) does not exist", goldenRoot_);
    }

    VirtualFileSystem* vfs = GetSubsystem<VirtualFileSystem>();
    vfs->MountDir(goldenRoot_);
    return true;
}


void VisualTestHarness::SetEngineTimeStep(float step)
{
    timeStep_ = Max(0.0001f, step);
}


void VisualTestHarness::SetEngineFPS(int fps)
{
    SetEngineTimeStep(1.0f / float(fps));
}


void VisualTestHarness::AddTest(VisualTest* test)
{
    allTests_.emplace_back(test);
}


VisualTest* VisualTestHarness::CreateTest()
{
    return CreateTest<VisualTest>();
}


void VisualTestHarness::RunAllTests()
{
    for (int i = 0; i < allTests_.size(); ++i) {
        RunOneTest(allTests_[i]);
    }
}


void VisualTestHarness::RunOneTest(VisualTest* test)
{
    // Ensure test has name assigned
    if (test->GetName().empty()) {
        Error("Can't run unnamed test");
        return;
    }

    // Ensure test is in corect state
    if (!test->IsState(VisualTestState::NotStarted)) {
        Error("Test ({}) in invalid state, skipping");
        return;
    }

    // Start test and step engine until completion
    URHO3D_LOGINFOF("Starting %s", test->GetName().c_str());
    test->Start();
    while (!test->IsComplete()) {
        StepEngine();
    }
    URHO3D_LOGINFOF("Finished %s", test->GetName().c_str());
}


void VisualTestHarness::SetDefaultEngineParameters()
{
    // Modify engine startup parameters
    engineParameters_[EP_WINDOW_TITLE] = "VisualTestHarness";
    engineParameters_[EP_APPLICATION_NAME] = "Visual Tests";
    engineParameters_[EP_LOG_NAME] = "conf://VisualTests.log";
    engineParameters_[EP_BORDERLESS] = false;
    engineParameters_[EP_HEADLESS] = false;
    engineParameters_[EP_SOUND] = false;
    engineParameters_[EP_RESOURCE_PATHS] = "CoreData;Data;VisualTestData";
    engineParameters_[EP_ORIENTATIONS] = "LandscapeLeft LandscapeRight Portrait";
    engineParameters_[EP_WINDOW_RESIZABLE] = true;
    engineParameters_[EP_WINDOW_MAXIMIZE] = false;
    engineParameters_[EP_WINDOW_WIDTH] = 10;
    engineParameters_[EP_WINDOW_HEIGHT] = 10;
    if (!engineParameters_.contains(EP_RESOURCE_PREFIX_PATHS))
    {
        if (GetPlatform() == PlatformId::MacOS ||
            GetPlatform() == PlatformId::iOS)
            engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../Resources;../..";
        else
            engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";..;../..";
    }
}


void VisualTestHarness::StepEngine()
{
    engine_->SetNextTimeStep(timeStep_);
    engine_->RunFrame();
}


const ea::string& VisualTestHarness::GetOutputRoot() const
{
    return outputRoot_;
}


void VisualTestHarness::SetOutputRoot(const ea::string& path)
{
    outputRoot_ = path;
}


const ea::string& VisualTestHarness::GetGoldenRoot() const
{
    return goldenRoot_;
}


void VisualTestHarness::SetGoldenRoot(const ea::string& path)
{
    goldenRoot_ = path;
}


ea::string VisualTestHarness::GetOutputPath(VisualTest* test) const
{
    return Format("{}/{}", outputRoot_, test->GetName());
}


ea::string VisualTestHarness::GetOutputPath(VisualTest* test, const ea::string& subpath) const
{
    return Format("{}/{}/{}", outputRoot_, test->GetName(), subpath);
}


ea::string VisualTestHarness::GetGoldenPath(VisualTest* test, const ea::string& subpath) const
{
    return Format("{}/{}", test->GetName(), subpath);
}


void VisualTestHarness::OnError(const ea::string& message)
{
    URHO3D_LOGERROR(message.c_str());
}


}

