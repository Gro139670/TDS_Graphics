#include <Header.hlsli>



PS_Input main(Static_VS_Input input)
{
    PS_Input output = (PS_Input) 0;
	
    float4 pos = input.Pos;
    pos.w = 1;
    
    output.Pos = mul(pos, WorldTransform);
    output.WorldPos = output.Pos;
    // ø˘µÂ -> ∫‰ ¿¸»Ø
    output.Pos = mul(output.Pos, View);
    // ∫‰ -> ≈ıøµ ¿¸»Ø
    output.Pos = mul(output.Pos, Projection);
    
    //output.PosShadow = mul(output.WorldPos, View);
    //output.PosShadow = mul(output.PosShadow, Projection);
    
    output.Normal = normalize(mul(input.Normal, (float3x3) WorldTransform));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) WorldTransform));
    output.BiTangent = cross(output.Normal, output.Tangent);
    
    //output.Normal = ComputeTangent(normal, input.Pos.xyz, input.UVW.xy);
    
    output.UVW = input.UVW;
    return output;
}

//float3 ComputeTangent(float3 normal, float3 position, float2 texCoord)
//{
//    // Compute tangent and bitangent
//    float3x3 TBNMatrix;
//    float3 dp1 = ddx(position);
//    float3 dp2 = ddy(position);
//    float2 duv1 = ddx(texCoord);
//    float2 duv2 = ddy(texCoord);

//    float det = duv1.x * duv2.y - duv1.y * duv2.x;
//    float invDet = 1.0f / max(det, 1e-6);

//    TBNMatrix[0] = (duv2.y * dp1 - duv1.y * dp2) * invDet;
//    TBNMatrix[1] = (-duv2.x * dp1 + duv1.x * dp2) * invDet;
//    TBNMatrix[2] = normal;

//    return mul(TBNMatrix, normal);
//}

//// ≈∫¡®∆Æ, ≥Î∏÷, πŸ¿Ã≈∫¡®∆Æ¿« ø‹¿˚ ∞ËªÍ
    //output.TangentSpaceNormal = mul(input.Normal, (float3x3) input.Tangent, (float3x3) bitangent);
    //output.TangentSpaceViewDir = mul((float3) float3(0, 0, 1), (float3x3) input.Tangent, (float3x3) bitangent);
    
    //output.TangentSpaceBinormal = bitangent;
    //// ≈∫¡®∆Æ Ω∫∆‰¿ÃΩ∫¿« ≥Î∏÷∞˙ ∫‰ µ∑∫º« ∞ËªÍ
    //output.TangentSpaceNormal = mul(normal, (float3x3) mul(tangent, bitangent));
    //output.TangentSpaceViewDir = mul(float3(0, 0, 1), float3x3(tangent, bitangent, output.TangentSpaceNormal));