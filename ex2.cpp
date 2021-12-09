#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <math.h>
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

vector<float> arr;
int n = 0;
int X1, Y1, X2, Y2, X3, Y3;
// point (x, y, z)
float vertices[]={};

void DDA(){
	cin>>X1>>Y1>>X2>>Y2;
	int dx, dy, steps;
	dy = Y2-Y1;
	dx = X2-X1;
	steps = max(abs(dx), abs(dy));
	float x, y, _x, _y;
	_x = (float)dx/steps;
	_y = (float)dy/steps;
	x =X1, y= Y1;
	n = steps;
	for(int i = 1; i <= steps; i++){
		arr.push_back(x*0.001);
		arr.push_back(y*0.001);
		x+=_x;
		y+=_y;
	}
}

void Bresenham(){
	cin>>X1>>Y1>>X2>>Y2;
	int x, y, dx, dy, d;
	y = Y1;
	dx = X2-X1;
	dy = Y2-Y1;
	d = 2*dy-dx; // init d
	n = dx;
	for(x = X1; x <= X2; x++){
		arr.push_back(x*0.001);
		arr.push_back(y*0.001);
		if(d < 0){
			d += 2*dy;
		} else {
			y++;
			d+=2*dy-2*dx;
		}
	}
}

int Max(int x, int y, int z){
	int max;
	max = x>y?x:y;
	max = max>z?max:z;
	return max;
}

int Min(int x, int y, int z){
	int min;
	min = x<y?x:y;
	min = min<z?min:z;
	return min;
}


bool inside(int x, int y){
	// 设M(x,y) 分别求向量AB,BC, CA, AM, BM, CM
	int ABx = X2-X1, ABy = Y2-Y1;
	int BCx = X3-X2, BCy = Y3-Y2;
	int CAx = X1-X3, CAy = Y1-Y3;
	int AMx = x-X1, AMy = y-Y1;
	int BMx = x-X2, BMy = y-Y2;
	int CMx = x-X3, CMy = y-Y3;
	// 叉乘同号  x1y2-x2y1
	int a = ABx*AMy - AMx*ABy;
	int b = BCx*BMy - BMx*BCy;
	int c = CAx*CMy - CMx*CAy;
	return (a>0&&b>0&&c>0)||(a<0&&b<0&&c<0);
}

void Tri(){
	cin>>X1>>Y1>>X2>>Y2>>X3>>Y3;
	int maxx = Max(X1, X2, X3);
	int maxy = Max(Y1, Y2, Y3);
	int minx = Min(X1, X2, X3);
	int miny = Min(Y1, Y2, Y3);
	for(int y = miny; y < maxy; y++){
		for(int x = minx; x < maxx; x++){
			if(inside(x, y)){
				arr.push_back(x*0.001);
				arr.push_back(y*0.001);
				n++;
			}
		}
	}
}

// 顶点缓冲对象
unsigned int VBO, VAO, EBO;
unsigned int shaderProgram;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource="#version 330\n"
	"in vec3 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 0.0, 1.0, 1);\n"
	"}\0";

void reshape_size(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

// ESC退出
void processInput(GLFWwindow* window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
	}
}

void shaders(){
    // 顶点着色器处理
    unsigned int vertexShader= glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // 片段着色器
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // 着色，将多个着色器合并之后并最终链接完成的版本
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // 删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void bind_data(){
	int len = arr.size();
	float vertices[len];
	for(int i = 0; i < len; i++){
		vertices[i] = arr[i];
	}
	/// 1. 绑定VAO
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// glGenBuffers(1, &EBO); // 添加EBO
	glBindVertexArray(VAO);
	// 2. 把顶点数组复制到缓冲中供OpenGL使用
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); //
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int main(){
	cout<<"input data:";
	// DDA();
	// Bresenham();
	Tri();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL){
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, reshape_size);
	
	

    // 使用该shaderprogram
	shaders();
	bind_data();

	while(!glfwWindowShouldClose(window)){
		processInput(window);
		// bgcolor 白色
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);

        // draw
		glDrawArrays(GL_POINTS, 0, n);// 类型，起始索引，个数

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1,&VBO);
	glDeleteProgram(shaderProgram);
	glfwTerminate();
	return 0;
}