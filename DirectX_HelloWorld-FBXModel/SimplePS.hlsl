struct VSOutput
{
	float4 svpos : SV_POSITION;	//頂点シェーダーから来た座標
	float4 color : COLOR;		//頂点シェーダーから来た色
	float2 uv : TEXCOORD;		//頂点シェーダから来たUV
};

float4 pixel(VSOutput input) : SV_TARGET
{
	//return input.color;	//色をそのまま表示する
	return float4(input.uv.xy, 1, 1);
}