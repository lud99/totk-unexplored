#pragma once

class FramebufferObject
{
public:
    FramebufferObject();
	
    bool Create(int width, int height);

	void Bind();
	void Unbind();

    void Delete();

	~FramebufferObject();

public:
    unsigned int m_Fbo; // Frame buffer object
    unsigned int m_TextureColorbuffer; // Renders to this texture
    unsigned int m_Rbo; // Render buffer object
};