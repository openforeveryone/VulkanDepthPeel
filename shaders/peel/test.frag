#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index=0, set=2, binding=0) uniform subpassInput subpass;
layout (location = 0) in vec4 color;
layout (location = 0) out vec4 outColor;

void main() {
   vec4 depth = subpassLoad(subpass);
   if (gl_FragCoord.z-0.00001 <= depth.r)
    discard;
   outColor = color;
}
