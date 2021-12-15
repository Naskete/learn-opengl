#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <iostream>
#include <vector>

using namespace std;

float minX, minY, maxX, maxY;
float X1, Y1, X2, Y2;

// 顶点缓冲对象
unsigned int VBO, VAO, EBO;
unsigned int shaderProgram;

vector<float> arr;

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


const char *fragmentShaderSource=
"#version 330\n"
"in vec3 color;\n"
"\n"
"void main()\n"
"{\n"
"	gl_FragColor = vec4(0.0, 0.0, 1.0, 1);\n"
"}";

// 获取窗口坐标
void getWindowsPosition(){
	arr.push_back(minX*0.001);
	arr.push_back(minY*0.001);
	arr.push_back(maxX*0.001);
	arr.push_back(minY*0.001);
	arr.push_back(maxX*0.001);
	arr.push_back(maxY*0.001);
	arr.push_back(minX*0.001);
	arr.push_back(maxY*0.001);
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

void CohenSutherland(float x0, float y0, float x2, float y2){
	int code, code1, code2;
	float x, y;
	// 获取两个端点的编码
	code1 = encode(x0, y0);
	code2 = encode(x2, y2);
	// 都为0则都在窗口内
	while(code1 != 0 || code2 != 0){
		if((code1&code2)!=0) break;
		code = code1;
		if(code1 == 0)
			code = code2;
		if((code & 1) == 1){
			y = y0 + (y2 - y0)*(minX-x0)/(x2-x0);
			x = minX;
		} else if((code & 2) == 2){
			y = y0 + (y2 - y0)*(maxX-x0)/(x2-x0);
			x = maxX;
		} else if((code & 4) == 4){
			x = x0 + (x2 - x0)*(minY-y0)/(y2-y0);
			y = minY;
		} else if ((code&8) == 8) {
			x = x0 + (x2 - x0)*(maxY-y0)/(y2-y0);
			y = maxY;
		}

		if(code == code1){
			x0 = x;
			y0 = y;
			code1 = encode(x, y);
		} else {
			x2 = x;
			y2 = y;
			code2 = encode(x, y);
		}
	}
	cout<<x0<<"\t"<<y0<<"\t"<<x2<<"\t"<<y2<<endl;
	arr.push_back(x0*0.001);
	arr.push_back(y0*0.001);
	arr.push_back(x2*0.001);
	arr.push_back(y2*0.001);
}

void LiangBarsky(float x0, float y0, float x2, float y2){
	float deltax = x2-x0, deltay = y2-y0;
	float p[4], q[4];
	p[0] = -deltax, p[1] = deltax, p[2] = -deltay, p[3] = deltay;
	q[0] = x0 - minX, q[1] = maxX - x0, q[2] = y0 - minY, q[3] = maxY - y0;
	// 平行x轴
	if(deltax==0){
		if(q[0] < 0 || q[1] <0){
			// not in
			cout<<"not in windows"<<endl;
			return ;
		} else {
			// 计算
			
		}
	} else if(deltay == 0){
		// 平行y轴
		if(q[2] < 0 || q[3] < 0){
			cout<<"not in windows"<<endl;
			return ;
		}
	} else {

	}

}

void SutherlandHodgeman(){

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
	// 窗口坐标数组
	float myWindows[12];
	for(int i = 0; i < 12; i++){
		myWindows[i] = arr[i];
	}
	for(int i = 0; i < 12; i++){
		cout<<myWindows[i]<<"\t";
	}

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
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
	cout<<"输入线段的左右端点坐标：";
	cin>>X1>>Y1>>X2>>Y2;
	CohenSutherland(X1, Y1, X2, Y2);
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

		// 窗口
		glDrawArrays(GL_LINE_LOOP, 0, 4); 
		glDrawArrays(GL_LINES, 4, 2);
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1,&VBO);
	glDeleteProgram(shaderProgram);
	glfwTerminate();
    return 0;
}