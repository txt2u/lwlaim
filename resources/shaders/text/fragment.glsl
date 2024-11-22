#version 410 core

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main() {
    // Get the alpha from the texture (assuming it’s a grayscale texture where R=G=B=A)
    float alpha = texture(text, TexCoords).r;
	
    // If alpha is 0, discard the fragment (improves performance for transparent parts)
    if (alpha < 0.1) {
        discard;
    }

    FragColor = vec4(textColor, alpha);
}
