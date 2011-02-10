uniform sampler2D tex;

void main()
{
    vec4 newPos;
    vec4 dv;
    float df;
    
	gl_TexCoord[0].xy = gl_MultiTexCoord0.xy;
	dv = texture2D(tex, gl_MultiTexCoord0.xy);
	df = 0.3 * dv.x + 0.59 * dv.y + 0.11 * dv.z;
	// newPos = vec4(gl_Vertex.x, gl_Vertex.y, gl_Vertex.z + (1.0 - df), 1.0);
	newPos = vec4(gl_Normal * (1.0-df), 0.0) + gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * newPos;
}
