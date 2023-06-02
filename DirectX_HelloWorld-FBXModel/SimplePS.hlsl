struct VSOutput
{
	float4 svpos : SV_POSITION;	//���_�V�F�[�_�[���痈�����W
	float4 color : COLOR;		//���_�V�F�[�_�[���痈���F
	float2 uv : TEXCOORD;		//���_�V�F�[�_���痈��UV
};

float4 pixel(VSOutput input) : SV_TARGET
{
	//return input.color;	//�F�����̂܂ܕ\������
	return float4(input.uv.xy, 1, 1);
}