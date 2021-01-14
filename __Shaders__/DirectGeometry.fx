struct VSIn
{
    uint vertexId : SV_VertexID;
};

struct VSOut
{
    float4 pos : SV_Position;
    float4 color : color;
};

VSOut VSmain(VSIn input)
{
    VSOut output;
	
    if (input.vertexId == 0)
        output.pos = float4(0.0, 0.5, 0.5, 0.f);
    else if (input.vertexId == 2)
        output.pos = float4(0.5, -0.5, 0.5, 0.f);
    else if (input.vertexId == 1)
        output.pos = float4(-0.5, -0.5, 0.5, 0.f);
	
    output.color = float4( 1.0f, 0.0f, 1.0f, 1.f );
	
    return output;
}

struct PSIn
{
    float4 pos : SV_Position;
    linear float4 color : color;
};

struct PSOut
{
    float4 color : SV_Target;
};


PSOut PSmain(PSIn input)
{
    PSOut output;
	
    output.color =float4( 1.0f, 0.0f, 1.0f, 1.f );
	
    return output;
}

VertexShader vsCompiled = CompileShader( vs_4_0, VSmain() );

technique10 t0
{
    pass p0
    {
        SetVertexShader( vsCompiled );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PSmain() ));
    }  
}

