#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <iostream>
#include <vector>

using namespace std;

float minX, minY, maxX, maxY;
float X1, Y1, X2, Y2;

float p[100][2];
int n, m;
bool flag = false;

// 顶点缓冲对象
unsigned int VBO[3], VAO[3];
unsigned int shaderProgram, shaderProgram2, shaderProgram3;

vector<float> arr;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


// blue
const char *fragmentShaderSource=
	"#version 330\n"
	"in vec3 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 0.0, 1.0, 1);\n"
	"}";

// color
const char *fragmentShader2Source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";

// green
const char *fragmentShader3Source=
	"#version 330\n"
	"in vec3 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 1);\n"
	"}";

// 获取窗口坐标
void getWindowsPosition(){
	arr.push_back(minX*0.001);
	arr.push_back(minY*0.001);
	arr.push_back(minX*0.001);
	arr.push_back(maxY*0.001);
	arr.push_back(maxX*0.001);
	arr.push_back(maxY*0.001);
	arr.push_back(maxX*0.001);
	arr.push_back(minY*0.001);
}

// 添加输入线段点的坐标
void getLinePoint(){
	arr.push_back(X1*0.001);
	arr.push_back(Y1*0.001);
	arr.push_back(X2*0.001);
	arr.push_back(Y2*0.001);
}

int encode(float x, float y){
	int code = 0;
	// 窗口左侧 0001
	if(x < minX) 
		code = 1;
	// 窗口右侧 0010
	else if (x > maxX)
		code = 2;
	// 窗口下侧 + 0100
	if (y < minY)
		code +=4;
	// 窗口上侧 + 1000
	else if(y > maxY)
		code+=8;
	return code;
}

void CohenSutherland(float x1, float y1, float x2, float y2){
	getLinePoint();
	int code, code1, code2;
	float x, y;
	// 获取两个端点的编码
	code1 = encode(x1, y1);
	code2 = encode(x2, y2);
	// 都为0则都在窗口内
	while(code1 != 0 || code2 != 0){
		if((code1&code2)!=0) break;
		code = code1;
		if(code1 == 0)
			code = code2;
		if((code & 1) == 1){
			y = y1 + (y2 - y1)*(minX-x1)/(x2-x1);
			x = minX;
		} else if((code & 2) == 2){
			y = y1 + (y2 - y1)*(maxX-x1)/(x2-x1);
			x = maxX;
		} else if((code & 4) == 4){
			x = x1 + (x2 - x1)*(minY-y1)/(y2-y1);
			y = minY;
		} else if ((code&8) == 8) {
			x = x1 + (x2 - x1)*(maxY-y1)/(y2-y1);
			y = maxY;
		}

		if(code == code1){
			x1 = x;
			y1 = y;
			code1 = encode(x, y);
		} else {
			x2 = x;
			y2 = y;
			code2 = encode(x, y);
		}
	}
	// cout<<x1<<"\t"<<y1<<"\t"<<x2<<"\t"<<y2<<endl;
	arr.push_back(x1*0.001);
	arr.push_back(y1*0.001);
	arr.push_back(x2*0.001);
	arr.push_back(y2*0.001);
}

bool Clip(float p, float q, float &maxu, float &minu){
	float r = 0;
	if(p < 0){
		r = q/p;
		if(r > minu){
			return false;
		}
		if (r > maxu){
			maxu = r;
		}
	} else if(p > 0){
		r = q/p;
		if(r < maxu) {
			return false;
		} 
		if (r < minu){
			minu = r;
		}
	} else {
		return q >= 0;
	}
	return true;
}

void LiangBarsky(float x1, float y1, float x2, float y2){
	getLinePoint();
	float deltax = x2-x1, deltay = y2-y1;
	float umax = 0, umin = 1;
	if(Clip(-deltax, x1 - minX, umax, umin)){
		if(Clip(deltax, maxX - x1, umax, umin)){
			if(Clip(-deltay, y1 - minY, umax, umin)){
				if(Clip(deltay, maxY - y1, umax, umin)){
					x1 = x1 + umax * deltax;
					y1 = y1 + umax * deltay;
					x2 = x1 + umin * deltax;
					y2 = y1 + umin * deltay;
				}
				arr.push_back(x1*0.001);
				arr.push_back(y1*0.001);
				arr.push_back(x2*0.001);
				arr.push_back(y2*0.001);
			}
		}
	}
}

void initPolygon(){
	cout<<"输入多边形点的个数:";
    cin>>n;
	m = n;
	cout<<"请输入各个点的坐标:"<<endl;
    for(int i = 0; i < n; i++){
        cin>>p[i][0]>>p[i][1];
		arr.push_back(p[i][0]*0.001);
		arr.push_back(p[i][1]*0.001);
    }
}

// 返回交点横坐标
int intersectX(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4){ 
    int m = (x1*y2 - y1*x2) * (x3-x4) - (x1-x2) * (x3*y4 - y3*x4); 
    int n = (x1-x2) * (y3-y4) - (y1-y2) * (x3-x4); 
    return m/n; 
}  
//返回交点纵坐标
int intersectY(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4) { 
    int m = (x1*y2 - y1*x2) * (y3-y4) - (y1-y2) * (x3*y4 - y3*x4); 
    int n = (x1-x2) * (y3-y4) - (y1-y2) * (x3-x4); 
    return m/n; 
} 

