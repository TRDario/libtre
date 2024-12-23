#version 450

const int  GLYPHS_PER_LINE = 16;
const int  GLYPH_LINES     = 16;
const int  GLYPH_SIZE      = 8;
const vec2 TEXTURE_SIZE    = vec2(GLYPH_SIZE * GLYPHS_PER_LINE, GLYPH_SIZE* GLYPH_LINES);

layout(std430, binding = 0) buffer b_glyphs
{
	int glyphs[];
};

layout(location = 0) uniform vec2 u_bounds;
layout(location = 1) uniform float u_scale;

layout(location = 0) in vec2 v_offset;

layout(location = 0) out vec2 vf_uv;
layout(location = 1) out vec4 vf_textColor;
layout(location = 2) out vec4 vf_backgroundColor;

void main()
{
	const int packedGlyphAttrs = glyphs[gl_InstanceID * 3];
	const int packedTextColor  = glyphs[gl_InstanceID * 3 + 1];
	const int packedBGColor    = glyphs[gl_InstanceID * 3 + 2];

	const vec2 glyphPos          = vec2(packedGlyphAttrs & 0xFF, packedGlyphAttrs >> 8 & 0xFF);
	const bool glyphRightAligned = bool(packedGlyphAttrs >> 16 & 0xFF);
	const int  glyphChar         = packedGlyphAttrs >> 24 & 0xFF;
	const vec4 glyphTextColor    = vec4((packedTextColor & 0xFF) / 255.0, (packedTextColor >> 8 & 0xFF) / 255.0,
										(packedTextColor >> 16 & 0xFF) / 255.0, (packedTextColor >> 24 & 0xFF) / 255.0);
	const vec4 glyphBGColor      = vec4((packedBGColor & 0xFF) / 255.0, (packedBGColor >> 8 & 0xFF) / 255.0,
										(packedBGColor >> 16 & 0xFF) / 255.0, (packedBGColor >> 24 & 0xFF) / 255.0);

	vf_uv = (vec2(glyphChar % GLYPHS_PER_LINE, glyphChar / GLYPHS_PER_LINE) + v_offset) * GLYPH_SIZE / TEXTURE_SIZE;
	vf_textColor       = glyphTextColor;
	vf_backgroundColor = glyphBGColor;

	vec2 finalPos;
	if (glyphRightAligned) {
		finalPos   = glyphPos * GLYPH_SIZE * u_scale;
		finalPos.x = u_bounds.x - finalPos.x;
		finalPos += v_offset * GLYPH_SIZE * u_scale;
	}
	else {
		finalPos = (v_offset + glyphPos) * GLYPH_SIZE * u_scale;
	}
	gl_Position = vec4(finalPos.x / (u_bounds.x / 2) - 1.0, -finalPos.y / (u_bounds.y / 2) + 1.0, 0, 1);
}