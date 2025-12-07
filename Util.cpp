#include "Util.h";

#define _CRT_SECURE_NO_WARNINGS
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Autor: Nedeljko Tesanovic
// Opis: pomocne funkcije za ucitavanje sejdera i tekstura
unsigned int compileShader(GLenum type, const char* source)
{
    //Uzima kod u fajlu na putanji "source", kompajlira ga i vraca sejder tipa "type"
    //Citanje izvornog koda iz fajla
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
    std::string temp = ss.str();
    const char* sourceCode = temp.c_str(); //Izvorni kod sejdera koji citamo iz fajla na putanji "source"

    int shader = glCreateShader(type); //Napravimo prazan sejder odredjenog tipa (vertex ili fragment)

    int success; //Da li je kompajliranje bilo uspjesno (1 - da)
    char infoLog[512]; //Poruka o gresci (Objasnjava sta je puklo unutar sejdera)
    glShaderSource(shader, 1, &sourceCode, NULL); //Postavi izvorni kod sejdera
    glCompileShader(shader); //Kompajliraj sejder

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //Provjeri da li je sejder uspjesno kompajliran
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog); //Pribavi poruku o gresci
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    //Pravi objedinjeni sejder program koji se sastoji od Vertex sejdera ciji je kod na putanji vsSource

    unsigned int program; //Objedinjeni sejder
    unsigned int vertexShader; //Verteks sejder (za prostorne podatke)
    unsigned int fragmentShader; //Fragment sejder (za boje, teksture itd)

    program = glCreateProgram(); //Napravi prazan objedinjeni sejder program

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource); //Napravi i kompajliraj vertex sejder
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource); //Napravi i kompajliraj fragment sejder

    //Zakaci verteks i fragment sejdere za objedinjeni program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //Povezi ih u jedan objedinjeni sejder program
    glValidateProgram(program); //Izvrsi provjeru novopecenog programa

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success); //Slicno kao za sejdere
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    //Posto su kodovi sejdera u objedinjenom sejderu, oni pojedinacni programi nam ne trebaju, pa ih brisemo zarad ustede na memoriji
    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}

unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

GLFWcursor* loadImageToCursor(const char* filePath, float scale) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;

    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 4);

    if (ImageData != NULL)
    {
        float diagonal = sqrtf(TextureWidth * TextureWidth + TextureHeight * TextureHeight);
        int canvasSize = static_cast<int>(diagonal * scale);

        if (canvasSize < 1) canvasSize = 1;

        unsigned char* rotatedData = new unsigned char[canvasSize * canvasSize * 4];
        memset(rotatedData, 0, canvasSize * canvasSize * 4);

        // Center points
        float origCenterX = TextureWidth / 2.0f;
        float origCenterY = TextureHeight / 2.0f;
        float canvasCenter = canvasSize / (2.0f * scale);


        float angle = M_PI / 4.0f;
        float cosA = cosf(angle);
        float sinA = sinf(angle);

        for (int y = 0; y < canvasSize; y++) {
            for (int x = 0; x < canvasSize; x++) {
                float normX = (x / scale) - canvasCenter;
                float normY = (y / scale) - canvasCenter;

                float srcX = normX * cosA + normY * sinA + origCenterX;
                float srcY = -normX * sinA + normY * cosA + origCenterY;

                int srcXi = static_cast<int>(srcX + 0.5f);
                int srcYi = static_cast<int>(srcY + 0.5f);

                if (srcXi >= 0 && srcXi < TextureWidth && srcYi >= 0 && srcYi < TextureHeight) {
                    int srcIndex = (srcYi * TextureWidth + srcXi) * 4;
                    int dstIndex = (y * canvasSize + x) * 4;

                    rotatedData[dstIndex] = ImageData[srcIndex];
                    rotatedData[dstIndex + 1] = ImageData[srcIndex + 1];
                    rotatedData[dstIndex + 2] = ImageData[srcIndex + 2]; 
                    rotatedData[dstIndex + 3] = ImageData[srcIndex + 3];
                }
            }
        }

        GLFWimage image;
        image.width = canvasSize;
        image.height = canvasSize;
        image.pixels = rotatedData;

        float hotX = TextureWidth / 5.0f;
        float hotY = TextureHeight / 6.0f;

        float relX = hotX - origCenterX;
        float relY = hotY - origCenterY;

        float rotatedHotX = relX * cosA - relY * sinA;
        float rotatedHotY = relX * sinA + relY * cosA;

        int hotspotX = static_cast<int>((rotatedHotX + canvasCenter) * scale + 0.5f);
        int hotspotY = static_cast<int>((rotatedHotY + canvasCenter) * scale + 0.5f);

        // Clamp to canvas bounds
        if (hotspotX < 0) hotspotX = 0;
        if (hotspotX >= canvasSize) hotspotX = canvasSize - 1;
        if (hotspotY < 0) hotspotY = 0;
        if (hotspotY >= canvasSize) hotspotY = canvasSize - 1;

        GLFWcursor* cursor = glfwCreateCursor(&image, hotspotX, hotspotY);

        delete[] rotatedData;
        stbi_image_free(ImageData);

        return cursor;
    }
    else {
        std::cout << "Kursor nije ucitan! Putanja kursora: " << filePath << std::endl;
        return nullptr;
    }
}