void SutherClip(float points[][2], int &num, float x1, float y1, float x2, float y2){
	float result[100][2];
	int size = 0;
	for(int i = 0; i < num; i++){
		int k = (i+1)%num;
		float ix = points[i][0], iy = points[i][1];
		float kx = points[k][0], ky = points[k][1];
		int flag_i = (x2-x1) * (iy-y1) - (y2-y1) * (ix-x1);
		int flag_k = (x2-x1) * (ky-y1) - (y2-y1) * (kx-x1);
		if((flag_i < 0) && (flag_k < 0)){
			result[size][0] = kx;
			result[size][1] = ky;
			size++;
		} else if ((flag_i >= 0) && (flag_k < 0)){
			result[size][0] = intersectX(x1, y1, x2, y2, ix, iy, kx, ky);
			result[size][1] = intersectY(x1, y1, x2, y2, ix, iy, kx, ky);
			size++;
			result[size][0] = kx;
			result[size][1] = ky;
			size++;
		} else if((flag_i < 0) && (flag_k >= 0)){
			result[size][0] = intersectX(x1, y1, x2, y2, ix, iy, kx, ky);
			result[size][1] = intersectY(x1, y1, x2, y2, ix, iy, kx, ky);
			size++;
		}
	}
	num = size;
	for(int i = 0; i < num; i++){
		points[i][0] = result[i][0];
		points[i][1] = result[i][1];
	}
}

void SutherlandHodgeman(float points[][2], int size){
	for(int i = 0; i < 4; i++){
		int k = (i+1)%4;
		SutherClip(points, size, arr[2*i]*1000,arr[2*i+1]*1000,arr[2*k]*1000,arr[2*k+1]*1000);
	}
	m = size;
	for(int i = 0; i < size; i++){
		// cout<<"("<<points[i][0]<<","<<points[i][1]<<")"<<endl;
		arr.push_back(points[i][0]*0.001);
		arr.push_back(points[i][1]*0.001);
	}
}

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

	unsigned int fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader2, 1, &fragmentShader2Source, NULL);
    glCompileShader(fragmentShader2);

	unsigned int fragmentShader3 = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader3, 1, &fragmentShader3Source, NULL);
    glCompileShader(fragmentShader3);

    // 着色，将多个着色器合并之后并最终链接完成的版本
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

	shaderProgram2 = glCreateProgram();
	glAttachShader(shaderProgram2, vertexShader);
    glAttachShader(shaderProgram2, fragmentShader2);
    glLinkProgram(shaderProgram2);

	shaderProgram3 = glCreateProgram();
	glAttachShader(shaderProgram3, vertexShader);
    glAttachShader(shaderProgram3, fragmentShader3);
    glLinkProgram(shaderProgram3);
    
    // 删除着色器对象
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void bind_data(){
	// 窗口坐标数组
	int len = arr.size();
	float myWindows[len];
	for(int i = 0; i < len; i++){
		myWindows[i] = arr[i];
	}
	// for(int i = 0; i < len; i++){
	// 	cout<<myWindows[i]<<"\t";
	// }

	glGenVertexArrays(3, VAO);
	glGenBuffers(3, VBO);
	glBindVertexArray(VAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myWindows), myWindows, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myWindows), myWindows, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myWindows), myWindows, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int main(){
	cout<<"输入窗口左下角与右上角坐标:";
	cin>>minX>>minY>>maxX>>maxY;
	getWindowsPosition();
	int choose;
	cout<<"选择裁剪算法:"<<endl;
	cout<<"1.CohenSutherland线段裁剪"<<endl;
	cout<<"2.LiangBarsky线段裁剪"<<endl;
	cout<<"3.SutherlandHodgeman多边形裁剪(请按顺时针方向输入点的坐标)"<<endl;
	cin>>choose;
	switch(choose){
		case 1:
		case 2:
			flag = false;
			break;
		case 3:
			flag = true;
			break;
		default:
			choose = 3;
			cout<<"多边形裁剪算法"<<endl;
	}
	if(flag){
		initPolygon();
		SutherlandHodgeman(p, m);
	} else {
		cout<<"输入线段的左右端点坐标：";
		cin>>X1>>Y1>>X2>>Y2;
		if(choose==1){
			CohenSutherland(X1, Y1, X2, Y2);
		}
		LiangBarsky(X1, Y1, X2, Y2);
	}
	
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
		
		// 窗口
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO[0]);
		glDrawArrays(GL_LINE_LOOP, 0, 4); 
		
		if(flag){
			// 绘制多边形
			glUseProgram(shaderProgram2);
			glBindVertexArray(VAO[1]);
			glDrawArrays(GL_LINE_LOOP, 4, n);

			// 绘制裁剪后的多边形
			glUseProgram(shaderProgram3);
			glBindVertexArray(VAO[2]);
			glDrawArrays(GL_LINE_LOOP, 4+n, m);
		} else {
			// 绘制裁剪线段
			glUseProgram(shaderProgram2);
			glBindVertexArray(VAO[1]);
			glDrawArrays(GL_LINES, 4, 2);
			// 绘制裁剪后的线段
			glUseProgram(shaderProgram);
			glBindVertexArray(VAO[0]);
			glDrawArrays(GL_LINES, 6, 2);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(2, VAO);
	glDeleteBuffers(2,VBO);
	glDeleteProgram(shaderProgram);
	glDeleteProgram(shaderProgram2);
	glfwTerminate();
    return 0;
}