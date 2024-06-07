#include <Header.hlsli>

Sprite_PS_Input main(Combine_VS_INPUT input )
{
    Sprite_PS_Input output = (Sprite_PS_Input) 0;
    output.Pos = input.Pos;
    output.UVW = input.UVW;
    
    return output;
}