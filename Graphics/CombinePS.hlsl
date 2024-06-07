#include <Header.hlsli>

float4 main(Sprite_PS_Input input) : SV_TARGET
{
    float4 baseColor = txCombine.Sample(LinearSampler, input.UVW.xy);
	
	
    return baseColor;
}