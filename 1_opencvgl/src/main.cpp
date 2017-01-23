#include <iostream>
#include <opencv2/highgui.hpp>
#include <GLFW/glfw3.h>
#include "linmath.h"

using namespace std;
using namespace cv;

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define TRACE  __FILE__ "\nLine:" TOSTRING(__LINE__)  "\n"
#define TRACE_STR(str) str "\n" TRACE

#define GLCHECK {int error = glGetError(); \
if (error != 0) { \
printf("ErrorGL: %i\n", error); \
printf(TRACE); \
return 1; \
}}

const static char* shaderV_glsl = ""
"// Vertex Shader \n"
"attribute vec3 pos; \n"
"attribute vec2 uv; \n"
"varying vec2 tex_uv; \n"
" \n"
"void main() \n"
"{ \n"
"	gl_Position = vec4(pos, 1); \n"
"	tex_uv = uv; \n"
"} \n";

const static char* shaderF_glsl = "// fragment shader\n "
#ifdef __ANDROID__
"	precision mediump float;\n "
#endif
"uniform sampler2D tex;\n "
"varying vec2 tex_uv;\n "
"\n "
"void main()\n "
"{\n "
"	//gl_FragColor = vec4(1.0,1.0,1.0,1.0);\n "
"	gl_FragColor = texture2D(tex, tex_uv).bgra;\n "
"	//gl_FragColor.a = 1.0;\n "
"}";




static const struct
{
	float x, y;
	float r, g, b;
} vertices[3] =
{
	{ -0.6f, -0.4f, 1.f, 0.f, 0.f },
	{  0.6f, -0.4f, 0.f, 1.f, 0.f },
	{   0.f,  0.6f, 0.f, 0.f, 1.f }
};
static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
static const char* fragment_shader_text =
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main(int argc, char* argv[])
{
	std::shared_ptr<float> pVertices;
	std::shared_ptr<float> pUV;
	GLuint m_texture;

	//initialize rectangle
	float left = -1;
	float top = 1;
	float w = 2;
	float h = 2;
	float z = 0.5f;
	pVertices = std::shared_ptr<float>(new float[12]{
		left, top - h, z,
		left, top, z,
		left + w, top - h, z,
		left + w, top, z

	}, std::default_delete<float[]>());
	pUV = std::shared_ptr<float>(new float[9]{
		0.f, 1.f,
		0.f, 0.f,
		1.f, 1.f,
		1.f, 0.f

	}, std::default_delete<float[]>());

	Mat img(480, 640, CV_8UC3, Scalar::all(0));
	Mat img2(300, 300, CV_8UC3, Scalar(0, 0, 255));
	img2.copyTo(img(Rect(Point(50, 50), img2.size())));

	GLFWwindow* window;
	GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	GLint mvp_location, vpos_location, vcol_location;

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		printf("glfw not initialized\n");
		return 1;
	}

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

//	glGenBuffers(1, &vertex_buffer);
//	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
//	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//Initialize texture
	int error;
	glGenTextures(1, &m_texture);
	if (!m_texture)
	{
		printf("Error creating texture\n");
		return false;
	}
	const int type = GL_RGB;
	glActiveTexture(GL_TEXTURE0);
	GLCHECK
	glBindTexture(GL_TEXTURE_2D, m_texture);
	GLCHECK
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	GLCHECK
	glTexImage2D(GL_TEXTURE_2D, 0, type, img.cols, img.rows, 0, type, GL_UNSIGNED_BYTE, img.ptr<uchar>(0));
	GLCHECK
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	GLCHECK
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLCHECK
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	GLCHECK
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	GLCHECK

	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	GLCHECK
	glShaderSource(vertex_shader, 1, &shaderV_glsl, NULL);
	GLCHECK
	glCompileShader(vertex_shader);
	GLCHECK
	GLint status;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		printf("Error vertex shader\n");
		return 1;
	}
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	GLCHECK
	glShaderSource(fragment_shader, 1, &shaderF_glsl, NULL);
	GLCHECK
	glCompileShader(fragment_shader);
	GLCHECK
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE)
	{
		printf("Error fragment shader\n");
		return 1;
	}
	program = glCreateProgram();
	GLCHECK
	glAttachShader(program, vertex_shader);
	GLCHECK
	glAttachShader(program, fragment_shader);
	GLCHECK
	glLinkProgram(program);
	GLCHECK
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE)
	{
		printf("Unable to link shader\n");
		return 1;
	}
	glValidateProgram(program);
	GLCHECK
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);
	if (status != GL_TRUE)
	{
		printf("Unable to validate shader\n");
		return 1;
	}

	printf("Program: %d\n", program);


	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
//		mat4x4 m, p, mvp;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float) height;
		glViewport(0, 0, width, height);
//		glClear(GL_COLOR_BUFFER_BIT);
//		mat4x4_identity(m);
//		mat4x4_rotate_Z(m, m, (float) glfwGetTime());
//		mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
//		mat4x4_mul(mvp, p, m);
//		glUseProgram(program);
//		glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
//		glDrawArrays(GL_TRIANGLES, 0, 3);

		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		GLCHECK
		glClearColor(0,0,0,1);
		GLCHECK

		glUseProgram(program);
		GLCHECK

		glActiveTexture(GL_TEXTURE0);
		GLCHECK
		glBindTexture(GL_TEXTURE_2D, m_texture);
		GLCHECK

		int mTextureUniformHandle = glGetUniformLocation(program, "tex");
		GLCHECK
		glUniform1i(mTextureUniformHandle, 0);
		GLCHECK

		// get the attribute and uniform locations
		GLuint m_pos = glGetAttribLocation(program, "pos");
		GLCHECK
		GLuint m_uv = glGetAttribLocation(program, "uv");
		GLCHECK

		// load the attribute data
		glEnableVertexAttribArray(m_pos);
		GLCHECK
		glVertexAttribPointer(m_pos, 3, GL_FLOAT, GL_TRUE, 0, pVertices.get());
		GLCHECK
		glEnableVertexAttribArray(m_uv);
		GLCHECK
		glVertexAttribPointer(m_uv, 2, GL_FLOAT, GL_TRUE, 0, pUV.get());
		GLCHECK

		//Draw
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		GLCHECK

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();


//	imshow("img", img);
//	waitKey(0);
  return 0;
}
