#include "StdAfx.h"
#include "GLRenderer.h"
#include "DImage.h"
#include "GL\gl.h"
#include "GL\glu.h"
#include "GL\glaux.h"
#include "GL\glut.h"
#include <math.h>
//#pragma comment(lib, "GL\\glut32.lib")

namespace
{
	const double PI = 3.14159265358979323846;
	const double RAD_TO_DEG = 180.0 / PI;
	const double VIEW_TARGET_Y = 8.0;
	const double ENV_SIZE = 120.0;
	const int LAMP_SEG = 36;
}

CGLRenderer::CGLRenderer(void)
	: m_hrc(NULL)
	, m_texLamp(0)
	, m_texSide(0)
	, m_texTop(0)
	, m_texBot(0)
	, m_viewR(35.0)
	, m_viewPhi(0.5)
	, m_viewTheta(-0.8)
	, m_angleArm1(-0.6)
	, m_angleArm2(1.3)
	, m_angleHead(-1.1)
{
}

CGLRenderer::~CGLRenderer(void)
{
}

bool CGLRenderer::CreateGLContext(CDC* pDC)
{
	PIXELFORMATDESCRIPTOR pfd ;
   	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
   	pfd.nSize  = sizeof(PIXELFORMATDESCRIPTOR);
   	pfd.nVersion   = 1; 
   	pfd.dwFlags    = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;   
   	pfd.iPixelType = PFD_TYPE_RGBA; 
   	pfd.cColorBits = 32;
   	pfd.cDepthBits = 24; 
   	pfd.iLayerType = PFD_MAIN_PLANE;
	
	int nPixelFormat = ChoosePixelFormat(pDC->m_hDC, &pfd);
	
	if (nPixelFormat == 0) return false; 

	BOOL bResult = SetPixelFormat (pDC->m_hDC, nPixelFormat, &pfd);
  	
	if (!bResult) return false; 

   	m_hrc = wglCreateContext(pDC->m_hDC); 

	if (!m_hrc) return false; 

	return true;	
}

void CGLRenderer::PrepareScene(CDC *pDC)
{
	wglMakeCurrent(pDC->m_hDC, m_hrc);
	//---------------------------------
	char lampFile[] = "lamp.jpg";
	char sideFile[] = "side.jpg";
	char topFile[] = "top.jpg";
	char botFile[] = "bot.jpg";

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	m_texLamp = LoadTexture(lampFile);
	m_texSide = LoadTexture(sideFile);
	m_texTop = LoadTexture(topFile);
	m_texBot = LoadTexture(botFile);
	//---------------------------------
	wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::DrawScene(CDC *pDC)
{
	if (m_hrc == NULL)
		return;

	wglMakeCurrent(pDC->m_hDC, m_hrc);
	//---------------------------------
	double eyeX = m_viewR * cos(m_viewPhi) * cos(m_viewTheta);
	double eyeY = VIEW_TARGET_Y + m_viewR * sin(m_viewPhi);
	double eyeZ = m_viewR * cos(m_viewPhi) * sin(m_viewTheta);
	GLfloat viewerLightDir[] = { 0.0f, 0.0f, 1.0f, 0.0f };

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glLightfv(GL_LIGHT0, GL_POSITION, viewerLightDir);

	gluLookAt(eyeX, eyeY, eyeZ,
		0.0, VIEW_TARGET_Y, 0.0,
		0.0, 1.0, 0.0);

	DrawEnvCube(ENV_SIZE);
	SetupLights();

	glPushMatrix();
	DrawLamp();
	glPopMatrix();

	DrawAxes();

	glFlush();
	SwapBuffers(pDC->m_hDC);
	//---------------------------------
	wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::Reshape(CDC *pDC, int w, int h)
{
	wglMakeCurrent(pDC->m_hDC, m_hrc);
	//---------------------------------
	if (w <= 0)
		w = 1;
	if (h <= 0)
		h = 1;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double)w / (double)h, 1.0, 500.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//---------------------------------
	wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::OnKeyDown(UINT nChar)
{
	switch (nChar)
	{
	case VK_LEFT:
		m_viewTheta -= 0.1;
		break;
	case VK_RIGHT:
		m_viewTheta += 0.1;
		break;
	case VK_UP:
		m_viewPhi += 0.1;
		break;
	case VK_DOWN:
		m_viewPhi -= 0.1;
		break;
	case '1':
		m_angleArm1 += 0.1;
		break;
	case '2':
		m_angleArm1 -= 0.1;
		break;
	case '3':
		m_angleArm2 += 0.1;
		break;
	case '4':
		m_angleArm2 -= 0.1;
		break;
	case '5':
		m_angleHead += 0.1;
		break;
	case '6':
		m_angleHead -= 0.1;
		break;
	case VK_ADD:
	case VK_OEM_PLUS:
		m_viewR -= 1.0;
		break;
	case VK_SUBTRACT:
	case VK_OEM_MINUS:
		m_viewR += 1.0;
		break;
	default:
		break;
	}

	if (m_viewPhi < 0.1)
		m_viewPhi = 0.1;
	if (m_viewPhi > 1.5)
		m_viewPhi = 1.5;

	if (m_viewR < 10.0)
		m_viewR = 10.0;
	if (m_viewR > 100.0)
		m_viewR = 100.0;
}

void CGLRenderer::DrawAxes()
{
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);

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

void CGLRenderer::SetupLights()
{
	GLfloat globalAmbient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
	GLfloat lightAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
	GLfloat lightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightSpecular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat worldLightDir[] = { 0.0f, 1.0f, 1.0f, 0.0f };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);

	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

	glLightfv(GL_LIGHT1, GL_AMBIENT, lightAmbient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, lightSpecular);
	glLightfv(GL_LIGHT1, GL_POSITION, worldLightDir);
}

UINT CGLRenderer::LoadTexture(char* fileName)
{
	DImage img;
	UINT texID = 0;
	CString filePath(fileName);
	CString projectPath = CString("GLK\\") + fileName;
	CString debugPath = CString("..\\GLK\\") + fileName;

	if (!img.Load(filePath) && !img.Load(projectPath) && !img.Load(debugPath))
		return 0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.Width(), img.Height(),
		GL_BGRA_EXT, GL_UNSIGNED_BYTE, img.GetDIBBits());

	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}

