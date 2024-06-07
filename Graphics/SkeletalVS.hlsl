 #include <Header.hlsli>

PS_Input main(Skeletal_VS_Input input)
{
    PS_Input output = (PS_Input) 0;
    float4 pos = input.Pos;
    pos.w = 1;
    
    float weight[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    weight[0] = input.Weights.x;
    weight[1] = input.Weights.y;
    weight[2] = input.Weights.z;
    weight[3] = 1.0f - (weight[0] + weight[1] + weight[2]);
    
    float4x4 matWolrdBlend;
    
    matWolrdBlend = weight[0] * BoneTransform[input.BoneIndices[0]];
    matWolrdBlend += weight[1] * BoneTransform[input.BoneIndices[1]];
    matWolrdBlend += weight[2] * BoneTransform[input.BoneIndices[2]];
    matWolrdBlend += weight[3] * BoneTransform[input.BoneIndices[3]];
    
    output.Pos = mul(pos, matWolrdBlend);
    output.Pos = mul(output.Pos, WorldTransform);
    output.WorldPos = output.Pos;
    // 월드 -> 뷰 전환
    output.Pos = mul(output.Pos, View);
    // 뷰 -> 투영 전환
    output.Pos = mul(output.Pos, Projection);
    
    //output.PosShadow = mul(output.WorldPos, View);
    //output.PosShadow = mul(output.WorldPos, Projection);
    
    float3x3 world = (float3x3) mul(WorldTransform, matWolrdBlend);
    
    output.Normal = normalize(mul(input.Normal, (float3x3) world));
    output.Tangent = normalize(mul(input.Tangent, (float3x3) world));
    output.BiTangent = cross(output.Normal, output.Tangent);
    
    
    output.UVW = input.UVW;
    return output;
}