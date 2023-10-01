#pragma once

#include <Urho3D/VisualTest/VisualTestDefs.h>

#include <Urho3D/Core/Object.h>
#include <Urho3D/Resource/Image.h>

namespace Urho3D
{

/// Describes options of an image comparison operation.
struct VISUALTEST_API VisualTestImageCompareOptions
{
    /// Compute SSIM value
    bool enableSSIM_ = true;
    /// Generate differnce image, difference image will highlight portions where
    /// the reference image did not match the generated image
    bool enableDifferenceImage_ = false;
    /// Tolerance for per-pixel matches, "0.0" indicates pixels much be exactly
    /// identical to be considered matching. Values greater than "0" allow
    /// matching pixels to be within the 
    float pixelMatchTolerance_ = 0.0f;
    /// The block size to use for SSIM, if SSIM is enabled
    int ssimBlockSize_ = 8;
};



/// Describes the result of an image comparison operation. The comparison
/// of some generated image 
struct VISUALTEST_API VisualTestImageCompareResult
{
    /// Percentage of pixel matches
    float pixelMatchPercent_ = 0.0f;

    /// SSIM score for the comparison
    float ssim_ = 0.0f;

    /// Generated difference image (if enabled)
    SharedPtr<Image> differenceImage_;
};


VisualTestImageCompareResult CompareImages(const VisualTestImageCompareOptions& options, Image* a, Image* b);


};
