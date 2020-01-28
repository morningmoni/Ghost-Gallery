#include "stdlib.h" // 用exit(0)退出程序需要用到这个库
#include "glut.h"
#include "glaux.H"
#include <iostream>
#include <string>
#include "3DS.H"
#include <tchar.h> //改宽字符用到
#include <math.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "WinMM.Lib")
#pragma comment(lib, "glaux.lib")
#define MAX_TEXTURES 100
#define PI 3.1415
using namespace std;

GLuint g_Texture[MAX_TEXTURES] = {0};
GLuint bmp_Texture[MAX_TEXTURES] = {0};
static long ImageWidth, ImageWidth_floor1;
static long ImageHeight, ImageHeight_floor1;
static long PixelLength, PixelLength_floor1;
static GLubyte *Image, *Image_floor1;

CLoad3DS g_Load3ds, g_Load3ds2, g_Load3ds3;
t3DModel g_3DModel_table;
t3DModel g_3DModel_shark;
t3DModel g_3DModel_fly;

double x = 0, y = 80, angle = 90, z = -250, speed = 5;
double hudu;
double tmpX = x, tmpZ = z;
double mouseX, mouseY;
double R = 0, G = 0, B = 0;
int h = 200, drawLine = 0, drawLineLoc, manHeight = 80, pick = 0, timeUp = 0, timecount = 660, succeed = 0;

void Reshape(int width, int Height)
{
	float Ration_W_H;
	Height = (Height == 0 ? 1 : Height);
	Ration_W_H = 1.0 * width / Height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, Ration_W_H, 0.1, 300000);
	glViewport(0, 0, width, Height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
//没用到
void getMouse(int button, int state, int x, int y)
{
	if (state == GLUT_UP)
	{
		mouseX = x;
		mouseY = y;
	}
}
//  从文件中创建纹理
void CreateTexture(UINT textureArray[], LPSTR strFileName, int textureID, wstring name)
{
	AUX_RGBImageRec *pBitmap = NULL;
	if (!strFileName) // 如果无此文件，则直接返回
		return;

	pBitmap = auxDIBImageLoad(name.c_str()); // 装入位图，并保存数据

	if (pBitmap == NULL) // 如果装入位图失败，则退出
		exit(0);
	// 生成纹理
	glGenTextures(1, &textureArray[textureID]);
	// 设置像素对齐格式
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, textureArray[textureID]);
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, pBitmap->sizeX, pBitmap->sizeY, GL_RGB, GL_UNSIGNED_BYTE, pBitmap->data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	if (pBitmap) // 释放位图占用的资源
	{
		if (pBitmap->data)
		{
			free(pBitmap->data);
		}

		free(pBitmap);
	}
}

