#include "interface.h"

// C++ 11 String Literal
// See http://en.cppreference.com/w/cpp/language/string_literal
const char* tVertexShaderSrc =
R"zzz(#version 330 core
in vec4 vPos;
in vec4 vNorm;
in vec2 vUV;
in float temp;
in float height;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 lPos;
uniform vec4 cPos;
out vec4 lDir;
out vec4 cDir;
out vec4 normal;
out vec2 UV;
out float fTemp;
out float fHeight;
out vec4 diffuse;
out vec4 wPos;

void main()
{
    // Compute color values based on temperature 0*F = Blue, 100*F = Red
    float R = clamp((1.0/25.0)*temp - 2.0, 0.0, 1.0);
    float G = sin((3.14159/100)*temp);
    float B = clamp(-(1.0/25.0)*temp + 2.0, 0.0, 1.0);

    // Transform vertex into clipping coordinates
	wPos = vec4(cPos.x + vPos.x, height, cPos.z + vPos.z, 1.0);
	gl_Position = projection * view * wPos;
	cDir = vec4(normalize(cPos.xyz - wPos.xyz), 1.0);

    // Compute light direction and transform to camera coordinates
    lDir = vec4(normalize(lPos.xyz), 1.0);

    // pass normal to fragment shader
    normal = vNorm;

	// pass UV to fragment shader
	UV = vUV;

	// pass height and temperature to fragment shader
	fTemp = temp;
	fHeight = height;

    // Set diffuse color according to temperature
	diffuse = vec4(R, G, B, 1.0);
}
)zzz";

const char* tFragmentShaderSrc =
R"zzz(#version 330 core
in vec4 normal;
in vec4 lDir;
in vec4 cDir;
in vec2 UV;
in float fTemp;
in float fHeight;
in vec4 diffuse;
uniform float ambConst;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;
uniform sampler2D dirtTex;
uniform sampler2D snowTex;
uniform sampler2D gravelTex;
uniform sampler2D grassTex;
uniform sampler2D iceTex;
uniform sampler2D rockTex;
uniform sampler2D sandTex;
out vec4 fCol;

void main()
{
	vec3 N = normalize(normal.xyz);
	vec3 L = normalize(lDir.xyz);
	vec3 C = normalize(cDir.xyz);
	vec4 tex = texture2D(dirtTex, UV);
	vec4 sandTex = texture2D(sandTex, UV);
	vec4 rockTex = texture2D(rockTex, UV);
	vec4 grassTex = texture2D(grassTex, UV);
	vec4 iceTex = texture2D(iceTex, UV);
	vec4 snowTex = texture2D(snowTex, UV);
	vec4 gravelTex = texture2D(gravelTex, UV);

	// height and temp modifiers
	float hMod = clamp(log(-fHeight+5.0)+0.5, 0.0, 1.0);
	float snHMod = clamp(sin(fTemp/5.0 - 6.2), 0.0, 1.0);
	float saMod = clamp((fTemp  - 75.0)/5.0+0.5,0.0,1.0);
	float rMod = clamp(-(fTemp  - 45.0)/5.0+1.0,0.0,1.0);
	float iMod = clamp(-(fTemp - 30.0)/10.0+0.25,0.0,1.0);
	float snMod = clamp(-(fTemp - 25.0)/15.0+1.0,0.0,1.0);
	float gMod = clamp(sin(fTemp/13-3.05),0.0,1.0);

	// add in sand
	tex = (1.0-saMod)*tex + saMod*sandTex;

	// add in rock
	tex = (1.0 - hMod)*((1.0 - rMod)*tex + rMod*rockTex) + hMod*rockTex;

	// add ice
	tex = (1.0-iMod)*tex + iMod*iceTex;

	// add snow
	tex = (1.0-snMod)*tex + snMod*snowTex;

	// Add in grass
	tex = hMod*((1.0-gMod)*tex + gMod*grassTex) + (1.0-hMod)*tex;

	// calculate ambient component
	 vec3 amb = ambConst * ambient.xyz + diffuse.xyz * (ambConst / 2.0);

	// calculate diffuse component
	 vec3 diff = tex.rgb * max(dot(L,N), 0.0);
          diff = clamp(diff, 0.0, 1.0);

	// calculate specular component
    vec3 rflct = normalize(-reflect(L, N));
	vec3 spec = specular.xyz * pow(max(dot(rflct,C),0.0),0.3*shininess);
         spec = clamp(spec, 0.0, 1.0);

	// combine components and set fully opaque
	fCol = vec4(diff + amb + spec, 1.0);
	//fCol = vec4(UV.x, 0.0, UV.y, 1.0);
}
)zzz";

