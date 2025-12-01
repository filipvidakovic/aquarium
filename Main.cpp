#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define _USE_MATH_DEFINES
#include <cmath> // za pi
#include <algorithm> // za max()
#include <iostream>
#include "Util.h"


#define NUM_SLICES 40

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}
//Gold fish
float uX = 0.0f;
float uY = -0.5f;
float uS = 0.5f;

float uFlipX = -1.0f;
bool ufacingRight = false; // Track current facing direction

//Clown fish
float cX = 0.0f;
float cY = -0.5f;
float cS = 0.5f;
float cFlipX = 1.0f;
bool cfacingRight = true;

//Bubbles
float bX = 0.0f;
float bY = 0.0f;


// Dimenzije ekrana su izdvojene kao globalne promenljive zbog zadatka 6
int screenWidth = 800;
int screenHeight = 800;
unsigned goldfishTexture;
unsigned clownfishTexture;
unsigned bubblesTexture;

float initialJumpTime = -1.0f;

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath); // Učitavanje teksture
    glBindTexture(GL_TEXTURE_2D, texture); // Vezujemo se za teksturu kako bismo je podesili

    // Generisanje mipmapa - predefinisani različiti formati za lakše skaliranje po potrebi (npr. da postoji 32 x 32 verzija slike, ali i 16 x 16, 256 x 256...)
    glGenerateMipmap(GL_TEXTURE_2D);

    // Podešavanje strategija za wrap-ovanje - šta da radi kada se dimenzije teksture i poligona ne poklapaju
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // S - tekseli po x-osi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); // T - tekseli po y-osi

    // Podešavanje algoritma za smanjivanje i povećavanje rezolucije: nearest - bira najbliži piksel, linear - usrednjava okolne piksele
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void formVAOs(float* verticesRect, size_t rectSize, unsigned int& VAOrect) {
    // formiranje VAO-ova je izdvojeno u posebnu funkciju radi čitljivijeg koda u main funkciji

    // Podsetnik za atribute:
/*
    Jedan VAO se vezuje za jedan deo celokupnog skupa verteksa na sceni.
    Na primer, dobra praksa je da se jedan VAO vezuje za jedan VBO koji se vezuje za jedan objekat, odnosno niz temena koja opisuju objekat.

    VAO je pomoćna struktura koja opisuje kako se podaci u nizu objekta interpretiraju.
    U render-petlji, za crtanje određenog objekta, naredbom glBindVertexArray(nekiVAO) se određuje koji se objekat crta.

    Potrebno je definisati koje atribute svako teme u nizu sadrži, npr. pozicija na lokaciji 0 i boja na lokaciji 1.

    Ova konfiguracija je specifična za naš primer na vežbama i može se menjati za različite potrebe u projektu.


    Atribut se opisuje metodom glVertexAttribPointer(). Argumenti su redom:
        index - identifikacioni kod atributa, u verteks šejderu je povezan preko svojstva location (location = 0 u šejderu -> indeks tog atributa treba staviti isto 0 u ovoj naredbi)
        size - broj vrednosti unutar atributa (npr. za poziciju su x i y, odnosno 2 vrednosti; za boju r, g i b, odnosno 3 vrednosti)
        type - tip vrednosti
        normalized - da li je potrebno mapirati na odgovarajući opseg (mi poziciju već inicijalizujemo na opseg (-1, 1), a boju (0, 1), tako da nam nije potrebno)
        stride - koliko elemenata u nizu treba preskočiti da bi se došlo od datog atributa u jednom verteksu do istog tog atributa u sledećem verteksu
        pointer - koliko elemenata u nizu treba preskočiti od početka niza da bi se došlo do prvog pojavljivanja datog atributa
*/
// Četvorougao

    unsigned int VBOrect;
    glGenVertexArrays(1, &VAOrect);
    glGenBuffers(1, &VBOrect);

    glBindVertexArray(VAOrect);
    glBindBuffer(GL_ARRAY_BUFFER, VBOrect);
    glBufferData(GL_ARRAY_BUFFER, rectSize, verticesRect, GL_STATIC_DRAW);

    // Atribut 0 (pozicija):
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Atribut 1 (tekstura):
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void drawRect(unsigned int rectShader, unsigned int VAOrect) {
    glUseProgram(rectShader);

    // Existing jump logic...
    if (initialJumpTime > 0) {
        float t = glfwGetTime() - initialJumpTime;
        float jumpHeight = 0.5f;
        float jumpSpeed = 4.0f;
        float s = sin(t * jumpSpeed) * jumpHeight;
        uY = -0.5f + std::max(s, 0.0f);
        if (uY == -0.5f)
            initialJumpTime = -1.0f;
    }

    // Pass uniforms to shader
    glUniform1f(glGetUniformLocation(rectShader, "uX"), uX);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), uY);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), uS);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), uFlipX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, goldfishTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawRect2(unsigned int rectShader, unsigned int VAOrect) {
    glUseProgram(rectShader);

    // Pass uniforms to shader
    glUniform1f(glGetUniformLocation(rectShader, "uX"), cX);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), cY);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), cS);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), cFlipX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, clownfishTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void drawRectBubbles(unsigned int rectShader, unsigned int VAOrect) {
    glUseProgram(rectShader);

    // Pass uniforms to shader
    glUniform1f(glGetUniformLocation(rectShader, "uX"), cX);
    glUniform1f(glGetUniformLocation(rectShader, "uY"), cY);
    glUniform1f(glGetUniformLocation(rectShader, "uS"), cS);
    glUniform1f(glGetUniformLocation(rectShader, "uFlipX"), cFlipX);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bubblesTexture);

    glBindVertexArray(VAOrect);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void squish_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        // uS se kreće između dva stanja: normalnog i spljeskanog
        if (uS == 1.0f) {
            uS = 0.5f;
        }
        else {
            uS = 1.0f;
        }
    }
    // Pošto je svakako potrebno pamtiti vreme kada je skok započet, možemo je postaviti na negativnu vrednost ako želimo da signaliziramo da četvorougao nije u skoku
    // Proverom da li je kliknut Space ali istovremeno da li skok traje, obezbeđuje se da telo ne može skočiti ukoliko je već u procesu skakanja
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && initialJumpTime < 0) {
        initialJumpTime = glfwGetTime();
    }
}

