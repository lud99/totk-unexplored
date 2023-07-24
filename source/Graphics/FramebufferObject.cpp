#include "FramebufferObject.h"

#include <glad/glad.h>

#include <iostream>

FramebufferObject::FramebufferObject()
{
    
}
	
bool FramebufferObject::Create(int width, int height)
{
    glGenFramebuffers(1, &m_Fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);   

    // Create texture
    glGenTextures(1, &m_TextureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, m_TextureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Attach it to the FramebufferObject object
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureColorbuffer, 0);     

    // Create render buffer
    glGenRenderbuffers(1, &m_Rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_Rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Attach rbo to fbo
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_Rbo);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FramebufferObject:: FramebufferObject is not complete!\n";
        return false;
    }
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  

    return true;
}

void FramebufferObject::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_Fbo);   
}
void FramebufferObject::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);   
}

void FramebufferObject::Delete()
{
    glDeleteFramebuffers(1, &m_Fbo);
    glDeleteTextures(1, &m_TextureColorbuffer);
    glDeleteRenderbuffers(1, &m_Rbo);
}

~FramebufferObject::FramebufferObject()
{

}