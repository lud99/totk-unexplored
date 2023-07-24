#include "QrCodeImage.h"

#include "../qrcodegen.hpp"
#include <iostream>

#include "../Graphics/BasicVertices.h"

#include <glm/gtc/type_precision.hpp>

QrCodeImage::QrCodeImage()
{

}

void QrCodeImage::SetMatrices(glm::mat4* projectionMatrix, glm::mat4* viewMatrix)
{
    m_Image.m_ProjectionMatrix = projectionMatrix;
    m_Image.m_ViewMatrix = viewMatrix;
}

void QrCodeImage::GenerateCode(const std::string& text, float renderSize)
{
    using namespace qrcodegen;

    std::cout<<("GENERATE QR\n");

    //if (m_Texture) 
        //delete m_Texture;

    if (m_TextureData)
        delete[] m_TextureData;

    // Create OpenGL texture
    if (m_TextureId == 0)
    {
        glGenTextures(1, &m_TextureId);
        glBindTexture(GL_TEXTURE_2D, m_TextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixel clear image
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    QrCode qr = QrCode::encodeText(text.c_str(), QrCode::Ecc::MEDIUM);

    int border = 1;
    int textureSize = border * 2 + qr.getSize();

    m_TextureData = new glm::u8vec3[textureSize * textureSize];

    // Create the rgb texture
    for (int x = 0; x < textureSize; x++) {
        for (int y = 0; y < textureSize; y++) {
            
            uint8_t color = 255; // white

            if (x >= border && x < qr.getSize() + border && y >= border && y < qr.getSize() + border)
                color = qr.getModule(x - border, y - border) ? 0 : 255;

            m_TextureData[x + textureSize * y].x = color;
            m_TextureData[x + textureSize * y].y = color;
            m_TextureData[x + textureSize * y].z = color;
        }
    }

    // Upload to GPU
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Pixel clear image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, m_TextureData);
    glBindTexture(GL_TEXTURE_2D, 0);

    glm::vec3 positions[4];
    BasicVertices::Quad::Construct(positions, renderSize, renderSize);

    m_Image.Create("", positions[0], positions[1], positions[2], positions[3]);
    m_Image.m_Mesh.m_Texture->m_Texture = m_TextureId;

    m_Show = true;
}

void QrCodeImage::Render()
{
    if (m_Show)
        m_Image.Render();
}

void QrCodeImage::Delete()
{
    if (m_TextureData)
        delete[] m_TextureData;

    m_TextureData = nullptr;

    if (m_TextureId != 0)
        glDeleteTextures(1, &m_TextureId);
    
    m_TextureId = 0;
}

QrCodeImage::~QrCodeImage()
{
    std::cout << "destroctor\n";
}