void center_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // Normalizacija da bi se pozicija izražena u pikselima namapirala na OpenGL-ov prozor sa opsegom (-1, 1)
        float xposNorm = (xpos / screenWidth) * 2 - 1;
        float yposNorm = -((ypos / screenHeight) * 2 - 1);

        if (
            xposNorm < 0.2 + uX && xposNorm > -0.2 + uX && // Da li je kursor između leve i desne ivice kvadrata
            yposNorm < 0.2 * uS + uY && yposNorm > -0.2 * uS + uY // Da li je kursor između donje i gornje ivice kvadrata, sa uračunatim pomeranjem ivica u spljeskanom stanju
            ) uX = 0;
    }
}

int main()
{
    // Inicijalizacija GLFW i postavljanje na verziju 3 sa programabilnim pajplajnom
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Formiranje prozora za prikaz sa datim dimenzijama i naslovom
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Vezba 3", NULL, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);
    // Vezivanje svih callback funkcija sa prozorom nakon što je prozor inicijalizovan
    glfwSetKeyCallback(window, squish_callback);
    glfwSetMouseButtonCallback(window, center_callback);

    // Inicijalizacija GLEW
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    // Potrebno naglasiti da program koristi alfa kanal za providnost
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    preprocessTexture(goldfishTexture, "res/4-gold-fish-png-image.png");
    preprocessTexture(clownfishTexture, "res/clown-fish.png");
    preprocessTexture(bubblesTexture, "res/bubbles.png");
    //glUseProgram(rectShader);

    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    // Na lokaciju 0 od mogućih 16 lokacija za teksture postavljamo uTex uniformu
    // Pomoću glActiveTexture() u render-petlji biramo na koju lokaciju (odnosno na koji sloj) crtamo teksturu koju bind-ujemo
    glUseProgram(rectShader);
    glUniform1i(glGetUniformLocation(rectShader, "uTex"), 0);

    unsigned int colorShader = createShader("color.vert", "color.frag");

    // Tri temena čine jedan trougao
    float verticesRect[] = {
         -0.2f, 0.2f, 0.0f, 1.0f, // gornje levo teme
         -0.2f, -0.2f, 0.0f, 0.0f,// donje levo teme
         0.2f, -0.2f, 1.0f, 0.0f, // donje desno teme
         0.2f, 0.2f, 1.0f, 1.0f,// gornje desno teme
    };

    // Krug od NUM_SLICES isečeka će imati 2 dodatna temena: jedno je centar, a još jedno je ponovljeno prvo teme na kružnici kako bi se oblik zatvorio
    // Broj elemenata u nizu je pomnožen sa 2 jer ćemo za svako teme pamtiti x i y koordinatu


    unsigned int VAOrect;
    unsigned int VAOrect2;
    unsigned int VAOrectBubbles;
    formVAOs(verticesRect, sizeof(verticesRect), VAOrect);
    formVAOs(verticesRect, sizeof(verticesRect), VAOrect2);
    formVAOs(verticesRect, sizeof(verticesRect), VAOrectBubbles);

    glClearColor(0.5f, 0.6f, 1.0f, 1.0f); // Postavljanje boje pozadine

    while (!glfwWindowShouldClose(window))
    {
        // Procesovanje inputa sa tastature je izolovan od crtanja
        // Ukoliko postoji puno tastera sa različitim funkcijama, moguće je razdvojiti processInput() kao posebnu funkciju

        //Gold fish
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            uX -= 0.001f;
            if (!ufacingRight) {
                uFlipX = 1.0f;
                ufacingRight = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            uX += 0.001f;
            if (ufacingRight) {
                uFlipX = -1.0f;
                ufacingRight = false;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            uY -= 0.001;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            uY += 0.001;
        }

        //Clown fish
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            cX -= 0.001f;
            if (!cfacingRight) {
                cFlipX = 1.0f;
                cfacingRight = true;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            cX += 0.001f;
            if (cfacingRight) {
                cFlipX = -1.0f;
                cfacingRight = false;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            cY -= 0.001;
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            cY += 0.001;
        }
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            drawRectBubbles(rectShader, VAOrectBubbles);
        }
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
            drawRectBubbles(rectShader, VAOrectBubbles);
        }

        glClear(GL_COLOR_BUFFER_BIT); // Bojenje pozadine, potrebno kako pomerajući objekti ne bi ostavljali otisak

        drawRect(rectShader, VAOrect);
        drawRect2(rectShader, VAOrect2);
        drawRect2(rectShader, VAOrectBubbles);

        glfwSwapBuffers(window); // Zamena bafera - prednji i zadnji bafer se menjaju kao štafeta; dok jedan procesuje, drugi se prikazuje.
        glfwPollEvents(); // Sinhronizacija pristiglih događaja
    }

    glDeleteProgram(rectShader);
    glDeleteProgram(colorShader);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}