void CGLRenderer::DrawEnvCube(double a)
{
	double h = a / 2.0;

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_DEPTH_BUFFER_BIT | GL_POLYGON_BIT);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDepthMask(FALSE);
	glColor3f(1.0f, 1.0f, 1.0f);

	glPushMatrix();
	glTranslated(0.0, h, 0.0);

	glBindTexture(GL_TEXTURE_2D, m_texSide);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3d(-h, -h,  h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d( h, -h,  h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d( h,  h,  h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-h,  h,  h);

		glTexCoord2f(0.0f, 1.0f); glVertex3d( h, -h, -h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d(-h, -h, -h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d(-h,  h, -h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d( h,  h, -h);

		glTexCoord2f(0.0f, 1.0f); glVertex3d(-h, -h, -h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d(-h, -h,  h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d(-h,  h,  h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-h,  h, -h);

		glTexCoord2f(0.0f, 1.0f); glVertex3d( h, -h,  h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d( h, -h, -h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d( h,  h, -h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d( h,  h,  h);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, m_texTop);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3d(-h,  h,  h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d( h,  h,  h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d( h,  h, -h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-h,  h, -h);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, m_texBot);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f); glVertex3d(-h, -h, -h);
		glTexCoord2f(1.0f, 1.0f); glVertex3d( h, -h, -h);
		glTexCoord2f(1.0f, 0.0f); glVertex3d( h, -h,  h);
		glTexCoord2f(0.0f, 0.0f); glVertex3d(-h, -h,  h);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();

	glDepthMask(TRUE);
	glPopAttrib();
}

void CGLRenderer::DrawCylinder(double r1, double r2, double h, int nSeg, int texMode, bool bIsOpen)
{
	if (nSeg < 3 || m_texLamp == 0)
		return;

	double alphaStep = 2.0 * PI / nSeg;
	double dr = r1 - r2;
	double L = sqrt(dr * dr + h * h);
	double nr = (L == 0.0) ? 1.0 : h / L;
	double ny = (L == 0.0) ? 0.0 : dr / L;
	float tMin = (texMode == 0) ? 0.0f : 0.5f;
	float tMax = (texMode == 0) ? 0.5f : 1.0f;

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_texLamp);
	glColor3f(1.0f, 1.0f, 1.0f);

	glBegin(GL_TRIANGLE_STRIP);
	for (int i = 0; i <= nSeg; i++)
	{
		double alpha = i * alphaStep;
		double c = cos(alpha);
		double s = sin(alpha);

		glNormal3d(nr * c, ny, nr * s);
		glTexCoord2f((float)i / nSeg, tMax);
		glVertex3d(r1 * c, 0.0, r1 * s);

		glTexCoord2f((float)i / nSeg, tMin);
		glVertex3d(r2 * c, h, r2 * s);
	}
	glEnd();

	if (!bIsOpen)
	{
		glNormal3d(0.0, -1.0, 0.0);
		glBegin(GL_TRIANGLE_FAN);
			glTexCoord2f(0.5f, 0.25f);
			glVertex3d(0.0, 0.0, 0.0);
			for (int i = 0; i <= nSeg; i++)
			{
				double alpha = (nSeg - i) * alphaStep;
				double c = cos(alpha);
				double s = sin(alpha);
				glTexCoord2f((float)(0.5 + 0.5 * c), (float)(0.25 + 0.25 * s));
				glVertex3d(r1 * c, 0.0, r1 * s);
			}
		glEnd();

		glNormal3d(0.0, 1.0, 0.0);
		glBegin(GL_TRIANGLE_FAN);
			glTexCoord2f(0.5f, 0.25f);
			glVertex3d(0.0, h, 0.0);
			for (int i = 0; i <= nSeg; i++)
			{
				double alpha = i * alphaStep;
				double c = cos(alpha);
				double s = sin(alpha);
				glTexCoord2f((float)(0.5 + 0.5 * c), (float)(0.25 + 0.25 * s));
				glVertex3d(r2 * c, h, r2 * s);
			}
		glEnd();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glPopAttrib();
}

void CGLRenderer::DrawLampBase()
{
	DrawCylinder(8.0, 7.0, 2.0, LAMP_SEG, 0, false);
}

void CGLRenderer::DrawLampArm()
{
	glPushMatrix();
	glTranslated(0.0, -1.0, 0.0);
	DrawCylinder(3.0, 3.0, 2.0, LAMP_SEG, 1, false);
	glPopMatrix();

	DrawCylinder(1.0, 1.0, 15.0, LAMP_SEG, 1, false);
}

void CGLRenderer::DrawLampHead()
{
	glPushMatrix();
	glTranslated(0.0, -1.0, 0.0);
	DrawCylinder(3.0, 3.0, 2.0, LAMP_SEG, 1, false);
	glPopMatrix();

	DrawCylinder(2.0, 2.0, 2.0, LAMP_SEG, 1, false);

	glPushMatrix();
	glTranslated(0.0, 2.0, 0.0);
	DrawCylinder(2.0, 6.0, 5.0, LAMP_SEG, 0, true);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 7.0, 0.0);
	DrawCylinder(6.0, 6.0, 1.0, LAMP_SEG, 0, true);
	glPopMatrix();
}

void CGLRenderer::DrawLamp()
{
	glPushMatrix();
	DrawLampBase();

	glTranslated(0.0, 2.0, 0.0);
	glRotated(m_angleArm1 * RAD_TO_DEG, 0.0, 0.0, 1.0);
	DrawLampArm();

	glTranslated(0.0, 15.0, 0.0);
	glRotated(m_angleArm2 * RAD_TO_DEG, 0.0, 0.0, 1.0);
	DrawLampArm();

	glTranslated(0.0, 15.0, 0.0);
	glRotated(m_angleHead * RAD_TO_DEG, 0.0, 0.0, 1.0);
	DrawLampHead();
	glPopMatrix();
}

void CGLRenderer::DestroyScene(CDC *pDC)
{
	wglMakeCurrent(pDC->m_hDC, m_hrc);

	if (m_texLamp != 0)
	{
		glDeleteTextures(1, &m_texLamp);
		m_texLamp = 0;
	}
	if (m_texSide != 0)
	{
		glDeleteTextures(1, &m_texSide);
		m_texSide = 0;
	}
	if (m_texTop != 0)
	{
		glDeleteTextures(1, &m_texTop);
		m_texTop = 0;
	}
	if (m_texBot != 0)
	{
		glDeleteTextures(1, &m_texBot);
		m_texBot = 0;
	}

	wglMakeCurrent(NULL,NULL); 
	if(m_hrc) 
	{
		wglDeleteContext(m_hrc);
		m_hrc = NULL;
	}
}
