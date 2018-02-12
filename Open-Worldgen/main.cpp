#include "interface.h"

// C++ 11 String Literal
// See http://en.cppreference.com/w/cpp/language/string_literal
const char* tVertexShaderSrc =
R"zzz(#version 330 core
in vec4 vPos;
in vec4 vNorm;
in float temp;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 lPos;
uniform vec4 cPos;
out vec4 lDir;
out vec4 cDir;
out vec4 normal;
out vec4 diffuse;
out vec4 wPos;
void main()
{
    // Compute color values based on temperature 0*F = Blue, 100*F = Red
    float R = clamp((1.0/25.0)*temp - 2.0, 0.0, 1.0);
    float G = sin((3.14159/100)*temp);
    float B = clamp(-(1.0/25.0)*temp + 2.0, 0.0, 1.0);

    // Transform vertex into clipping coordinates
	wPos = vPos;
	gl_Position = projection * view * vPos;
	cDir = vec4(cPos.xyz - gl_Position.xyz, 1.0);

    // Compute light direction and transform to camera coordinates
    lDir = vec4(normalize(lPos.xyz), 1.0);

    //  Transform normal to camera coordinates
    normal = vNorm;

    // Set diffuse color according to temperature
	diffuse = vec4(R, G, B, 1.0);
}
)zzz";

const char* tFragmentShaderSrc =
R"zzz(#version 330 core
in vec4 normal;
in vec4 lDir;
in vec4 cDir;
in vec4 diffuse;
uniform float ambConst;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;
out vec4 fCol;
void main()
{
	vec3 N = normal.xyz;
	vec3 L = lDir.xyz;
	vec3 C = cDir.xyz;
	// calculate ambient component
	 vec3 amb = ambConst * ambient.xyz + diffuse.xyz * (ambConst / 2.0);

	// calculate diffuse component
     vec3 diff = diffuse.xyz * max(dot(L,N), 0.0);
          diff = clamp(diff, 0.0, 1.0);

	// calculate specular component
    vec3 rflct = normalize(-reflect(L, N));
	vec3 spec = specular.xyz * pow(max(dot(rflct,C),0.0),0.3*shininess);
         spec = clamp(spec, 0.0, 1.0);

	// combine components and set fully opaque
	fCol = vec4(diff + amb + spec, 1.0);
	//fCol = diffuse;
}
)zzz";

// FIXME: Implement shader effects with an alternative shader.

void ErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << "\n";
}

