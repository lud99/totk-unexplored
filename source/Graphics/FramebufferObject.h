#pragma once

// Written with information from https://learnopengl.com/Advanced-OpenGL/Framebuffers
class FramebufferObject
{
public:
    FramebufferObject();
	
    bool Create(int width, int height);

	void Bind();
	void Unbind();

    void Delete();

public:
    unsigned int m_Fbo; // Frame buffer object
    unsigned int m_TextureColorbuffer; // Renders to this texture
    unsigned int m_Rbo; // Render buffer object
};