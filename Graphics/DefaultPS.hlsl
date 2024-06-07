#include <Header.hlsli>

const static float PI = 3.141592f;

float3 Fresnel(float HdotV, float3 F0)
{
    float viewSeta = pow((1 - HdotV), 5);
    float3 reflect = (1 - F0) * viewSeta;
    
    return F0 + reflect;
}

float NDF(float vNdotH, float roughness)
{
    float result = pow(roughness + 0.01f, 2);
    float rough = pow((((result - 1) * pow(vNdotH, 2)) + 1), 2) * PI;
    
    return result / rough;
}

float GAF(float NdotL, float NdotV, float NdotH, float VdotH)
{
    return min(1, min(abs(2 * NdotH * NdotV / VdotH), abs(2 * NdotH * NdotL / VdotH)));
}

float4 main(PS_Input input) : SV_Target
{
    //float4 opacity = txOpacityMap.Sample(LinearSampler, input.UVW.xy);
    //clip((1 - (opacity.a - opacity.r)) - 0.355f);
    
    
    float4 FinalColor = { 1, 1, 1, 1 };
    float3 PBRs = 0, PBRd = 0;
    
    
    float4 baseColor = txDiffuseMap.Sample(LinearSampler, input.UVW.xy);
    //baseColor += ObjectBaseColor;
    baseColor.rgb = baseColor.rgb * baseColor.rgb;
    float3 vNormalMap = txNormalMap.Sample(LinearSampler, input.UVW.xy) * 2.0f - 1.0f;

    
    // get normal
    float3 normal;// = input.Normal;
    float3x3 vTangentSpace = float3x3(input.Tangent, input.BiTangent, input.Normal);
    normal = normalize(mul(vNormalMap, vTangentSpace));
    // Gamma Collect
    

    
    // half, view, light, basecolor;
    float3 view = CameraPosition - input.WorldPos.xyz;
    view = normalize(view);
    float3 halfVector = normalize((-WorldDirectionLight.xyz) + view.xyz);
    

    float NdotL = max(0, dot(normal, -WorldDirectionLight.xyz));
    float NdotH = (dot(normal, halfVector));
    float NdotV = (dot(normal, view));
    float HdotV = (dot(halfVector, view));
    
    
    float vMetal = txMetalness.Sample(LinearSampler, input.UVW.xy).x;
    vMetal += (1.0f - vMetal) * Metallic;
    float vRough = txShininessMap.Sample(LinearSampler, input.UVW.xy).x;
    vRough += (1.0f - vRough) * Loughness;
    float3 F0 = ((0.04f * 1) * (1 - vMetal)) + (baseColor.rgb * vMetal);
    
    float3 F = Fresnel(HdotV, F0);
    float D = NDF(NdotH, vRough * vRough);
    float G = GAF(NdotL, NdotV, NdotH, HdotV);
 

    
    
    PBRs = (F * D * G) / (4 * NdotL * NdotV);
    
    
    float3 kd = lerp(1.0 - F, 0.04, vMetal);
    
    PBRd = kd * baseColor.rgb;

    float3 PBR = ((PBRs + PBRd) * NdotL * 1) * WorldDirectionPower;
    PBR = saturate(PBR);
    
    //float currentShadowDepth = input.PosShadow.z / input.PosShadow.w;
    //float2 uv = input.PosShadow.xy / input.PosShadow.w;
    
    //uv.y = -uv.y;
    //uv = (uv * 0.5f) + 0.5f;
    
    //PBR = saturate(PBR);
    
    //if (uv.x >= 0.0f && uv.x <= 1.0f && uv.y >= 0.0f && uv.y <= 1.0f)
    //{
    //    float sampleShadowDepth = txShadowMap.Sample(LinearSampler, uv).r;
    //    if (currentShadowDepth > sampleShadowDepth + 0.001)
    //    {
    //        PBR *= 0.25f;
    //    }

    //}
    
    FinalColor.rgb = PBR;
    
    //FinalColor.rgb = (FinalColor.rgb * (1 - txSpecularMap.Sample(LinearSampler, input.UVW.xy).r));
    
    FinalColor.rgb = sqrt(FinalColor.rgb);
    FinalColor.a = baseColor.a;
    
    FinalColor.rgb += txEmissiveMap.Sample(LinearSampler, input.UVW.xy).rgb;
    
    FinalColor.rgb += ((ObjectColor.rgb - FinalColor.rgb) * ObjectColorPower);
    
    FinalColor.rgb *= CameraColor.rgb;
    
    return FinalColor;

}