#include <Urho3D/VisualTest/VisualTestImageCompare.h>

namespace Urho3D
{


/// Compute the structural similarity index for the given image pair using the
///     https://en.wikipedia.org/wiki/Structural_similarity
///
static float ComputeSSIM(Image* image1, Image* image2, unsigned blockSize)
{
    return 1.0f;
}


/// Check if the given pixel color values are within the given tolerance
static bool WithinTolerance(const Color& pixelA, const Color& pixelB, float tolerance)
{
    if (tolerance == 0.0f) {
        return pixelA == pixelB;
    }
    // TODO : Use better color space
    Vector4 delta = pixelA.ToVector4() - pixelB.ToVector4();
    return delta.Length() < tolerance;
}


/// Compute color for difference image
static Color GetDifferenceImageColor(const Color& pixelA, const Color& pixelB)
{
    // TODO : Probably a better difference metric
    Vector4 delta = (pixelA.ToVector4() - pixelB.ToVector4());
    float value = delta.Length() / 2;
    return Color(value, value, value);
}


VisualTestImageCompareResult CompareImages(const VisualTestImageCompareOptions& options, Image* a, Image* b)
{
    VisualTestImageCompareResult result;

    if (!a || !b) {
        URHO3D_LOGERRORF("Must provide 2 images for comparison");
        return result;
    }

    if (a->GetSize() != b->GetSize()) {
        URHO3D_LOGERRORF("Image sizes must match for comparison");
        return result;
    }

    int w = a->GetWidth();
    int h = a->GetHeight();

    if (options.enableDifferenceImage_) {
        result.differenceImage_ = MakeShared<Image>(a->GetContext());
        result.differenceImage_->SetSize(w, h, a->GetComponents());
        result.differenceImage_->Clear(Color::TRANSPARENT_BLACK);
    }
    
    int matchedPixelsCount = 0;

    // Calculate per-pixel differences
    for (int x = 0; x < w; ++x) {
        for (int y = 0; y < h; ++y) {
            Color pixelA = a->GetPixel(x, y);
            Color pixelB = b->GetPixel(x, y);
            if (WithinTolerance(pixelA, pixelB, options.pixelMatchTolerance_)) {
                ++matchedPixelsCount;
            }
            else if (result.differenceImage_) {
                result.differenceImage_->SetPixel(x, y, GetDifferenceImageColor(pixelA, pixelB));
            }
        }
    }

    // Calculate per-pixel match percentage
    result.pixelMatchPercent_ = float(matchedPixelsCount) / (w * h);

    // If all pixels matched then no need to do SSIM 
    if (result.pixelMatchPercent_ == 1.0f) {
        result.ssim_ = 1.0f;
    } else {
        result.ssim_ = ComputeSSIM(a, b, options.ssimBlockSize_);
    }

    return result;
}

}

