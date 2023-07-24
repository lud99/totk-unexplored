#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_precision.hpp>

#include "../Graphics/Texture2D.h"
#include "../Graphics/Quad.h"

class Texture2D;

class QrCodeImage 
{
public:
    QrCodeImage();
    
    void SetMatrices(glm::mat4* projectionMatrix, glm::mat4* viewMatrix = nullptr);

    void GenerateCode(const std::string& text, float renderSize);

    void Render();

    void Delete();

    ~QrCodeImage();

public:
    bool m_Show = false;

private:
    unsigned int m_TextureId = 0;
    glm::u8vec3* m_TextureData = nullptr;  

    TexturedQuad m_Image;
};