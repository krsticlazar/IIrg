// #GLRenderer.h - deklaracije i polja koja skoro uvek dodajes
//
// public:
//     bool CreateGLContext(CDC* pDC);
//     void PrepareScene(CDC* pDC);
//     void Reshape(CDC* pDC, int w, int h);
//     void DrawScene(CDC* pDC);
//     void DestroyScene(CDC* pDC);
//     void OnKeyDown(UINT nChar);
//
// protected:
//     void DrawAxes();
//     void SetupLights();
//     UINT LoadTexture(char* fileName);
//     void PolarToCartesian(double R, double phi, double theta, double& x, double& y, double& z);
//     void CalcRotAxis(double x1, double y1, double z1, double x2, double y2, double z2,
//         double& x3, double& y3, double& z3);
//     HGLRC m_hrc;
//     UINT m_texEnv;
//     double m_cameraR;
//     double m_cameraPhi;
//     double m_cameraTheta;
//     double m_openAngle;


// #GLRenderer.cpp - include i helper za uglove
//
// #include "stdafx.h"
// #include "GLRenderer.h"
// #include "DImage.h"
// #include "GL\\gl.h"
// #include "GL\\glu.h"
// #include "GL\\glaux.h"
// #include "GL\\glut.h"
// #include <atlconv.h>
// #include <math.h>
//
// namespace
// {
//     const double PI = 3.14159265358979323846;
//
//     double DegToRad(double angle)
//     {
//         return angle * PI / 180.0;
//     }
// }


// #CGLRenderer ctor - tipican start za kameru i teksturu
CGLRenderer::CGLRenderer(void)
    : m_hrc(NULL)
    , m_texEnv(0)
    , m_cameraR(20.0)
    , m_cameraPhi(25.0)
    , m_cameraTheta(45.0)
    , m_openAngle(0.0)
{
}


// #CreateGLContext - obavezno
bool CGLRenderer::CreateGLContext(CDC* pDC)
{
    PIXELFORMATDESCRIPTOR pfd;
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int nPixelFormat = ChoosePixelFormat(pDC->m_hDC, &pfd);
    if (nPixelFormat == 0)
        return false;

    if (!SetPixelFormat(pDC->m_hDC, nPixelFormat, &pfd))
        return false;

    m_hrc = wglCreateContext(pDC->m_hDC);
    return m_hrc != NULL;
}


// #PrepareScene - pocetno stanje scene, pozadina, texture support
void CGLRenderer::PrepareScene(CDC* pDC)
{
    if (m_hrc == NULL)
        return;

    wglMakeCurrent(pDC->m_hDC, m_hrc);

    GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    char fileName[] = "Env.jpg";

    glClearColor(0.02f, 0.08f, 0.06f, 1.0f);
    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    m_texEnv = LoadTexture(fileName);

    wglMakeCurrent(NULL, NULL);
}


// #Reshape - perspektiva, FOV menjas po blanket-u
void CGLRenderer::Reshape(CDC* pDC, int w, int h)
{
    if (m_hrc == NULL)
        return;

    wglMakeCurrent(pDC->m_hDC, m_hrc);

    if (w <= 0)
        w = 1;
    if (h <= 0)
        h = 1;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(50.0, (double)w / (double)h, 1.0, 200.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    wglMakeCurrent(NULL, NULL);
}


// #DrawScene - kamera, svetla, ose i mesto gde nastavljas crtanje zadatka
void CGLRenderer::DrawScene(CDC* pDC)
{
    if (m_hrc == NULL)
        return;

    wglMakeCurrent(pDC->m_hDC, m_hrc);

    double eyeX, eyeY, eyeZ;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    PolarToCartesian(m_cameraR, m_cameraPhi, m_cameraTheta, eyeX, eyeY, eyeZ);
    gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    SetupLights();
    DrawAxes();

    // Ovde nastavljas DrawSphere / DrawFlower / ostatak scene.

    glFlush();
    SwapBuffers(pDC->m_hDC);

    wglMakeCurrent(NULL, NULL);
}


// #DestroyScene - obavezno brisanje tekstura i context-a
void CGLRenderer::DestroyScene(CDC* pDC)
{
    if (m_hrc == NULL)
        return;

    wglMakeCurrent(pDC->m_hDC, m_hrc);

    // Obavezno brisanje teksture ako je prethodno kreirana.
    if (m_texEnv != 0)
    {
        glDeleteTextures(1, &m_texEnv);
        m_texEnv = 0;
    }

    wglMakeCurrent(NULL, NULL);

    if (m_hrc)
    {
        wglDeleteContext(m_hrc);
        m_hrc = NULL;
    }
}


// #OnKeyDown - tipican sablon za kameru i A/S animaciju
void CGLRenderer::OnKeyDown(UINT nChar)
{
    switch (nChar)
    {
    case VK_LEFT:
        m_cameraTheta -= 5.0;
        break;
    case VK_RIGHT:
        m_cameraTheta += 5.0;
        break;
    case VK_UP:
        m_cameraPhi += 5.0;
        break;
    case VK_DOWN:
        m_cameraPhi -= 5.0;
        break;
    case VK_ADD:
    case VK_OEM_PLUS:
        m_cameraR -= 1.0;
        break;
    case VK_SUBTRACT:
    case VK_OEM_MINUS:
        m_cameraR += 1.0;
        break;
    case 'A':
        m_openAngle += 5.0;
        break;
    case 'S':
        m_openAngle -= 5.0;
        break;
    default:
        break;
    }

    if (m_cameraPhi > 89.0)
        m_cameraPhi = 89.0;
    if (m_cameraPhi < -89.0)
        m_cameraPhi = -89.0;

    if (m_cameraR < 5.0)
        m_cameraR = 5.0;
    if (m_cameraR > 80.0)
        m_cameraR = 80.0;

    if (m_openAngle < 0.0)
        m_openAngle = 0.0;
    if (m_openAngle > 90.0)
        m_openAngle = 90.0;

    if (m_cameraTheta >= 360.0)
        m_cameraTheta -= 360.0;
    if (m_cameraTheta < 0.0)
        m_cameraTheta += 360.0;
}


// #DrawAxes - 50 jedinica, X plavo, Y crveno, Z zeleno
void CGLRenderer::DrawAxes()
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glLineWidth(2.0f);

    glBegin(GL_LINES);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(50.0f, 0.0f, 0.0f);

        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 50.0f, 0.0f);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 50.0f);
    glEnd();

    glPopAttrib();
}


