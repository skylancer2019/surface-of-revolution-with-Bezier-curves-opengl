
#include <Windows.h>
#include <GL/glut.h>  // GLUT, include glu.h and gl.h
#include <vector>
#include <math.h>      

#define PI 3.14159265

double wwidth;
double hheight;
float mouseX, mouseY;
bool mouseRightDown;

float cameraAngleX;
float cameraAngleY;
bool mouseDown = false;
bool drawRevo = false;
double loopradius =0.1;//旋转面的取点间隔
double  BezierGap = 0.05;//贝塞尔曲线取点间隔

class Point2
{
public:
    double x, y;
    Point2(double _x, double _y) {
        x = _x;
        y = _y;
    }

	Point2() {}
};
class Point3
{
public:
	double x, y, z;
	Point3(double _x, double _y, double _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	Point3(Point2 bef)
	{
		x = bef.x;
		y = bef.y;
		z = 0;
	}
	Point3() {}
};
struct face
{
	Point3 a;
	Point3 b;
	Point3 c;
	face(Point3 _a, Point3 _b, Point3 _c) :a(_a), b(_b), c(_c) {}
};


bool drawBe = false;
bool drawPoint = false;

std::vector<Point3> BezierPoint;//贝塞尔曲线上的点
std::vector<Point3> BezierNormal;//法向量
std::vector<face> faces;//旋转曲面上的三角形面片
std::vector<Point2> points;//从鼠标处获取点的位置与顺序
void DrawPoint()
{
    glColor3f(0.0, 1.0, 0.0);
glBegin(GL_POINTS);
    for (int i = 0; i < points.size(); i++)
    {
        
        glVertex2d(points[i].x, points[i].y);
       
    }
 glEnd();

    glColor3f(1.0, 0.0, 0.0);
glBegin(GL_LINES);
    for (int i = 0; i < points.size() - 1&&!points.empty(); i++)
    {
        
        glVertex2d(points[i].x, points[i].y);
        glVertex2d(points[i + 1].x, points[i + 1].y);
      
    }
   glEnd();  

}
void init()
{
	GLfloat mat_specular[] = { 1.0,1.0,1.0,1.0 };
	GLfloat mat_shininess[] = { 10.0 };
	GLfloat light_position[] = { 1000.0, 1000.0, 1000.0, 1.0 };
	GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };   
	GLfloat Light_Model_Ambient[] = { 0.2, 0.2, 0.2, 1.0 }; //环境光
	glClearColor(0.0, 0.0, 0.0, 0.0);  //背景色
	glShadeModel(GL_SMOOTH);         
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);   //散射光属性
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);  //镜面反射光
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, Light_Model_Ambient);  //环境光参数

	glEnable(GL_LIGHTING);  
	glEnable(GL_LIGHT0);     
	glEnable(GL_DEPTH_TEST); 
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//	glPolygonMode(GL_BACK, GL_LINE);

	
}
Point2 GetBeZier(Point2 p0, Point2 p1, Point2 p2, Point2 p3, double t)
{
	double x;
	x = p0.x + (-3 * p0.x + 3 * p1.x) * t + (3 * p0.x - 6 * p1.x + 3 * p2.x) * t * t + (-p0.x + 3 * p1.x - 3 * p2.x + p3.x) * t * t * t;
	double y;
	y = p0.y + (-3 * p0.y + 3 * p1.y) * t + (3 * p0.y - 6 * p1.y + 3 * p2.y) * t * t + (-p0.y + 3 * p1.y - 3 * p2.y + p3.y) * t * t * t;
	return Point2(x, y);

}

void DrawBezier()
{
	int i = 0;
	Point2 preP;
	glBegin(GL_LINES);
	while (i < points.size())
	{
		
		Point2 p0 = points[i];preP = p0;
		i++;
		if (i >= points.size())
		{
		
			break;
		}
		Point2 p1 = points[i];
		i++;
		if (i >= points.size())
		{
			break;
		}
		Point2 p2 = points[i];
		i++;
		
		if (i >= points.size())
		{
			break;
		}
		else
		{
			Point2 p3 = points[i];
			for (double t = 0.0; t<=1.0;t += BezierGap)
			{
				Point2 d = GetBeZier(p0, p1, p2, p3, t);
					Point3 temp(d);
					BezierPoint.push_back(temp);//存储旋转曲线上的点
				glVertex2d(preP.x, preP.y);
				glVertex2d(d.x, d.y);
				preP = d;

			}
			glVertex2d(preP.x, preP.y);
			glVertex2d(p3.x, p3.y);
		}
	}
	glEnd();
}
//假设cur是在XOY平面上的
Point3 GetCoordi(Point3 cur, double radiu)
{
	Point3 newmake;
	newmake.y = cur.y;
	newmake.x = cur.x*cos(radiu);
	newmake.z = -cur.x * sin(radiu);
	return newmake;
}