void loadBMP(char *name, long ImageWidth, long ImageHeight, long PixelLength, GLubyte *Image, int index)
{
	// 打开文件
	FILE *pFile = fopen(name, "rb");
	if (pFile == 0)
		exit(0);

	// 读取图象的大小信息
	fseek(pFile, 0x0012, SEEK_SET);
	fread(&ImageWidth, sizeof(ImageWidth), 1, pFile);
	fseek(pFile, 0x0016, SEEK_SET);
	fread(&ImageHeight, sizeof(ImageHeight), 1, pFile);

	// 计算像素数据长度
	PixelLength = ImageWidth * 3;
	while (PixelLength % 4 != 0)
		++PixelLength;
	PixelLength *= ImageHeight;

	// 读取像素数据
	Image = (GLubyte *)malloc(PixelLength);
	if (Image == 0)
		exit(0);

	fseek(pFile, 54, SEEK_SET);
	fread(Image, PixelLength, 1, pFile);

	// 关闭文件
	fclose(pFile);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, &bmp_Texture[index]);
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[index]);
	/*  定义纹理 */
	// 注意GL_BGR_EXT，BMP采用的是蓝绿红存储
	glTexImage2D(GL_TEXTURE_2D, 0, 3, ImageWidth, ImageHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, Image);
	/*  控制滤波 */
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT /*GL_CLAMP*/);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR /* GL_NEAREST*/); // 修改 试一下
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	/*  说明映射方式*/
	if (timeUp)
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND /*GL_DECAL*/); //add blend
	else
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL); //add blend
}
void loadAllBmp()
{
	loadBMP("floor.bmp", ImageWidth_floor1, ImageHeight_floor1, PixelLength_floor1, Image_floor1, 0);
	loadBMP("wall.bmp", ImageWidth, ImageHeight, PixelLength, Image, 1);
	loadBMP("floor2.bmp", ImageWidth, ImageHeight, PixelLength, Image, 2);
	loadBMP("wall2.bmp", ImageWidth, ImageHeight, PixelLength, Image, 3);
	loadBMP("floor3.bmp", ImageWidth, ImageHeight, PixelLength, Image, 4);
	loadBMP("wall3.bmp", ImageWidth, ImageHeight, PixelLength, Image, 5);
	loadBMP("star.bmp", ImageWidth, ImageHeight, PixelLength, Image, 6);
	loadBMP("ceiling.bmp", ImageWidth, ImageHeight, PixelLength, Image, 7);
	loadBMP("ceiling2.bmp", ImageWidth, ImageHeight, PixelLength, Image, 8);
	loadBMP("hole.bmp", ImageWidth, ImageHeight, PixelLength, Image, 9);

	loadBMP("pic1.bmp", ImageWidth, ImageHeight, PixelLength, Image, 11);
	loadBMP("pic2.bmp", ImageWidth, ImageHeight, PixelLength, Image, 12);
	loadBMP("pic3.bmp", ImageWidth, ImageHeight, PixelLength, Image, 13);
	loadBMP("pic4.bmp", ImageWidth, ImageHeight, PixelLength, Image, 14);
	loadBMP("pic5.bmp", ImageWidth, ImageHeight, PixelLength, Image, 15);
	loadBMP("pic6.bmp", ImageWidth, ImageHeight, PixelLength, Image, 16);
}
GLfloat light0_param[] = {1, 0, 0, 1}, light0_shine = 64, light0_posi[] = {0, 100, -500, 1}, sun_emission[] = {0.8, 0.1, 0.1, 1};
void lightInit()
{
	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightf(GL_LIGHT0, GL_EMISSION, *sun_emission);
	glLightf(GL_LIGHT0, GL_AMBIENT, *light0_param);
	glLightf(GL_LIGHT0, GL_DIFFUSE, *light0_param);
	glLightf(GL_LIGHT0, GL_SPECULAR, *light0_param);
	glLightf(GL_LIGHT0, GL_SHININESS, light0_shine);
	//glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 50);
	//glLightf(GL_LIGHT0, GL_POSITION, *light0_posi);
	//glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
	//GLfloat  spot_direction[] = { x + 10 * cos(hudu), y, z - 10 * sin(hudu) };
	//glLightf(GL_LIGHT0, GL_SPOT_DIRECTION, *spot_direction);
	//glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 50);
}
void Init()
{
	g_Load3ds.Import3DS(&g_3DModel_table, "table.3ds"); // 将3ds文件装入到模型结构体中
	g_Load3ds2.Import3DS(&g_3DModel_shark, "shark.3ds");
	g_Load3ds3.Import3DS(&g_3DModel_fly, "fly.3ds");

	for (int i = 0; i < g_3DModel_shark.numOfMaterials; i++)
	{

		if (strlen(g_3DModel_shark.pMaterials[i].strFile) > 0)
		{
			CreateTexture(g_Texture, g_3DModel_shark.pMaterials[i].strFile, i, L"shark.bmp");
		}
		g_3DModel_shark.pMaterials[i].texureId = i;
	}
	glEnable(GL_NORMALIZE);

	loadAllBmp();
}
//之后没用
void Ground()
{
	glLineWidth(1);
	glColor3f(0.2, 0.2, 0.2);
	glBegin(GL_LINES);

	// 画地面横线
	for (int i = 0; i < 30; i++)
	{
		glVertex3f(-3000, 0, -i * 100);
		glVertex3f(3000, 0, -i * 100);
	}
	// 画地面竖线
	for (int i = -30; i < 30; i++)
	{
		glVertex3f(-i * 100, 0, 0);
		glVertex3f(-i * 100, 0, -3000);
	}
	glEnd();
}
void draw3DS(t3DModel obj, wstring name)
{
	glEnable(GL_TEXTURE_2D);

	for (int i = 0; i < obj.numOfMaterials; i++)
	{
		// 判断是否是一个文件名
		if (strlen(obj.pMaterials[i].strFile) > 0)
		{
			//  使用纹理文件名称来装入位图
			CreateTexture(g_Texture, obj.pMaterials[i].strFile, i, name);
		}

		// 设置材质的纹理ID
		obj.pMaterials[i].texureId = i;
	}

	//遍历模型中所有的对象
	for (int i = 0; i < obj.numOfObjects; i++)
	{
		// 如果对象的大小小于0，则退出
		if (obj.pObject.size() <= 0)
			break;

		// 获得当前显示的对象
		t3DObject *pObject = &obj.pObject[i];

		//判断该对象是否有纹理映射
		if (pObject->bHasTexture)
		{

			// 打开纹理映射
			glEnable(GL_TEXTURE_2D);
			glColor3ub(255, 255, 255);
			glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
		}
		else
		{

			//关闭纹理映射
			glDisable(GL_TEXTURE_2D);
			glColor3ub(255, 255, 255);
		}
		// 开始以g_ViewMode模式绘制

		glBegin(GL_TRIANGLES);
		// 遍历所有的面
		for (int j = 0; j < pObject->numOfFaces; j++)
		{
			//遍历三角形的所有点
			for (int whichVertex = 0; whichVertex < 3; whichVertex++)
			{
				// 获得面对每个点的索引
				int index = pObject->pFaces[j].vertIndex[whichVertex];

				// 给出法向量
				glNormal3f(pObject->pNormals[index].x, pObject->pNormals[index].y, pObject->pNormals[index].z);

				//如果对象具有纹理
				if (pObject->bHasTexture)
				{

					// 确定是否有UVW纹理坐标
					if (pObject->pTexVerts)
					{
						glTexCoord2f(pObject->pTexVerts[index].x, pObject->pTexVerts[index].y);
					}
				}
				else
				{

					if (obj.pMaterials.size() && pObject->materialID >= 0)
					{
						BYTE *pColor = obj.pMaterials[pObject->materialID].color;
						glColor3ub(pColor[0], pColor[1], pColor[2]);
					}
				}
				glVertex3f(pObject->pVerts[index].x, pObject->pVerts[index].y, pObject->pVerts[index].z);
			}
		}
		glEnd(); // 绘制结束
	}

	glDisable(GL_TEXTURE_2D);
}
void drawWall()
{
	glEnable(GL_TEXTURE_2D);
	//ground
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[0]);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, 0);
	glTexCoord2f(0.0, 28.0);
	glVertex3f(-200, 0, -2800);
	glTexCoord2f(32.0, 28.0);
	glVertex3f(3000, 0, -2800);
	glTexCoord2f(32.0, 0.0);
	glVertex3f(3000, 0, 0);
	glEnd();
	//ceiling
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[7]);
	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, h, 0);
	glTexCoord2f(0.0, 28.0);
	glVertex3f(-200, h, -2800);
	glTexCoord2f(32.0, 28.0);
	glVertex3f(3000, h, -2800);
	glTexCoord2f(32.0, 0.0);
	glVertex3f(3000, h, 0);
	glEnd();
	//wall
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[1]);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-200, h, 0);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(-200, h, -1400);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(-200, 0, -1400);

	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, -1400);
	glTexCoord2f(0, 1.0);
	glVertex3f(-200, h, -1400);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(1200, h, -1400);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(1200, 0, -1400);

	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1200, 0, -1400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1200, h, -1400);
	glTexCoord2f(5.0, 1.0);
	glVertex3f(1200, h, -2400);
	glTexCoord2f(5.0, 0.0);
	glVertex3f(1200, 0, -2400);

	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1200, 0, -2400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1200, h, -2400);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(-200, h, -2400);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(-200, 0, -2400);

	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, -2400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-200, h, -2400);
	glTexCoord2f(2.0, 1.0);
	glVertex3f(-200, h, -2800);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-200, 0, -2800);

	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, -2800);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-200, h, -2800);
	glTexCoord2f(9.0, 1.0);
	glVertex3f(1600, h, -2800);
	glTexCoord2f(9.0, 0.0);
	glVertex3f(1600, 0, -2800);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1600, 0, -2800);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1600, h, -2800);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(1600, h, -1400);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(1600, 0, -1400);

	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1600, 0, -1400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1600, h, -1400);
	glTexCoord2f(5.0, 1.0);
	glVertex3f(2600, h, -1400);
	glTexCoord2f(5.0, 0.0);
	glVertex3f(2600, 0, -1400);

	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(2600, 0, -1400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(2600, h, -1400);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(2600, h, -2800);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(2600, 0, -2800);

	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(2600, 0, -2800);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(2600, h, -2800);
	glTexCoord2f(2.0, 1.0);
	glVertex3f(3000, h, -2800);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(3000, 0, -2800);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(3000, 0, -2800);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(3000, h, -2800);
	glTexCoord2f(9.0, 1.0);
	glVertex3f(3000, h, -1000);
	glTexCoord2f(9.0, 0.0);
	glVertex3f(3000, 0, -1000);

	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(3000, 0, -1000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(3000, h, -1000);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(1600, h, -1000);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(1600, 0, -1000);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1600, 0, -1000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1600, h, -1000);
	glTexCoord2f(3.0, 1.0);
	glVertex3f(1600, h, -400);
	glTexCoord2f(3.0, 0.0);
	glVertex3f(1600, 0, -400);

	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1600, 0, -400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1600, h, -400);
	glTexCoord2f(7.0, 1.0);
	glVertex3f(3000, h, -400);
	glTexCoord2f(7.0, 0.0);
	glVertex3f(3000, 0, -400);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(3000, 0, -400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(3000, h, -400);
	glTexCoord2f(2.0, 1.0);
	glVertex3f(3000, h, 0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(3000, 0, 0);

	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(3000, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(3000, h, 0);
	glTexCoord2f(9.0, 1.0);
	glVertex3f(1200, h, 0);
	glTexCoord2f(9.0, 0.0);
	glVertex3f(1200, 0, 0);

	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1200, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1200, h, 0);
	glTexCoord2f(5.0, 1.0);
	glVertex3f(1200, h, -1000);
	glTexCoord2f(5.0, 0.0);
	glVertex3f(1200, 0, -1000);

	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1200, 0, -1000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(1200, h, -1000);
	glTexCoord2f(5.0, 1.0);
	glVertex3f(200, h, -1000);
	glTexCoord2f(5.0, 0.0);
	glVertex3f(200, 0, -1000);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(200, 0, -1000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(200, h, -1000);
	glTexCoord2f(5.0, 1.0);
	glVertex3f(200, h, 0);
	glTexCoord2f(5.0, 0.0);
	glVertex3f(200, 0, 0);

	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(200, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(200, h, 0);
	glTexCoord2f(2.0, 1.0);
	glVertex3f(-200, h, 0);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-200, 0, 0);

	glEnd();
	//pic
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[11]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(550, 20, -1400 + 1);
	glTexCoord2f(0, 1.0);
	glVertex3f(550, 180, -1400 + 1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(700, 180, -1400 + 1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(700, 20, -1400 + 1);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[12]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(2100, 40, -1002);
	glTexCoord2f(0, 1.0);
	glVertex3f(2100, 160, -1002);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1950, 160, -1002);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(1950, 40, -1002);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[13]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1900, 40, -1400 + 1);
	glTexCoord2f(0, 1.0);
	glVertex3f(1900, 160, -1400 + 1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(2100, 160, -1400 + 1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(2100, 40, -1400 + 1);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[14]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(700, 20, -1002);
	glTexCoord2f(0, 1.0);
	glVertex3f(700, 180, -1002);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(500, 180, -1002);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(500, 20, -1002);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[15]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(80, 20, -1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(80, 180, -1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-80, 180, -1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-80, 20, -1);
	glEnd();
	//glBindTexture(GL_TEXTURE_2D, bmp_Texture[16]);
	//glBegin(GL_QUADS);
	//glTexCoord2f(0.0, 0.0); glVertex3f(3000 - 1, 20, -1800 - 80);
	//glTexCoord2f(0.0, 1.0); glVertex3f(3000 - 1, 180, -1800 - 80);
	//glTexCoord2f(1.0, 1.0); glVertex3f(3000 - 1, 180, -1800);
	//glTexCoord2f(1.0, 0.0); glVertex3f(3000 - 1, 20, -1800);
	//glEnd();
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200, 0, -2400);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-200, h, -2400);
	glTexCoord2f(2.0, 1.0);
	glVertex3f(-200, h, -2800);
	glTexCoord2f(2.0, 0.0);
	glVertex3f(-200, 0, -2800);
	//hole
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[9]);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-200 + 1, 0, -2440);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-200 + 1, 20, -2440);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-200 + 1, 20, -2480);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-200 + 1, 0, -2480);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
void drawWall2()
{
	glEnable(GL_TEXTURE_2D);
	//ground
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[2]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4000, 0, 0);
	glTexCoord2f(0.0, 80.0);
	glVertex3f(-4000, 0, -8000);
	glTexCoord2f(2.5, 80.0);
	glVertex3f(-4150, 0, -8000);
	glTexCoord2f(2.5, 0.0);
	glVertex3f(-4150, 0, 0);
	glEnd();
	//ceiling
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[8]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4000, h, 0);
	glTexCoord2f(0.0, 80.0);
	glVertex3f(-4000, h, -8000);
	glTexCoord2f(3, 80.0);
	glVertex3f(-4150, h, -8000);
	glTexCoord2f(3, 0.0);
	glVertex3f(-4150, h, 0);
	glEnd();
	//wall
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[3]);
	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4000, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4000, h, 0);
	glTexCoord2f(80.0, 1.0);
	glVertex3f(-4000, h, -8000);
	glTexCoord2f(80.0, 0.0);
	glVertex3f(-4000, 0, -8000);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4000, 0, -8000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4000, h, -8000);
	glTexCoord2f(2.5, 1.0);
	glVertex3f(-4150, h, -8000);
	glTexCoord2f(2.5, 0.0);
	glVertex3f(-4150, 0, -8000);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4150, 0, -8000);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4150, h, -8000);
	glTexCoord2f(80.0, 1.0);
	glVertex3f(-4150, h, 0);
	glTexCoord2f(80.0, 0.0);
	glVertex3f(-4150, 0, 0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4150, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4150, h, 0);
	glTexCoord2f(2.5, 1.0);
	glVertex3f(-4000, h, 0);
	glTexCoord2f(2.5, 0.0);
	glVertex3f(-4000, 0, 0);

	glEnd();
	//pic
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[16]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-4075 - 40, 20, -1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-4075 - 40, 160, -1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-4075 + 40, 160, -1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-4075 + 40, 20, -1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, bmp_Texture[12]);
	glBegin(GL_QUADS);
	for (int i = 0; i < 20; ++i)
	{
		glTexCoord2f(0.0, 0.0);
		glVertex3f(-4000 - 1, 40, -50 - 400 * i);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-4000 - 1, 160, -50 - 400 * i);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(-4000 - 1, 160, -200 - 400 * i);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-4000 - 1, 40, -200 - 400 * i);

		glTexCoord2f(0.0, 0.0);
		glVertex3f(-4150 + 1, 40, -50 - 400 * i);
		glTexCoord2f(0.0, 1.0);
		glVertex3f(-4150 + 1, 160, -50 - 400 * i);
		glTexCoord2f(1.0, 1.0);
		glVertex3f(-4150 + 1, 160, -200 - 400 * i);
		glTexCoord2f(1.0, 0.0);
		glVertex3f(-4150 + 1, 40, -200 - 400 * i);
	}
	glEnd();

	glDisable(GL_TEXTURE_2D);
}
void drawWall3()
{
	glEnable(GL_TEXTURE_2D);
	//ground
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[4]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-3000, 0, 0);
	glTexCoord2f(0.0, 5.0);
	glVertex3f(-3000, 0, -400);
	glTexCoord2f(10, 5.0);
	glVertex3f(-3300, 0, -400);
	glTexCoord2f(10, 0.0);
	glVertex3f(-3300, 0, 0);
	glEnd();

	//wall
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-3000, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-3000, 100, 0);
	glTexCoord2f(3, 1.0);
	glVertex3f(-3300, 100, 0);
	glTexCoord2f(3, 0.0);
	glVertex3f(-3300, 0, 0);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-3300, 0, 0);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-3300, 100, 0);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3300, 100, -100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3300, 0, -100);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3300, 100, -100);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3300, 0, -100);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3155, 0, -100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3155, 100, -100);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3155, 0, -100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3155, 100, -100);
	glTexCoord2f(1, 2.0);
	glVertex3f(-3155, 100, -300);
	glTexCoord2f(0, 2.0);
	glVertex3f(-3155, 0, -300);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3155, 100, -300);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3155, 0, -300);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3300, 0, -300);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3300, 100, -300);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3300, 0, -300);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3300, 100, -300);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3300, 100, -400);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3300, 0, -400);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3300, 100, -400);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3300, 0, -400);
	glTexCoord2f(1, 3.0);
	glVertex3f(-3000, 0, -400);
	glTexCoord2f(0, 3.0);
	glVertex3f(-3000, 100, -400);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3000, 0, -400);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3000, 100, -400);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3000, 100, -300);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3000, 0, -300);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3000, 100, -300);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3000, 0, -300);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3145, 0, -300);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3145, 100, -300);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3145, 0, -300);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3145, 100, -300);
	glTexCoord2f(1, 2.0);
	glVertex3f(-3145, 100, -100);
	glTexCoord2f(0, 2.0);
	glVertex3f(-3145, 0, -100);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3145, 100, -100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3145, 0, -100);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3000, 0, -100);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3000, 100, -100);

	glTexCoord2f(0, 0.0);
	glVertex3f(-3000, 0, -100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3000, 100, -100);
	glTexCoord2f(1, 1.0);
	glVertex3f(-3000, 100, 0);
	glTexCoord2f(0, 1.0);
	glVertex3f(-3000, 0, 0);
	glEnd();
	//ceiling
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[6]);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0.0);
	glVertex3f(-3400, 110, 100);
	glTexCoord2f(1, 0.0);
	glVertex3f(-3400, 110, -500);
	glTexCoord2f(1, 1.0);
	glVertex3f(-2970, 110, -500);
	glTexCoord2f(0, 1.0);
	glVertex3f(-2970, 110, 100);
	glEnd();
	//pic
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[15]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-3150 + 80, 20, -1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-3150 + 80, 100, -1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-3150 - 80, 100, -1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-3150 - 80, 20, -1);
	glEnd();
	glBindTexture(GL_TEXTURE_2D, bmp_Texture[16]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-3150 - 40, 0, -400 + 1);
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-3150 - 40, 100, -400 + 1);
	glTexCoord2f(1.0, 1.0);
	glVertex3f(-3150 + 40, 100, -400 + 1);
	glTexCoord2f(1.0, 0.0);
	glVertex3f(-3150 + 40, 0, -400 + 1);
	glEnd();
	glDisable(GL_TEXTURE_2D);
}
void Disp()
{

	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	hudu = angle / 180 * PI;
	gluLookAt(x, manHeight, z,
			  x + 10 * cos(hudu), y, z - 10 * sin(hudu),
			  0, 1, 0);
	if (timeUp)
	{
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		lightInit();
		float ambient[] = {R, G, B, 1}, diffuse[] = {R, G, B, 1},
			  specular[] = {0.35, 0.05, 0., 1}, emission[] = {R, G, B, 1};
		glMaterialfv(GL_LIGHT0, GL_EMISSION, sun_emission);
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
		if (timeUp == 1)
		{
			loadAllBmp();
			timeUp = 2;
		}
	}
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glPushMatrix();
	glTranslatef(0, 100, -500);
	glutSolidSphere(10, 200, 200);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0, -500);
	glRotatef(30, 0, 1, 0);
	glScalef(0.6, 0.6, 0.6);
	draw3DS(g_3DModel_table, L"table.jpg");
	glPopMatrix();

	glPushMatrix();
	glTranslatef(2750, 80, -2600);
	glRotatef(90, 0, 1, 0);
	glScalef(0.02, 0.02, 0.02);
	draw3DS(g_3DModel_fly, L"table.jpg");
	glPopMatrix();

	if (!pick)
	{
		glPushMatrix();
		glTranslatef(2800, 0, -100);
		glRotatef(60, 0, 1, 0);
		glScalef(1, 1, 1);
		draw3DS(g_3DModel_shark, L"shark.bmp");
		glPopMatrix();
	}
	glDisable(GL_LIGHTING);

	//Ground();
	if (timeUp)
		glEnable(GL_LIGHTING);
	drawWall();
	drawWall2();
	if (timeUp)
		glDisable(GL_LIGHTING);
	drawWall3();

	if (drawLine == 1)
	{
		glColor3f(0, 0, 0);
		glLineWidth(5);
		glBegin(GL_LINE_STRIP);
		glVertex3f(-4150 + 2, 80, drawLineLoc);
		glVertex3f(-4150 + 2, 75, drawLineLoc - 10);
		glVertex3f(-4150 + 2, 72, drawLineLoc + 40);
		glVertex3f(-4150 + 2, 65, drawLineLoc - 20);
		glVertex3f(-4150 + 2, 72, drawLineLoc + 40);
		glVertex3f(-4150 + 2, 65, drawLineLoc - 20);
		glVertex3f(-4150 + 2, 96, drawLineLoc + 20);
		glVertex3f(-4150 + 2, 47, drawLineLoc - 70);
		glEnd();
	}
	else if (drawLine == 2)
	{
		glColor3f(0, 0, 0);
		glLineWidth(5);
		glBegin(GL_LINE_STRIP);
		glVertex3f(-4000 - 2, 80, drawLineLoc);
		glVertex3f(-4000 - 2, 75, drawLineLoc - 10);
		glVertex3f(-4000 - 2, 72, drawLineLoc + 40);
		glVertex3f(-4000 - 2, 65, drawLineLoc - 50);
		glVertex3f(-4000 - 2, 72, drawLineLoc + 40);
		glVertex3f(-4000 - 2, 65, drawLineLoc - 50);
		glVertex3f(-4000 - 2, 96, drawLineLoc + 20);
		glVertex3f(-4000 - 2, 47, drawLineLoc - 70);
		glEnd();
	}
	glutSwapBuffers();
}

