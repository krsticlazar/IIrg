#pragma once

class CGLRenderer
{
public:
	CGLRenderer(void);
	virtual ~CGLRenderer(void);
		
	bool CreateGLContext(CDC* pDC);			// kreira OpenGL Rendering Context
	void PrepareScene(CDC* pDC);			// inicijalizuje scenu,
	void Reshape(CDC* pDC, int w, int h);	// kod koji treba da se izvrsi svaki put kada se promeni velicina prozora ili pogleda i
	void DrawScene(CDC* pDC);				// iscrtava scenu
	void DestroyScene(CDC* pDC);			// dealocira resurse alocirane u drugim funkcijama ove klase,
	void OnKeyDown(UINT nChar);

protected:
	void DrawAxes();
	void SetupLights();
	UINT LoadTexture(char* fileName);
	void DrawEnvCube(double a);
	void DrawCylinder(double r1, double r2, double h, int nSeg, int texMode, bool bIsOpen);
	void DrawLampBase();
	void DrawLampArm();
	void DrawLampHead();
	void DrawLamp();

	HGLRC	 m_hrc; //OpenGL Rendering Context 
	UINT	 m_texLamp;
	UINT	 m_texSide;
	UINT	 m_texTop;
	UINT	 m_texBot;
	double	 m_viewR;
	double	 m_viewPhi;
	double	 m_viewTheta;
	double	 m_angleArm1;
	double	 m_angleArm2;
	double	 m_angleHead;

};
