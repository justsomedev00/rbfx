#pragma once

#include "../Container/ConstString.h"

#define VISUALTEST_API URHO3D_API

namespace Urho3D
{

// Component category for scene components related to visual testing
URHO3D_GLOBAL_CONSTANT(ConstString Category_VisualTest{ "Component/VisualTest" });

/// Visual test state
enum class VisualTestState
{
    // Test has not yet started
    NotStarted,
    // Test is currently starting up
    Starting,
    // Test is running
    Running,
    // Test has failed
    Failed,
    // Test has passed
    Passed
};

/// Visual test capture state
enum class VisualTestCaptureState
{
    // Capture is waiting
    Pending,
    // Capture is queued for rendering this frame
    Queued,
    // Capture is complete, will produce no further images
    Complete,
    // Capture is failed, will produce no further images
    Failed
};

/// Visual test capture formats
enum class VisualTestCaptureFormat
{
    BMP, PNG, TGA, JPG
};


}
