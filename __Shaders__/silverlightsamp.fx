/// <summary>The tint color.</summary>
/// <type>Color</type>
/// <defaultValue>0.9,0.7,0.3,1</defaultValue>
float4 TintColor : register(C0);

sampler2D Input : register(s0);

float4 main(float2 uv : TEXCOORD) : COLOR 
{
   // Sample the original color at the coordinate
   float4 color = tex2D(Input, uv);
    
   // Convert the color to gray
   float gray = dot(color.rgb, float3(0.2126, 0.7152, 0.0722)); 
    
   // Create the gray color with the original alpha value
   float4 grayColor = float4(gray, gray, gray, color.a); 
   
   // Return the tinted pixel
   return grayColor * TintColor;
}