// WATER SHADERS!! >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const char* wVertexShaderSrc =
R"zzz(#version 330 core
in vec4 vPos;
in vec4 vNorm;
in vec2 vUV;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 lPos;
uniform vec4 cPos;
out vec4 lDir;
out vec4 cDir;
out vec4 normal;
out vec2 UV;
out vec4 diffuse;
out vec4 wPos;

void main()
{
    // Transform vertex into clipping coordinates
	wPos = vec4(cPos.x + vPos.x, vPos.y, cPos.z + vPos.z, 1.0);
	gl_Position = projection * view * wPos;
	cDir = vec4(normalize(cPos.xyz - wPos.xyz), 1.0);

    // Compute light direction and transform to camera coordinates
    lDir = vec4(normalize(lPos.xyz), 1.0);

    // pass normal to fragment shader
    normal = vNorm;

	// pass UV to fragment shader
	UV = vUV;

    // Set diffuse color to be blue
	diffuse = vec4(0.0, 0.0, 1.0, 1.0);
}
)zzz";

const char* wFragmentShaderSrc =
R"zzz(#version 330 core
in vec4 normal;
in vec4 lDir;
in vec4 cDir;
in vec2 UV;
in vec4 diffuse;
uniform float ambConst;
uniform vec4 ambient;
uniform vec4 specular;
uniform float shininess;
uniform sampler2D texMap;
out vec4 fCol;

