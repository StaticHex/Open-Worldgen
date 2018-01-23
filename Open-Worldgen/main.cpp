#include "interface.h"

// C++ 11 String Literal
// See http://en.cppreference.com/w/cpp/language/string_literal
const char* vertex_shader =
R"zzz(#version 330 core
in vec4 vertex_position;
in vec4 vertex_normal;
//in vec2 vertex_uv;
in float temp_data;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 lightPos;
uniform vec4 camera_position;
//out vec2 uv_coord;
out vec4 light_direction;
out vec4 camera_direction;
out vec4 normal;
out vec4 diffuse;
out vec4 world_position;
void main()
{
float R = clamp((1.0/25.0)*temp_data - 2.0, 0.0, 1.0);
float G = sin((3.14159/100)*temp_data);
float B = clamp(-(1.0/25.0)*temp_data + 2.0, 0.0, 1.0);

    // Transform vertex into clipping coordinates
	world_position = vertex_position;
	gl_Position = projection * view * vertex_position;
	camera_direction = camera_position - gl_Position;

    // Lighting in camera coordinates
    // Compute light direction and transform to camera coordinates
    light_direction = normalize(lightPos - vertex_position);

    //  Transform normal to camera coordinates
    normal = vertex_normal;
	diffuse = vec4(R, G, B, 1.0);
	//uv_coord = vertex_uv;
}
)zzz";

const char* fragment_shader =
R"zzz(#version 330 core
in vec4 normal;
in vec4 light_direction;
in vec4 diffuse;
in vec2 uv_coord;
uniform int stage;
uniform float ambient;
uniform vec4 specular;
uniform float shininess;
uniform sampler2D textureSampler;
in vec4 camera_direction;
out vec4 fragment_color;
void main()
{
	fragment_color = diffuse;
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
	Sector testSector = Sector();
	testSector.drawSector(tVertices, tNormals, tUv, tTemps);
	for (int i = 0; i < 36; i += 3) {
		tFaces.push_back(vec3(i, i + 1, i + 2));
	}
	
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
	GLuint vertex_shader_id = 0;
	const char* vertex_source_pointer = vertex_shader;
	
	CHECK_GL_ERROR(vertex_shader_id = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(vertex_shader_id, 1, &vertex_source_pointer, nullptr));
	glCompileShader(vertex_shader_id);
	CHECK_GL_SHADER_ERROR(vertex_shader_id);

	// Setup fragment shader.
	GLuint fragment_shader_id = 0;
	const char* fragment_source_pointer = fragment_shader;
	CHECK_GL_ERROR(fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(fragment_shader_id, 1, &fragment_source_pointer, nullptr));
	glCompileShader(fragment_shader_id);
	CHECK_GL_SHADER_ERROR(fragment_shader_id);

	// Let's create our program.
	GLuint program_id = 0;
	CHECK_GL_ERROR(program_id = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(program_id, vertex_shader_id));
	CHECK_GL_ERROR(glAttachShader(program_id, fragment_shader_id));

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
	CHECK_GL_ERROR(glBindAttribLocation(program_id, 0, "vertex_position"));
	CHECK_GL_ERROR(glBindAttribLocation(program_id, 1, "vertex_normal"));
	CHECK_GL_ERROR(glBindAttribLocation(program_id, 2, "vertex_uv"));
	CHECK_GL_ERROR(glBindAttribLocation(program_id, 3, "temp_data"));
	CHECK_GL_ERROR(glBindFragDataLocation(program_id, 0, "fragment_color"));
	glLinkProgram(program_id);
	CHECK_GL_PROGRAM_ERROR(program_id);

	// Get the uniform locations.
	GLint projection_matrix_location = 0;
	CHECK_GL_ERROR(projection_matrix_location =
		glGetUniformLocation(program_id, "projection"));
	GLint view_matrix_location = 0;
	CHECK_GL_ERROR(view_matrix_location =
		glGetUniformLocation(program_id, "view"));
	GLint lightPos_location = 0;
	CHECK_GL_ERROR(lightPos_location =
		glGetUniformLocation(program_id, "lightPos"));
	GLint camera_position_location = 0;
	CHECK_GL_ERROR(camera_position_location =
		glGetUniformLocation(program_id, "camera_position"));
	GLint ambient_location = 0;
	CHECK_GL_ERROR(ambient_location =
		glGetUniformLocation(program_id, "ambient"));
	GLint specular_location = 0;
	CHECK_GL_ERROR(specular_location =
		glGetUniformLocation(program_id, "specular"));
	GLint shininess_location = 0;
	CHECK_GL_ERROR(shininess_location =
		glGetUniformLocation(program_id, "shininess"));

	// run geometry here so old buffers are bound
	tVertices.clear();
	tNormals.clear();
	tFaces.clear();
	tTemps.clear();
	testSector.drawSector(tVertices, tNormals, tUv, tTemps);
	for (int i = 0; i < 36; i += 3) {
		tFaces.push_back(vec3(i, i + 1, i + 2));
	}
	
	while (!glfwWindowShouldClose(window)) {
		// Setup some basic window stuff.
		glfwGetFramebufferSize(window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);
		glClearColor(0.0f, 0.8f, 1.0f, 0.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		
		// Switch to the Geometry VAO.
		CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kGeometryVao]));

		// Compute the projection matrix.
		aspect = static_cast<float>(winWidth) / winHeight;
		glm::mat4 projection_matrix =
			glm::perspective(glm::radians(45.0f), aspect, 0.0001f, 1000.0f);

		// Compute the view matrix
		// FIXME: change eye and center through mouse/keyboard events.
		glm::mat4 view_matrix = camera.getViewMatrix();

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
		CHECK_GL_ERROR(glUseProgram(program_id));

		// Pass uniforms in.
		CHECK_GL_ERROR(glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE,
			&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(view_matrix_location, 1, GL_FALSE,
			&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(lightPos_location, 1, &lightPos[0]));
		CHECK_GL_ERROR(glUniform4fv(camera_position_location, 1, &camera.getEye()[0]));
		CHECK_GL_ERROR(glUniform1f(ambient_location, ambient));
		CHECK_GL_ERROR(glUniform4fv(specular_location, 1, &tSpecular[0]));
		CHECK_GL_ERROR(glUniform1f(shininess_location, tShininess));
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