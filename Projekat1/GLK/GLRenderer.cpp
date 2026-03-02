#include "StdAfx.h"
#include "GLRenderer.h"
#include "DImage.h"
#include "GL\gl.h"
#include "GL\glu.h"
#include "GL\glaux.h"
#include "GL\glut.h"
#include <math.h>
//#pragma comment(lib, "GL\\glut32.lib")

CGLRenderer::CGLRenderer(void)
	: m_hrc(NULL)
	, m_texEnv(0)
	, m_viewR(20.0)
	, m_viewPhi(25.0)
	, m_viewTheta(45.0)
	, m_openAngle(0.0)
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
	char fileName[] = "Env.jpg";

	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	m_texEnv = LoadTexture(fileName);
	//---------------------------------
	wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::DrawScene(CDC *pDC)
{
	if (m_hrc == NULL) return;	
	wglMakeCurrent(pDC->m_hDC, m_hrc);
	//---------------------------------
	double eyeX, eyeY, eyeZ;

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	PolarToCartesian(m_viewR, m_viewPhi, m_viewTheta, eyeX, eyeY, eyeZ);
	gluLookAt(eyeX, eyeY, eyeZ,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);

	SetupLights();

	glPushMatrix();
	glTranslated(eyeX, eyeY, eyeZ);
	DrawSphere(60.0f, 36, 18);
	glPopMatrix();

	glPushMatrix();
	DrawSphere(2.0f, 36, 18);
	glPopMatrix();

	glPushMatrix();
	DrawAxes();
	glPopMatrix();

	glPushMatrix();
	glTranslated(12.0, 0.0, 0.0);
	DrawSphere(1.0f, 24, 12);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, 0.0, 12.0);
	DrawSphere(1.0f, 24, 12);
	glPopMatrix();

	glFlush();
	SwapBuffers(pDC->m_hDC);
	//---------------------------------
	wglMakeCurrent(NULL, NULL);
}

void CGLRenderer::Reshape(CDC *pDC, int w, int h)
{
	wglMakeCurrent(pDC->m_hDC, m_hrc);
	//---------------------------------
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(50.0, (double)w / (double)h, 1.0, 100.0);

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
		m_viewTheta -= 5.0;
		break;
	case VK_RIGHT:
		m_viewTheta += 5.0;
		break;
	case VK_UP:
		m_viewPhi += 5.0;
		break;
	case VK_DOWN:
		m_viewPhi -= 5.0;
		break;
	case VK_ADD:
	case VK_OEM_PLUS:
		m_viewR -= 1.0;
		break;
	case VK_SUBTRACT:
	case VK_OEM_MINUS:
		m_viewR += 1.0;
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

	if (m_viewPhi > 89.0)
		m_viewPhi = 89.0;
	if (m_viewPhi < -89.0)
		m_viewPhi = -89.0;

	if (m_viewR < 5.0)
		m_viewR = 5.0;
	if (m_viewR > 80.0)
		m_viewR = 80.0;

	if (m_openAngle < 0.0)
		m_openAngle = 0.0;
	if (m_openAngle > 90.0)
		m_openAngle = 90.0;
}

void CGLRenderer::DrawAxes()
{
	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT | GL_LINE_BIT);

	glDisable(GL_LIGHTING);
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
	GLfloat light0Pos[] = { 0.0f, 0.0f, 1.0f, 0.0f };
	GLfloat light1Pos[] = { 0.0f, 1.0f, 1.0f, 0.0f };

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

UINT CGLRenderer::LoadTexture(char* fileName)
{
	DImage img;
	UINT texID = 0;

	CString filePath(fileName);
	if (!img.Load(filePath))
		return 0;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, img.Width(), img.Height(),
		GL_BGRA_EXT, GL_UNSIGNED_BYTE, img.GetDIBBits());

	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}

void CGLRenderer::PolarToCartesian(double R, double phi, double theta, double& x, double& y, double& z)
{
	double phiRad = phi * 3.14159265358979323846 / 180.0;
	double thetaRad = theta * 3.14159265358979323846 / 180.0;

	x = R * cos(phiRad) * cos(thetaRad);
	y = R * sin(phiRad);
	z = R * cos(phiRad) * sin(thetaRad);
}

void CGLRenderer::DrawSphere(float R, int n, int m)
{
	if (n < 3 || m < 2 || m_texEnv == 0)
		return;

	double phiStep = 180.0 / m;
	double thetaStep = 360.0 / n;

	glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);

	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_texEnv);
	glColor3f(1.0f, 1.0f, 1.0f);

	for (int i = 0; i < m; i++)
	{
		double phi1 = -90.0 + i * phiStep;
		double phi2 = phi1 + phiStep;

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= n; j++)
		{
			double theta = j * thetaStep;
			double x, y, z;
			float s = (float)j / n;
			float t1 = (float)i / m;
			float t2 = (float)(i + 1) / m;

			PolarToCartesian(R, phi1, theta, x, y, z);
			glTexCoord2f(s, t1);
			glVertex3f((float)x, (float)y, (float)z);

			PolarToCartesian(R, phi2, theta, x, y, z);
			glTexCoord2f(s, t2);
			glVertex3f((float)x, (float)y, (float)z);
		}
		glEnd();
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glPopAttrib();
}

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

void CGLRenderer::DestroyScene(CDC *pDC)
{
	wglMakeCurrent(pDC->m_hDC, m_hrc);

	// Obavezno brisanje teksture ako je prethodno kreirana.
	if (m_texEnv != 0)
	{
		glDeleteTextures(1, &m_texEnv);
		m_texEnv = 0;
	}

	wglMakeCurrent(NULL,NULL); 
	if(m_hrc) 
	{
		wglDeleteContext(m_hrc);
		m_hrc = NULL;
	}
}