void TimerFun(int value)
{
	R += 0.025;
	G += 0.015;
	B += 0.01;
	if (R > 1)
		R -= 1;
	if (G > 1)
		G -= 1;
	if (B > 1)
		B -= 1;
	glutPostRedisplay();
	glutTimerFunc(50, TimerFun, 1);
}
void timeCount(int value)
{
	timecount--;
	cout << timecount << " ";
	if (timecount == 64)
	{
		timeUp = 1;
		cout << "\n灯怎么灭啦！！！！！" << endl;
		PlaySound(L"2.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	}
	if (timecount < 0)
	{
		manHeight += 1;
		cout << "GAME OVER" << endl;
	}
	if (succeed)
	{
		cout << "用时：" << 660 - timecount << "秒" << endl;
		return;
	}

	glutPostRedisplay();
	glutTimerFunc(1000, timeCount, 2);
}
int mark;
void judgeEdge()
{
	if (tmpX > -200 + 2 && tmpX <= 200 && ((tmpZ < 0 - 2 && tmpZ > -1400 + 2) || (tmpZ < -2400 - 2 && tmpZ > -2800 + 2)))
		mark = 1;
	else if (tmpX > 200 && tmpX <= 1200 && ((tmpZ < -1000 - 2 && tmpZ > -1400 + 2) || (tmpZ < -2400 - 2 && tmpZ > -2800 + 2)))
		mark = 1;
	else if (tmpX > 1200 && tmpX <= 1600 && (tmpZ < 0 - 2 && tmpZ > -2800 + 2))
		mark = 1;
	else if (tmpX > 1600 && tmpX <= 2600 && ((tmpZ < 0 - 2 && tmpZ > -400 + 2) || (tmpZ < -1000 - 2 && tmpZ > -1400 + 2)))
		mark = 1;
	else if (tmpX > 2600 && tmpX < 3000 - 2 && ((tmpZ < 0 - 2 && tmpZ > -400 + 2) || (tmpZ < -1000 - 2 && tmpZ > -2800 + 2)))
		mark = 1;

	else if (tmpX > -4150 + 2 && tmpX < -4000 - 2 && tmpZ < 0 - 2 && tmpZ > -8000 + 2)
		mark = 1;

	else if (tmpX > -3300 + 2 && tmpX < -3000 - 2 && ((tmpZ < 0 - 2 && tmpZ > -100) || (tmpZ < -300 && tmpZ > -400 + 2)))
	{
		mark = 1;
	}
	else if (manHeight == 10 && tmpX < -195 && tmpZ >= -2480 && tmpZ <= -2440)
	{
		for (int i = 0; i < 10; ++i)
			cout << "***************************************" << endl;
		cout << "****************已通关！****************" << endl;
		cout << "现在可使用上下左右箭头来移动（其实一开始就可以），有什么不一样吗？试试就知道：）" << endl;
		succeed = 1;
		manHeight = 80;
		y = manHeight;
		mark = 1;
		tmpX = 0;
		tmpZ = -200;
	}
	//pick
	//cout << fabs(tmpX - 2800) << " " << fabs(tmpZ + 100)<<endl;
	if (fabs(tmpX - 2800) < 50 && fabs(tmpZ + 100) < 50)
	{
		if (!pick)
		{
			cout << "\n这只笔的造型好奇怪，好像还没多少水了" << endl;
			Sleep(2000);
		}
		pick = 1;
	}

	//pass through
	//1->3
	if (tmpX >= -80 && tmpX <= 80 && manHeight == 80 && tmpZ > -5 && tmpZ < 100)
	{
		tmpX = -3150;
		tmpZ = -20;
		manHeight = 40;
		y = manHeight;
		mark = 1;
		cout << "\n我居然...穿越了！" << endl;
		cout << "前面的墙好狭窄，我该怎么过去呢？" << endl;
	}
	//3->1
	else if (tmpX >= -3150 - 80 && tmpX <= -3150 + 80 && manHeight == 40 && tmpZ > -5 && tmpZ < 100)
	{
		tmpX = 0;
		tmpZ = -20;
		manHeight = 80;
		y = manHeight;
		mark = 1;
		cout << "\n我又回来了，一定得想办法逃出去" << endl;
	}
	//3->2
	else if (tmpX >= -3150 - 40 && tmpX <= -3150 + 40 && tmpZ > -450 && tmpZ < -395)
	{
		tmpX = -4075;
		tmpZ = -20;
		manHeight = 80;
		y = manHeight;
		mark = 1;
		cout << "\n又一次穿越了..." << endl;
		cout << "\n这么多一样的画,看起来好诡异" << endl;
	}
	//2->3
	else if (tmpX >= -4075 - 40 && tmpX <= -4075 + 40 && manHeight == 80 && tmpZ > -5 && tmpZ < 100)
	{
		tmpX = -3150;
		tmpZ = -400 + 20;
		manHeight = 40;
		y = manHeight;
		mark = 1;
		cout << "\n又回到了石头墙，天已经这么晚了……" << endl;
	}
	//2->1
	else if (drawLine == 1 && tmpX > -4150 - 100 && tmpX < -4135 && tmpZ > drawLineLoc - 50 && tmpZ < drawLineLoc + 50)
	{
		tmpX = 2000;
		tmpZ = -1020;
		manHeight = 10;
		y = manHeight;
		mark = 1;
		cout << "\n好像又回去了，咦，我怎么变小了？！" << endl;
	}
	else if (drawLine == 2 && tmpX > -4000 - 15 && tmpX < -4000 + 50 && tmpZ > drawLineLoc - 50 && tmpZ < drawLineLoc + 50)
	{
		tmpX = 2000;
		tmpZ = -1020;
		manHeight = 10;
		y = manHeight;
		mark = 1;
		cout << "\n天啊，我怎么变小了？！不是在做梦吧？怎么又回来了！" << endl;
	}
}
void info()
{
	cout << "外面雨那么大，还好这有间屋子" << endl;
	Sleep(2000);
	cout << "看起来还不错，是个艺术馆吗" << endl;
	Sleep(2000);
	cout << "怎么这么晚了还开门" << endl;
	Sleep(2000);
	cout << "看不见人影啊..." << endl;
	Sleep(2000);
	cout << "有人吗--" << endl;
	Sleep(5000);
	cout << "有";
	Sleep(1000);
	cout << "人";
	Sleep(1000);
	cout << "吗----------" << endl;
	Sleep(3000);
	cout << "咦，门怎么消失了...." << endl;
	Sleep(2000);
	cout << "感觉不太对劲，我得赶快想办法离开这里...." << endl;
	Sleep(2000);
	cout << "使用WSAD移动，QE旋转，Z来恢复正常视角,ESC退出 " << endl;
	Sleep(5000);
	cout << "请将控制台拖至可见位置，以防漏掉提示信息：） " << endl;
	Sleep(5000);
}
void KeyProcess(unsigned char key, int xx, int yy)
{
	double angle_C = angle / 180 * PI;
	mark = 0;
	switch (key)
	{
	case 90:
	case 122:
		y = manHeight;
		break;
	case 27:
		glutDestroyWindow(1);
		exit(0);
		break;
	case 81:
	case 113:
		angle += 5;
		//cout << "q" << endl;
		break;
	case 69:
	case 101:
		angle -= 5;
		//cout << "e" << endl;
		break;
	case 87:
	case 119: //forward

		tmpX = x;
		tmpZ = z;
		tmpX += speed * cos(angle_C);
		tmpZ -= speed * sin(angle_C);

		judgeEdge();
		//cout << x << " " << z << endl;
		if (mark)
		{
			x = tmpX;
			z = tmpZ;
		}
		break;
	case 83:
	case 115: //backward

		tmpX = x;
		tmpZ = z;
		tmpX -= speed * cos(angle_C);
		tmpZ += speed * sin(angle_C);

		judgeEdge();

		if (mark)
		{
			x = tmpX;
			z = tmpZ;
		}
		break;
	case 65:
	case 97: //left
		tmpX = x;
		tmpZ = z;
		tmpX -= speed * sin(angle_C);
		tmpZ -= speed * cos(angle_C);
		judgeEdge();
		if (tmpX > -3154 && tmpX < -3146 && tmpZ > -300 && tmpZ < -100)
			mark = 1;
		//cout << x << " " << z << endl;

		if (mark)
		{
			x = tmpX;
			z = tmpZ;
		}
		break;
	case 68:
	case 100: //right
		tmpX = x;
		tmpZ = z;
		tmpX += speed * sin(angle_C);
		tmpZ += speed * cos(angle_C);
		judgeEdge();
		if (tmpX > -3154 && tmpX < -3146 && tmpZ > -300 && tmpZ < -100)
			mark = 1;
		//cout << x << " " << z << endl;

		if (mark)
		{
			x = tmpX;
			z = tmpZ;
		}
		break;
	}
	Disp();
}
void SKey(int key, int xx, int yy)
{
	double angle_C = angle / 180 * PI;
	int mark = 0;

	switch (key)
	{
	case GLUT_KEY_UP:
	{

		z -= speed * sin(angle_C);
		x += speed * cos(angle_C);

		break;
	}
	case GLUT_KEY_DOWN:
	{
		z += speed * sin(angle_C);
		x -= speed * cos(angle_C);

		break;
	}
	case GLUT_KEY_LEFT:
	{
		x -= speed * sin(angle_C);
		z -= speed * cos(angle_C);

		break;
	}
	case GLUT_KEY_RIGHT:
	{
		x += speed * sin(angle_C);
		z += speed * cos(angle_C);

		break;
	}
	}
	Disp();
}
int cur = 0;
int loc[100];
int loc2[100];
void PassiveMouseMove(int xx, int yy)
{
	loc2[cur] = yy;
	loc[cur++] = xx;
	if (cur >= 2)
	{
		//cout << loc[cur - 1] - loc[cur - 2] << endl;
		angle -= (loc[cur - 1] - loc[cur - 2]) / 3;
		y -= (loc2[cur - 1] - loc2[cur - 2]) / 5;
		if (y < 0)
			y = 0;
		else if (y > 120)
			y = 120;
	}
	if (cur == 99)
		cur = 0;
	Disp();
}
void ActiveMouseMove(int xx, int yy)
{
	PassiveMouseMove(xx, yy);
	if (pick && tmpX > -4150 && tmpX < -4000 && !drawLine && int(fabs(angle)) % 360 > 150 && int(fabs(angle)) % 360 < 210)
	{
		//cout << "draw1" << endl;
		drawLineLoc = z;
		drawLine = 1;
		cout << "我随便画了几笔，笔便再也不下水了" << endl;
		Sleep(2000);
	}
	else if (pick && tmpX > -4150 && tmpX < -4000 && !drawLine && (int(fabs(angle)) % 360 > 330 || int(fabs(angle)) % 360 < 30))
	{
		//cout << "draw2" << endl;
		drawLineLoc = z;
		drawLine = 2;
		cout << "我往墙上画了两下，希望没把画给糟蹋了" << endl;
		Sleep(1000);
	}
	else if (pick && !drawLine)
	{
		cout << "这里好像没法写字，我还是省省吧" << endl;
		Sleep(1000);
	}

	//cout << int(fabs(angle)) % 360 << endl;

	Disp();
}
void main(void)
{
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(400, 20);
	glutInitWindowSize(1000, 600);
	glutCreateWindow("Ghost Gallery");
	Init();
	PlaySound(L"1.wav", NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	info();
	//glutFullScreen();
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Disp);
	glutKeyboardFunc(KeyProcess);
	glutSpecialFunc(SKey);
	glutMouseFunc(getMouse);
	glutPassiveMotionFunc(PassiveMouseMove);
	glutMotionFunc(ActiveMouseMove);
	glutTimerFunc(10, TimerFun, 1);
	glutTimerFunc(1000, timeCount, 2);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);

	glutMainLoop();
}
