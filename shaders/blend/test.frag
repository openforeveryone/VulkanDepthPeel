#version 400
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

//layout (location = 0) in vec4 color;
layout (input_attachment_index=0, set=2, binding=0) uniform subpassInput subpass;
layout (location = 0) out vec4 outColor;

void main() {
   vec4 color = subpassLoad(subpass);
   outColor = vec4(color.r*color.a, color.g*color.a, color.b*color.a, color.a);
//   outColor = color;
}