void main()
{
	vec3 N = normalize(normal.xyz);
	vec3 L = normalize(lDir.xyz);
	vec3 C = normalize(cDir.xyz);
	vec4 waterTex = texture2D(texMap, UV);
	vec4 tex = 0.75*waterTex + 0.25*diffuse;

	// calculate ambient component
	vec3 amb = ambConst * ambient.xyz + diffuse.xyz * (ambConst / 2.0);

	// calculate diffuse component
	vec3 diff = tex.rgb * max(dot(L,N), 0.0);
         diff = clamp(diff, 0.0, 1.0);

	// calculate specular component
    vec3 rflct = normalize(-reflect(L, N));
	vec3 spec = specular.xyz * pow(max(dot(rflct,C),0.0),0.3*shininess);
         spec = clamp(spec, 0.0, 1.0);

	// combine components and set fully opaque
	fCol = vec4(diff + amb + spec, 0.75);
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
	GLint depthBufferSize = 0;
	std::cout << "Renderer: " << renderer << "\n";
	std::cout << "OpenGL version supported:" << version << "\n";

	// BEGIN LOAD TEXTURES INTO OPENGL
	const char* imagepath = "./assets/128x128atlas.bmp";
	unsigned char header[54]; // Each BMP file begins with a 54-byte header
	unsigned int dataPos;	  // Position in the file where the actual data begins
	unsigned int width, height;
	unsigned int imageSize; // = width*height
	unsigned char* data; // the actual data for the image
	unsigned char* dirtData;
	unsigned char* snowData;
	unsigned char* gravelData;
	unsigned char* waterData;
	unsigned char* grassData;
	unsigned char* iceData;
	unsigned char* rockData;
	unsigned char* sandData;

	GLuint texID = 0; // our texture object to return
	GLuint dirtTex = 0;
	GLuint snowTex = 0;
	GLuint gravelTex = 0;
	GLuint waterTex = 0; // our water texture
	GLuint grassTex = 0;
	GLuint iceTex = 0;
	GLuint rockTex = 0;
	GLuint sandTex = 0;
	FILE* file = fopen(imagepath, "rb");

	// This is here for debugging
	if (!file) {
		// Debug code: Used to list contents of directory specified by path
		// This can be removed if need be
		vector<std::string> v;
		std::string path(imagepath);
		WIN32_FIND_DATA data;
		HANDLE hFind;
		if ((hFind = FindFirstFile(path.c_str(), &data)) != INVALID_HANDLE_VALUE) {
			do {
				v.push_back(data.cFileName);
			} while (FindNextFile(hFind, &data) != 0);
			FindClose(hFind);
		}
		for (int i = 0; i < v.size(); i++) {
			std::cout << v[i].c_str() << std::endl;
		}
		// end debug code << end debug code << end debug code 
		cout << "image failed to load!\n";
		system("Pause");
	}

	if ((fread(header, 1, 54, file) != 54) || (header[0] != 'B' && header[1] != 'M')) {
		std::cout << "image is not .bmp or is corrupted\n";
		system("Pause");
	}

	// Get information about the image from the header
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);
	int subImgSize = (width / 4)*(height / 2) * 3; // the size of an individual texture in the atlas

	// This is here to ensure image is set up correctly (in case header information 
	// is missing or corrupted and was not caught by debug statements)
	if (imageSize == 0) {
		imageSize = width*height * 3;
	}
	if (dataPos == 0) {
		dataPos = 54; // Start reading after the header
	}

	// read data and close the file
	data = new unsigned char[imageSize];
	fread(data, 1, imageSize, file);
	fclose(file);

	// Load in our texture to our texture object
	glGenTextures(1, &texID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// END LOAD TEXTURES INTO OPENGL

	dirtData = new unsigned char[subImgSize];
	snowData = new unsigned char[subImgSize];
	gravelData = new unsigned char[subImgSize];
	waterData = new unsigned char[subImgSize];
	grassData = new unsigned char[subImgSize];
	iceData = new unsigned char[subImgSize];
	rockData = new unsigned char[subImgSize];
	sandData = new unsigned char[subImgSize];

	int subW = (width / 4) * 3;
	int subH = (height / 2);
	int count = 0;
	for (int i = 0; i < subH; i++) {
		for (int j = 0; j < subW; j++) {
			dirtData[count] = data[i*width * 3 + j];
			snowData[count] = data[i*width * 3 + j + subW];
			gravelData[count] = data[i*width * 3 + j + subW * 2];
			waterData[count] = data[i*width*3 + j +  + subW * 3];
			grassData[count] = data[i*width * 3 + j + subImgSize * 4];
			iceData[count] = data[i*width * 3 + j + subImgSize * 4 + subW];
			rockData[count] = data[i*width * 3 + j + subImgSize * 4 + subW * 2];
			sandData[count] = data[i*width * 3 + j + subImgSize * 4 + subW * 3];
			count++;
		}
	}
	glGenTextures(1, &dirtTex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dirtTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, dirtData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &snowTex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, snowTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, snowData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &gravelTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gravelTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, gravelData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &waterTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, waterTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width/4, height/2, 0, GL_BGR, GL_UNSIGNED_BYTE, waterData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &grassTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, grassTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, grassData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &iceTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, iceTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, iceData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &rockTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, rockTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, rockData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glGenTextures(1, &sandTex);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, sandTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, sandData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Init map data
	Occulus single = Occulus(camera.getEye());
	single.draw(tVertices, tNormals, tUv, tTemps, tHeights, tFaces);
	single.drawWater(wVertices, wNormals, wUV, wFaces);
	

	/* 
	================================================================================
	== Begin Setup for Terrain Shaders                                            ==
	================================================================================
	*/
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

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kHeightBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.

	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tHeights.size(), nullptr,
		GL_STATIC_DRAW));

	CHECK_GL_ERROR(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(3));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kGeometryVao][kUVBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tUv.size() * 2, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(4));

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
	CHECK_GL_ERROR(glBindAttribLocation(tProgram, 3, "height"));
	CHECK_GL_ERROR(glBindAttribLocation(tProgram, 4, "vUV"));
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
	GLint dirtTexLoc = 0;
	CHECK_GL_ERROR(dirtTexLoc =
		glGetUniformLocation(tProgram, "dirtTex"));
	GLint snowTexLoc = 0;
	CHECK_GL_ERROR(snowTexLoc =
		glGetUniformLocation(tProgram, "snowTex"));
	GLint gravelTexLoc = 0;
	CHECK_GL_ERROR(gravelTexLoc =
		glGetUniformLocation(tProgram, "gravelTex"));
	GLint grassTexLoc = 0;
	CHECK_GL_ERROR(grassTexLoc =
		glGetUniformLocation(tProgram, "grassTex"));
	GLint iceTexLoc = 0;
	CHECK_GL_ERROR(iceTexLoc =
		glGetUniformLocation(tProgram, "iceTex"));
	GLint rockTexLoc = 0;
	CHECK_GL_ERROR(rockTexLoc =
		glGetUniformLocation(tProgram, "rockTex"));
	GLint sandTexLoc = 0;
	CHECK_GL_ERROR(sandTexLoc =
		glGetUniformLocation(tProgram, "sandTex"));

	/*
	================================================================================
	== BEGIN SETUP FOR WATER SHADERS                                              ==
	================================================================================
	*/

	// Switch to the VAO for Geometry.
	CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kWaterVao]));

	// Generate buffer objects
	CHECK_GL_ERROR(glGenBuffers(kNumVbos, &gBufferObjects[kWaterVao][0]));

	// Setup vertex data in a VBO.
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wVertices.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(0));


	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kNormalBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wNormals.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(1));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kUVBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wUV.size() * 2, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
	CHECK_GL_ERROR(glEnableVertexAttribArray(2));

	// Setup element array buffer.
	CHECK_GL_ERROR(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gBufferObjects[kWaterVao][kIndexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		sizeof(uint32_t) * wFaces.size() * 3,
		&wFaces[0], GL_STATIC_DRAW));

	// Setup vertex shader.
	GLuint wVertexShader = 0;
	const char* wVertexShaderSrcPtr = wVertexShaderSrc;
	CHECK_GL_ERROR(wVertexShader = glCreateShader(GL_VERTEX_SHADER));
	CHECK_GL_ERROR(glShaderSource(wVertexShader, 1, &wVertexShaderSrcPtr, nullptr));
	glCompileShader(wVertexShader);
	CHECK_GL_SHADER_ERROR(wVertexShader);

	// Setup fragment shader.
	GLuint wFragmentShader = 0;
	const char* wFragmentShaderSrcPtr = wFragmentShaderSrc;
	CHECK_GL_ERROR(wFragmentShader = glCreateShader(GL_FRAGMENT_SHADER));
	CHECK_GL_ERROR(glShaderSource(wFragmentShader, 1, &wFragmentShaderSrcPtr, nullptr));
	glCompileShader(wFragmentShader);
	CHECK_GL_SHADER_ERROR(wFragmentShader);

	// Let's create our program.
	GLuint wProgram = 0;
	CHECK_GL_ERROR(wProgram = glCreateProgram());
	CHECK_GL_ERROR(glAttachShader(wProgram, wVertexShader));
	CHECK_GL_ERROR(glAttachShader(wProgram, wFragmentShader));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kVertexBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wVertices.size() * 4, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kNormalBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wNormals.size() * 4, nullptr,
		GL_STATIC_DRAW));
	// Bind attributes.
	CHECK_GL_ERROR(glBindAttribLocation(wProgram, 0, "vPos"));
	CHECK_GL_ERROR(glBindAttribLocation(wProgram, 1, "vNorm"));
	CHECK_GL_ERROR(glBindAttribLocation(wProgram, 2, "vUV"));
	CHECK_GL_ERROR(glBindFragDataLocation(wProgram, 0, "fCol"));
	glLinkProgram(wProgram);
	CHECK_GL_PROGRAM_ERROR(wProgram);

	// Get the uniform locations.
	GLint projMatLocW = 0;
	CHECK_GL_ERROR(projMatLocW =
		glGetUniformLocation(wProgram, "projection"));
	GLint viewMatLocW = 0;
	CHECK_GL_ERROR(viewMatLocW =
		glGetUniformLocation(wProgram, "view"));
	GLint lPosLocW = 0;
	CHECK_GL_ERROR(lPosLocW =
		glGetUniformLocation(wProgram, "lPos"));
	GLint cPosLocW = 0;
	CHECK_GL_ERROR(cPosLocW =
		glGetUniformLocation(wProgram, "cPos"));
	GLint ambCLocW = 0;
	CHECK_GL_ERROR(ambCLocW =
		glGetUniformLocation(wProgram, "ambConst"));
	GLint ambLocW = 0;
	CHECK_GL_ERROR(ambLocW =
		glGetUniformLocation(wProgram, "ambient"));
	GLint specLocW = 0;
	CHECK_GL_ERROR(specLocW =
		glGetUniformLocation(wProgram, "specular"));
	GLint shinyLocW = 0;
	CHECK_GL_ERROR(shinyLocW =
		glGetUniformLocation(wProgram, "shininess"));
	GLint texLocW = 0;
	CHECK_GL_ERROR(texLocW =
		glGetUniformLocation(wProgram, "texMap"));

	/*
	================================================================================
	== END OF SHADER SETUP AREA                                                   ==
	================================================================================
	*/
	// run geometry here so old buffers are bound
	tVertices.clear();
	tNormals.clear();
	tFaces.clear();
	tTemps.clear();
	tUv.clear();
	tHeights.clear();
	wVertices.clear();
	wNormals.clear();
	wUV.clear();
	wFaces.clear();
	single.draw(tVertices, tNormals, tUv, tTemps, tHeights, tFaces);
	single.drawWater(wVertices, wNormals, wUV, wFaces);

	// Send vertices to the GPU. (Do outside loop for efficiency)
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		gBufferObjects[kGeometryVao][kVertexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tVertices.size() * 4,
		&tVertices[0], GL_STATIC_DRAW));
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		gBufferObjects[kGeometryVao][kUVBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * tUv.size() * 2,
		&tUv[0], GL_STATIC_DRAW));

	// Send Vertices for water to theGPU
	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		gBufferObjects[kWaterVao][kVertexBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wVertices.size() * 4,
		&wVertices[0], GL_STATIC_DRAW));

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
		gBufferObjects[kWaterVao][kUVBuffer]));
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wUV.size() * 2,
		&wUV[0], GL_STATIC_DRAW));

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

		single.update(camera.getEye(), tHeights, tNormals, tTemps);

		// Compute the projection matrix.
		aspect = static_cast<float>(winWidth) / winHeight;

		mat4 projection_matrix =
			perspective(radians(45.0f), aspect, 1.0f, 1000.0f);

		// Compute the view matrix
		mat4 view_matrix = camera.getViewMatrix();

		// Send normals, temps, and heights to the GPU for terrain generator
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
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
			gBufferObjects[kGeometryVao][kHeightBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * tHeights.size(),
			&tHeights[0], GL_STATIC_DRAW));

		// Use our program.
		CHECK_GL_ERROR(glUseProgram(tProgram));

		// Pass uniforms for terrain shaders in.
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
		CHECK_GL_ERROR(glUniform1i(dirtTexLoc, 0));
		CHECK_GL_ERROR(glUniform1i(snowTexLoc, 1));
		CHECK_GL_ERROR(glUniform1i(gravelTexLoc, 2));
		CHECK_GL_ERROR(glUniform1i(grassTexLoc, 4));
		CHECK_GL_ERROR(glUniform1i(iceTexLoc, 5));
		CHECK_GL_ERROR(glUniform1i(rockTexLoc, 6));
		CHECK_GL_ERROR(glUniform1i(sandTexLoc, 7));


		// Draw our triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, tFaces.size() * 3, GL_UNSIGNED_INT, 0));

		// Switch to water vao and then send everything to the GPU
		// Switch to the Water VAO.
		CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kWaterVao]));

		// Send normals for water shader to GPU here (may remove later)
		CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER,
			gBufferObjects[kWaterVao][kNormalBuffer]));
		CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
			sizeof(float) * wNormals.size() * 4,
			&wNormals[0], GL_STATIC_DRAW));

		// Use our program.
		CHECK_GL_ERROR(glUseProgram(wProgram));

		// Pass uniforms for water shaders in.
		CHECK_GL_ERROR(glUniformMatrix4fv(projMatLocW, 1, GL_FALSE,
			&projection_matrix[0][0]));
		CHECK_GL_ERROR(glUniformMatrix4fv(viewMatLocW, 1, GL_FALSE,
			&view_matrix[0][0]));
		CHECK_GL_ERROR(glUniform4fv(lPosLocW, 1, &lightPos[0]));
		CHECK_GL_ERROR(glUniform4fv(cPosLocW, 1, &camera.getEye()[0]));
		CHECK_GL_ERROR(glUniform4fv(ambLocW, 1, &wAmbient[0]));
		CHECK_GL_ERROR(glUniform1f(ambCLocW, ambConstant));
		CHECK_GL_ERROR(glUniform4fv(specLocW, 1, &wSpecular[0]));
		CHECK_GL_ERROR(glUniform1f(shinyLocW, wShininess));
		CHECK_GL_ERROR(glUniform1i(texLocW, 3));
		
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, wFaces.size() * 3, GL_UNSIGNED_INT, 0));
		// END OF WATER SHADER STUFF

		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}