void DrawRevolution(double radiu)//BezierPoint
{
	/*
	要将绘制的曲线矫正到y轴上
	*/
	bool Up_to_down = false;
	if (BezierPoint[0].y > BezierPoint[BezierPoint.size() - 1].y)//从上到下
	{
		Up_to_down = true;
	}

	double head = BezierPoint[0].x;
	int lastindex = BezierPoint.size() - 1;
	double tail = BezierPoint[lastindex].x;
	bool headr = false;
	double change;
	if (head > tail)
	{
		headr = true;
		change = 0 - head;
		BezierPoint[lastindex].x= BezierPoint[0].x;//使得曲线的首尾x值相同
	}
	else
	{
		change = 0 - tail;
		BezierPoint[0].x = BezierPoint[lastindex].x;
	}
	//修正到y轴上
	for (int i = 0; i < BezierPoint.size(); i++)
	{	
		BezierPoint[i].x += change;
	}
	double preR = 0;//上一列转动的角度
	double curR = radiu;//当前转动的角度
	bool breakflag = false;
	//循环转动绘制面片
	while (true)
	{//i=0的点在轴上，不会动
		if (2 * PI - curR <0.001)
		{
			curR = 2*PI;
			breakflag = true;
		}
		int i = 1;//代表多少行向上
		for (i = 1; i <= lastindex-1; i++)
		{
			Point3 prem = GetCoordi(BezierPoint[i], preR);
			Point3 newm = GetCoordi(BezierPoint[i], curR);
			Point3 LeftDown = GetCoordi(BezierPoint[i+1], preR);
			Point3 RightUp = GetCoordi(BezierPoint[i - 1],curR);

			if (Up_to_down)
			{
				faces.push_back(face(prem, newm, RightUp));
				faces.push_back(face(newm, prem, LeftDown));
			}
			else
			{
				faces.push_back(face(RightUp, newm, prem));
				faces.push_back(face(prem, newm, LeftDown));
			}

		}
		preR = curR;
		curR += radiu;
		if (breakflag)
		{
			break;
		}
	}


}
void draw()
{
	glBegin(GL_TRIANGLES);
	
	//glColor3f(0.0, 0.0, 0.0);
	//glColor3f(0.0, 0.749, 1.0);
	
	for (int i = 0; i < faces.size(); i++) {
		
		face cur = faces[i];
		Point3 u(cur.b.x - cur.a.x, cur.b.y - cur.a.y, cur.b.z - cur.a.z);
		Point3 v(cur.c.x - cur.b.x, cur.c.y - cur.b.y, cur.c.z - cur.b.z);
		double xn = u.y * v.z - u.z * v.y;
		double yn = u.z * v.x - u.x * v.z;
		double zn = u.x * v.y - u.y * v.x;
		//glNormal3f(xn, yn, zn);
		glVertex3f(cur.a.x,cur.a.y,cur.a.z);
		
		//glNormal3f(xn, yn, zn);
		glVertex3f(cur.b.x, cur.b.y, cur.b.z);
		
		//glNormal3f(xn, yn, zn);
		glVertex3f(cur.c.x, cur.c.y, cur.c.z);
		
	}


	glEnd();

}

void KeyBoards(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		drawBe = true;//绘制bezier曲线
		glutPostRedisplay();
		break;
	case 'r'://清空当前画的曲线
		drawBe = false;
		drawRevo = false;
		points.clear();
		BezierPoint.clear();
		faces.clear();
		glDisable(GL_LIGHTING);
		glShadeModel(GL_2D);
		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, wwidth, hheight);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, 1000.0, 0.0, 600.0);
		glutPostRedisplay();
		break;
	case 'e'://绘制revolution曲面
		drawRevo = true;
		DrawRevolution(loopradius);
		
		glutPostRedisplay();


	}
}
void display()
{
 	if (drawRevo)
	{	init();
		glViewport(0, 0, wwidth, hheight);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-500.0, 500.0, 0.0, 600, 500.0, -500);
	//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	
	if (drawRevo)
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_MODELVIEW);
		
		glRotatef(cameraAngleX, 1, 0, 0);
		glRotatef(cameraAngleY, 0, 1, 0);
		draw();
		//glutSolidTeapot(200);
	}
	else
	{
		glLoadIdentity();
		glColor3d(1.0, 0.0, 0.0);

		if (mouseDown)
		{
			DrawPoint();
			mouseDown = false;
		}
		if (drawBe)
		{
			DrawBezier();
		}
	}
	glutSwapBuffers();
	//angle += 0.02f;

}
void MouseMotion(int x, int y)
{
	cameraAngleX = cameraAngleY = 0;
	//mouseLeftDown = false;
	if (mouseRightDown)
	{
		cameraAngleY += (x - mouseX) * 0.1f;
		cameraAngleX += (y - mouseY) * 0.1f;
		mouseX = x;
		mouseY = y;
		glutPostRedisplay();
	}

}
void reshape(GLsizei width, GLsizei height)
{
	if (height == 0)height = 1;
	float size = 2;
	GLfloat aspect = (GLfloat)width / (GLfloat)height;
	glViewport(0, 0, width, height);
	wwidth = width; hheight = height;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (drawRevo)
	{
		glOrtho(-1000.0,1000.0,0.0,600,1000.0,-1000);

	}
	else
	{
		gluOrtho2D(0.0, 1000.0, 0.0, 600.0);
	}

}

void MouseFunc(int button, int state, int x, int y)
{

	mouseX = x;
	mouseY = y; mouseRightDown = false;
		if (state == GLUT_DOWN)
		{
			if (button == GLUT_RIGHT_BUTTON)
			{
				mouseRightDown = true;
			}
			else
			{
				mouseDown = true;
				points.push_back(Point2((double)x, (double)(600 - y)));
				glutPostRedisplay();
			}
			
		}
		else
		{
			if (state == GLUT_UP)
			{
				if (button == GLUT_RIGHT_BUTTON)
				{
					mouseRightDown = false;
				}
		//		mouseDown = false;
			}
		}

}



int main(int argc, char** argv)
{
	
	glutInit(&argc, argv);

	glutInitWindowSize(1000, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("OpenGL test1");
	
	
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glPointSize(5);
	
	
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MouseMotion);
	glutKeyboardFunc(KeyBoards);
	glutMainLoop();
	return 0;

}






