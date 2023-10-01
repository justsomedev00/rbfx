#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/VisualTest/VisualTest.h>


namespace Urho3D
{

/// Subsystem for controlling the run of a set of visual tests.
/// Provides common services for tests:
///
/// - Locating appropriate golden data sets
/// - Writing generated data to the appropriate output directories.
/// - Performing image comparisons
/// - etc.
/// 
class VISUALTEST_API VisualTestHarness : public Object
{
    URHO3D_OBJECT(VisualTestHarness, Object);

public:

    /// Create visual test harness system for running visual
    /// tests within an engine instance
    VisualTestHarness(Context* context);

    /// Control engine time step to use for the tests
    float GetEngineTimeStep() const { return timeStep_; }
    void SetEngineTimeStep(float step);
    void SetEngineFPS(int fps);

    /// Get engine parameters
    const StringVariantMap& GetEngineParameters() const { return engineParameters_; }
    StringVariantMap& GetEngineParameters() { return engineParameters_; }

    /// Initialize the test harness, will create the engine,
    /// setup the data paths and perform all other one-time
    /// setup
    bool Initialize();

    /// Add a test to the harness
    void AddTest(VisualTest* test);

    /// Create a new empty visual test and add it to the harness
    VisualTest* CreateTest();

    /// Create a new test and add it to the harness
    template <class T> T* CreateTest(); 

    /// Run all tests in the harness
    void RunAllTests();

    /// Run a single test
    void RunOneTest(VisualTest* test);

    /// Get/set explicit output path root
    const ea::string& GetOutputRoot() const;
    void SetOutputRoot(const ea::string& path);

    /// Get/set path root for golden data
    const ea::string& GetGoldenRoot() const;
    void SetGoldenRoot(const ea::string& path);

    /// Get the output directory for the given test
    ea::string GetOutputPath(VisualTest* test) const;
    ea::string GetOutputPath(VisualTest* test, const ea::string& subpath) const;

    /// Get the golden data path for the given test
    ea::string GetGoldenPath(VisualTest* test, const ea::string& subpath) const;

    /// Emit error message for harness
    template <class... Args>
    void Error(const Args&... args)
    {
        OnError(Format(args...));
    }

protected:

    /// Handle error
    virtual void OnError(const ea::string& message);

private:

    /// Perform engine setup
    bool SetupEngine();

    /// Setup data paths
    bool SetupPaths();

    /// Step the engine one frame
    void StepEngine();

    /// Set default engine parameters
    void SetDefaultEngineParameters();

    /// Time step (seconds) for the engine, tests are run at a fixed
    /// time step for consistent results (default 60 fps)
    float timeStep_{ 1.0f / 60.0f };
    /// Current platform for the test
    ea::string platform_;
    /// Root for output data
    ea::string outputRoot_;
    /// Root for goden data
    ea::string goldenRoot_;
    /// The engine running the tests
    SharedPtr<Engine> engine_;
    /// The engine parameters for the harness
    StringVariantMap engineParameters_;
    /// All tests that have been added to the harness
    ea::vector< SharedPtr<VisualTest> > allTests_;
};

template <class T>
T* VisualTestHarness::CreateTest()
{
    T* test = new T(GetContext());
    AddTest(test);
    return test;
}


}
