//
// Copyright (c) 2017-2023 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once

#include <Urho3D/Resource/Image.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/VisualTest/VisualTestDefs.h>

namespace Urho3D
{

/// Forward declarations
class VisualTestCapture;
class VisualTestHarness;

/// Visual test 
class VISUALTEST_API VisualTest : public Object
{
    URHO3D_OBJECT(VisualTest, Object);

public:

    /// Type for loading scene for a test
    using SceneLoader = ea::function< SharedPtr<Scene> (VisualTest*) >;

    /// Create a new test
    VisualTest(Context* context);

    /// Register system metadata
    static void RegisterObject(Context* context);

    /// Get/Set test name
    const ea::string& GetName() const { return name_; }
    void SetName(const ea::string& name) { name_ = name; }

    /// Test state queries
    VisualTestState GetState() const { return state_; }
    bool IsState(VisualTestState state) const { return state_ == state; }
    bool IsFailed() const { return IsState(VisualTestState::Failed); }
    bool IsPassed() const { return IsState(VisualTestState::Passed); }
    bool IsComplete() const { return IsFailed() || IsPassed(); }

    /// Check if all captures in the given scene are complete
    bool HasCapturesRemaining() const;

    /// Get/set variable values for the test
    const Variant& GetVariable(const ea::string& var) const;
    void SetVariable(const ea::string& var, const Variant& value);
    StringVariantMap& GetVariables() { return variables_; }
    const StringVariantMap& GetVariables() const { return variables_; }

    /// Convenience methods to get/set boolean switch variables for the
    /// test
    bool GetSwitch(const ea::string& name) const { return GetVariable(name).GetBool(); }
    void SetSwitch(const ea::string& name, bool on = true) { SetVariable(name, Variant(on)); }

    /// Start the test, will initialize the scene, perform startup
    /// handling and set the test to the Running state.
    void Start();

    /// Get the output path for the test data
    ea::string GetOutputPath(const ea::string& subpath = ea::string());

    /// Get the golden path for the test data
    ea::string GetGoldenPath(const ea::string& subpath = ea::string());

    /// Called to indicate immediate failure of the test
    /// Emit error message for harness
    template <typename... Args>
    void Fail(const Args&... args)
    {
        FailInternal(Format(args...));
    }

    /// Get the current scene for the test, returns nullptr if no scene has
    /// yet been loaded for the test
    Scene* GetScene() const { return scene_; }

    /// Set the scene loader for the test, when the test is run the loader
    /// will be invoked to create the scene.
    void SetSceneLoader(const SceneLoader& sceneLoader);

protected:

    /// Check for completion of the test, default implementation checks
    /// that test is either failed or all captures are complete
    virtual void CheckComplete();

    /// Called after the scene has been loaded to perform any programmatic scene
    /// processing
    virtual void OnStartup();

    /// Teardown handling
    virtual void OnFinish();

    /// Called on scene update, default implementation does nothing, override
    /// this for custom update handling
    virtual void OnSceneUpdate(float timeStep);

    /// Called when a capture fails for some reason
    virtual void OnCaptureFailed(VisualTestCapture* capture, const ea::string& reason);

    /// Called when a capture is ready and has finished producing its image
    virtual void OnCaptureImageReady(VisualTestCapture* capture);

    /// Called to perform a capture image compare
    virtual void CompareCaptureImage(VisualTestCapture* capture);

    /// Save a capture image to file
    virtual void SaveCaptureImageToFile(VisualTestCapture* capture);

    /// Report test failure
    virtual void OnFail(const ea::string& reason);

    /// Get the golden image for the capture
    SharedPtr<Image> GetGoldenImage(const ea::string& imageFileName);

    /// Whether the test has passed or failed
    VisualTestState state_{ VisualTestState::NotStarted };
    /// The name of the test
    ea::string name_;
    /// The scene loader for the test
    SceneLoader sceneLoader_;
    /// The scene for the test
    SharedPtr<Scene> scene_;
    /// Variables set for the test that may influence custom behaviors
    /// like how the scene is created or test variations
    StringVariantMap variables_;

private:

    /// Ensure output directory for the test exists
    bool EnsureOutputDirectoryExists();

    /// Clean the output directory
    bool CleanOutputDirectory();

    /// Internal failure handling
    void FailInternal(const ea::string& reason);

    /// Do test teardown
    void Finish();

    /// Handle a capture failure
    void HandleCaptureFailed(StringHash eventType, VariantMap& eventData);

    /// Handle a successful capture
    void HandleCaptureReady(StringHash eventType, VariantMap& eventData);

    /// Handle scene update
    void HandleSceneUpdate(StringHash eventType, VariantMap& eventData);

    /// Handle frame end
    void HandleFrameEnd(StringHash eventType, VariantMap& eventData);
};


/// Register all visual testing utility objects
VISUALTEST_API void RegisterVisualTestLibrary(Context* context);

}