int main(int argc, char* argv[])
{

	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);

	// Ask an OpenGL 3.3 core profile context 
	// It is required on OSX and non-NVIDIA Linux
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(winWidth, winHeight,
		&winTitle[0], nullptr, nullptr);
	CHECK_SUCCESS(window != nullptr);
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MousePosCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSwapInterval(1);
	const GLubyte* renderer = glGetString(GL_RENDERER);  // get renderer string
	const GLubyte* version = glGetString(GL_VERSION);    // version as a string
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	// Init map data
	Occulus single = Occulus(camera.getEye());
	single.draw(tVertices, tNormals, tUv, tTemps, tFaces);
	
	// Setup our VAO array.
	CHECK_GL_ERROR(glGenVertexArrays(kNumVaos, &gArrayObjects[0]));
	
	// Switch to the VAO for Geometry.
	CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kGeometryVao]));
	
	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &gBufferObjects[kGeometryVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tVertices.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kNormalBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tNormals.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(1));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kTempBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.

	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tTemps.size(), nullptr,
		GL_STATIC_DRAW));
	
	CHECK_GL_ERROR(glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(2));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint32_t) * tFaces.size() * 3,
		&tFaces[0], GL_STATIC_DRAW));
	
	// Setup vertex shader.
	GLuint tVertexShader = 0;
	const char* tVertexShaderSrcPtr = tVertexShaderSrc;
	
	CHECK_GL_ERROR(tVertexShader = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(tVertexShader, 1, &tVertexShaderSrcPtr, nullptr));
	glCompileShader(tVertexShader);
	CHECK_GL_SHADER_ERROR(tVertexShader);

	// Setup fragment shader.
	GLuint tFragmentShader = 0;
	const char* tFragmentShaderSrcPtr = tFragmentShaderSrc;
	CHECK_GL_ERROR(tFragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(tFragmentShader, 1, &tFragmentShaderSrcPtr, nullptr));
	glCompileShader(tFragmentShader);
	CHECK_GL_SHADER_ERROR(tFragmentShader);

	// Let's create our program.
	GLuint tProgram = 0;
	CHECK_GL_ERROR(tProgram = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(tProgram, tVertexShader));
	CHECK_GL_ERROR(glAttachShader(tProgram, tFragmentShader));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tVertices.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kNormalBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tNormals.size() * 4, nullptr,
		GL_STATIC_DRAW));
	// Bind attributes.
	CHECK_GL_ERROR(glBindAttribLocation(tProgram, 0, "vPos"));
	CHECK_GL_ERROR(glBindAttribLocation(tProgram, 1, "vNorm"));
	CHECK_GL_ERROR(glBindAttribLocation(tProgram, 2, "temp"));
	CHECK_GL_ERROR(glBindFragDataLocation(tProgram, 0, "fCol"));
	glLinkProgram(tProgram);
	CHECK_GL_PROGRAM_ERROR(tProgram);

	// Get the uniform locations.
	GLint projMatLoc = 0;
	CHECK_GL_ERROR(projMatLoc =
		glGetUniformLocation(tProgram, "projection"));
	GLint viewMatLoc = 0;
	CHECK_GL_ERROR(viewMatLoc =
		glGetUniformLocation(tProgram, "view"));
	GLint lPosLoc = 0;
	CHECK_GL_ERROR(lPosLoc =
		glGetUniformLocation(tProgram, "lPos"));
	GLint cPosLoc = 0;
	CHECK_GL_ERROR(cPosLoc =
		glGetUniformLocation(tProgram, "cPos"));
	GLint ambCLoc = 0;
	CHECK_GL_ERROR(ambCLoc =
		glGetUniformLocation(tProgram, "ambConst"));
	GLint ambLoc = 0;
	CHECK_GL_ERROR(ambLoc =
		glGetUniformLocation(tProgram, "ambient"));
	GLint specLoc = 0;
	CHECK_GL_ERROR(specLoc =
		glGetUniformLocation(tProgram, "specular"));
	GLint shinyLoc = 0;
	CHECK_GL_ERROR(shinyLoc =
		glGetUniformLocation(tProgram, "shininess"));

	// run geometry here so old buffers are bound
	
	tVertices.clear();
	tNormals.clear();
	tFaces.clear();
	tTemps.clear();
	single.draw(tVertices, tNormals, tUv, tTemps, tFaces);
	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);
		glClearColor(0.0f, 0.8f, 1.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// Switch to the Geometry VAO.
		CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kGeometryVao]));

		single.update(camera.getEye(), tVertices, tNormals, tUv, tTemps);

		// Compute the projection matrix.
		aspect = static_cast<float>(winWidth) / winHeight;
		mat4 projection_matrix =
			perspective(radians(45.0f), aspect, 0.0001f, 1000.0f);

		// Compute the view matrix
		// FIXME: change eye and center through mouse/keyboard events.
		mat4 view_matrix = camera.getViewMatrix();

		// Send vertices to the GPU.
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
			gBufferObjects[kGeometryVao][kVertexBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * tVertices.size() * 4,
			&tVertices[0], GL_STATIC_DRAW));
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
			gBufferObjects[kGeometryVao][kNormalBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * tNormals.size() * 4,
			&tNormals[0], GL_STATIC_DRAW));
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
			gBufferObjects[kGeometryVao][kTempBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * tTemps.size(),
			&tTemps[0], GL_STATIC_DRAW));

		// Use our program.
		CHECK_GL_ERROR(glUseProgram(tProgram));

		// Pass uniforms in.
		CHECK_GL_ERROR(glUniformMatrix4fv(projMatLoc, 1, GL_FALSE,
			&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(viewMatLoc, 1, GL_FALSE,
			&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(lPosLoc, 1, &lightPos[0]));
		CHECK_GL_ERROR(glUniform4fv(cPosLoc, 1, &camera.getEye()[0]));
		CHECK_GL_ERROR(glUniform4fv(ambLoc, 1, &tAmbient[0]));
		CHECK_GL_ERROR(glUniform1f(ambCLoc, ambConstant));
		CHECK_GL_ERROR(glUniform4fv(specLoc, 1, &tSpecular[0]));
		CHECK_GL_ERROR(glUniform1f(shinyLoc, tShininess));
		//CHECK_GL_ERROR(glUniform1i(tex_location, 0));

		// Draw our triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, tFaces.size() * 3, GL_UNSIGNED_INT, 0));
		
		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}