// #SetupLights - dva direkciona bela izvora i global ambient
void CGLRenderer::SetupLights()
{
    GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat light0Pos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
    GLfloat light1Pos[] = { 0.0f, 1.0f, 1.0f, 0.0f };

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Pos);

    glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
    glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
}


// #LoadTexture - DImage helper, obavezno dodaj DImage.cpp i DImage.h u projekat
UINT CGLRenderer::LoadTexture(char* fileName)
{
    DImage img;
    UINT texID = 0;

    USES_CONVERSION;
    CString filePath(A2CT(fileName));
    if (!img.Load(filePath))
        return 0;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.Width(), img.Height(),
        GL_BGRA_EXT, GL_UNSIGNED_BYTE, img.GetDIBBits());

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}


// #PolarToCartesian - kamera ili sfera
void CGLRenderer::PolarToCartesian(double R, double phi, double theta, double& x, double& y, double& z)
{
    double phiRad = DegToRad(phi);
    double thetaRad = DegToRad(theta);

    x = R * cos(phiRad) * cos(thetaRad);
    y = R * sin(phiRad);
    z = R * cos(phiRad) * sin(thetaRad);
}


// #CalcRotAxis - osa kroz dve tacke, normalizovana
void CGLRenderer::CalcRotAxis(double x1, double y1, double z1, double x2, double y2, double z2,
    double& x3, double& y3, double& z3)
{
    double len;

    x3 = x2 - x1;
    y3 = y2 - y1;
    z3 = z2 - z1;

    len = sqrt(x3 * x3 + y3 * y3 + z3 * z3);
    if (len == 0.0)
    {
        x3 = 0.0;
        y3 = 1.0;
        z3 = 0.0;
        return;
    }

    x3 /= len;
    y3 /= len;
    z3 /= len;
}


// #SetMaterial - kratak helper za materijale
void SetMaterial(
    float ar, float ag, float ab,
    float dr, float dg, float db,
    float sr, float sg, float sb,
    float shininess)
{
    GLfloat ambient[] = { ar, ag, ab, 1.0f };
    GLfloat diffuse[] = { dr, dg, db, 1.0f };
    GLfloat specular[] = { sr, sg, sb, 1.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}


// #GLKView.h - za tastaturu
//
// public:
//     afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);


// #GLKView.cpp - poruka za tastaturu i fokus
//
// U BEGIN_MESSAGE_MAP dodaj:
// ON_WM_KEYDOWN()
//
// U OnInitialUpdate dodaj:
// SetFocus();
//
// Implementacija:
// void CGLKView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
// {
//     m_glRenderer.OnKeyDown(nChar);
//     Invalidate(FALSE);
//     CView::OnKeyDown(nChar, nRepCnt, nFlags);
// }


// #vcxproj - ako koristis teksture preko DImage
//
// Dodaj u GLK.vcxproj:
// <ClInclude Include="DImage.h" />
// <ClCompile Include="DImage.cpp" />
//
// Dodaj isto i u GLK.vcxproj.filters.
// Ubaci i samu sliku, npr. Env.jpg, u isti folder gde je GLRenderer.cpp.
