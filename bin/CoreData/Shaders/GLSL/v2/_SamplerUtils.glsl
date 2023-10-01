#ifndef _SAMPLER_UTILS_GLSL_
#define _SAMPLER_UTILS_GLSL_

#ifndef _CONFIG_GLSL_
    #error Include _Config.glsl before _SamplerUtils.glsl
#endif


#ifdef URHO3D_PIXEL_SHADER

/// Convert sampled value from sNormal to normal in tangent space.
half3 DecodeNormal(half4 normalInput)
{
    #ifdef PACKEDNORMAL
        half3 normal;
        normal.xy = normalInput.ag * 2.0 - 1.0;
        normal.z = sqrt(max(1.0 - dot(normal.xy, normal.xy), 0.0));
        return normal;
    #else
        return normalize(normalInput.rgb * 2.0 - 1.0);
    #endif
}

/// Convert sampled depth buffer value to linear depth in [0, 1] range.
/// For orthographic cameras, 0 is near plane.
/// For perspective cameras, 0 is focus point and is never actually returned.
float ReconstructDepth(float hwDepth)
{
#ifdef URHO3D_XR
    vec4 depthReconstruct = cDepthReconstruct[vInstID & 1];
#else
    vec4 depthReconstruct = cDepthReconstruct;
#endif
    float fwDepth = HwDepthToForwardDepth(hwDepth);
    // May be undefined for orthographic projection
    float linearDepth = depthReconstruct.y / (fwDepth - depthReconstruct.x);
    return depthReconstruct.z != 0.0 ? fwDepth : linearDepth;
}

#endif // URHO3D_PIXEL_SHADER
#endif // _SAMPLER_UTILS_GLSL_
