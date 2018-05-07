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
out vec4 cDist;

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
	cDist = vec4(cPos.xyz - wPos.xyz, 1.0);

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
in vec4 cDist;
in vec2 UV;
in float fTemp;
in float fHeight;
in vec4 diffuse;
uniform float ambConst;
uniform vec4 ambient;
uniform vec4 specular;
uniform vec4 sceneCol;
uniform float fogDist;
uniform float shininess;
uniform sampler2D dirtTex;
uniform sampler2D snowTex;
uniform sampler2D gravelTex;
uniform sampler2D grassTex;
uniform sampler2D iceTex;
uniform sampler2D rockTex;
uniform sampler2D sandTex;
uniform float seaLev;
out vec4 fCol;

void main()
{
	vec3 N = normalize(normal.xyz);
	vec3 L = normalize(lDir.xyz);
	vec3 C = normalize(cDir.xyz);
	vec4 tex = texture2D(dirtTex, UV);
	vec4 dirtTex = texture2D(dirtTex, UV);
	vec4 sandTex = texture2D(sandTex, UV);
	vec4 rockTex = texture2D(rockTex, UV);
	vec4 grassTex = texture2D(grassTex, UV);
	vec4 iceTex = texture2D(iceTex, UV);
	vec4 snowTex = texture2D(snowTex, UV);
	vec4 gravelTex = texture2D(gravelTex, UV);

	// height and temp modifiers
	float hMod = clamp(log(-fHeight+5.0)+0.5, 0.0, 1.0);
	float snHMod = clamp(cos((fHeight + 0.5)/(1.0*3.14159))/1.5 + 0.5,0.0,1.0);
	float saMod = clamp((fTemp  - 75.0)/5.0+0.5,0.0,1.0);
	float rMod = clamp(-(fTemp  - 45.0)/5.0+1.0,0.0,1.0);
	float dMod = clamp(-(fTemp  - 70.0)/5.0+1.0,0.0,1.0);
	float iMod = clamp(-(fTemp - 32.0)/10.0+0.25,0.0,1.0);
	float snMod = clamp(-(fTemp - 16.0)/15.0+1.0,0.0,1.0);
	float gMod = clamp(sin(fTemp/13-3.05),0.0,1.0);
	float grMod = clamp(-(fTemp - 35.0)/10.0+0.25,0.0,1.0);
	float slMod = clamp(log(-fHeight + 1.0 + seaLev) + 1.0, 0.0, 1.0);
	float wfMod = clamp(log(-fHeight + 0.5 + seaLev)/2.5, 0.0, 1.0);

	// add in rock
	tex = (1.0 - hMod)*((1.0 - rMod)*tex + rMod*rockTex) + hMod*rockTex;

	// add ice
	tex = (1.0-iMod)*tex + iMod*iceTex;

	// add snow
	tex = (1.0-snMod)*tex + snMod*snowTex;

	// Add in grass
	tex = hMod*((1.0-gMod)*tex + gMod*grassTex) + (1.0-hMod)*tex;


	// add in sand
	tex = (1.0 - slMod) * ((1.0-saMod)*tex + saMod*sandTex) + slMod * ((1.0 - saMod) * tex + saMod*sandTex);

	// add in mud
	tex = (1.0 - slMod) * tex + slMod * ((1.0 - dMod) * tex + dMod * dirtTex);

	// add in gravel
	tex = (1.0 - slMod) * tex + slMod * ((1.0 - grMod) * tex + grMod * gravelTex);

	// add in blue for areas under water
	float aquaC = 1.0 - wfMod;
	vec4 wBlue = wfMod * vec4(0.0, aquaC*0.67, aquaC, 1.0);
		 wBlue += (clamp(wfMod-0.5,0.0,1.0)) * vec4(0.0, 0.0, aquaC*0.5, 1.0);
	tex = (1.0 - wfMod) * tex + wBlue;
	
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
	float sceneFact = pow(length(cDist) / fogDist, 2);
	vec3 comboCol = (diff+amb+spec) * clamp(1.0 - sceneFact, 0.0, 1.0);
	vec3 fogCol = sceneCol.xyz * clamp(sceneFact, 0.0, 1.0);
	fCol = vec4(comboCol + fogCol, 1.0);
	//fCol = vec4(clamp(fogDist, 0.0, 1.0), 0.0, 0.0, 1.0);
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
uniform int time;
uniform float seaLev;
out vec4 lDir;
out vec4 cDir;
out vec4 cDist;
out vec4 normal;
out vec2 UV;
out vec4 diffuse;
out vec4 wPos;

vec4 calcNorm(vec4 p1, vec4 p2, vec4 p3) {
	vec3 pp1 = p1.xyz;
	vec3 pp2 = p2.xyz;
	vec3 pp3 = p3.xyz;
	vec3 U = normalize(pp2 - pp1);
	vec3 V = normalize(pp3 - pp1);
	return vec4(cross(U, V), 1.0);
}

float calcP(float t, float p) {
	return t * 2.0 * 3.14159 / p;
}

float calcY(float xPos, float zPos, float t) {
	float oSFac = 0.0167;
	float smSFac = 0.5;
	float lgSFac = 2.0;
	float wave = oSFac*cos(lgSFac*sin(smSFac*cos(sin(xPos + t) + zPos - t) + xPos + t) + smSFac*cos(lgSFac*sin(cos(zPos + t) + xPos - t) + zPos + t));
		  wave += oSFac*cos(lgSFac*sin(smSFac*cos(sin(xPos - t) + zPos + t) + xPos + t) + smSFac*cos(lgSFac*sin(cos(zPos - t) + xPos + t) + zPos + t));
		  wave += oSFac*cos(lgSFac*sin(smSFac*cos(sin(xPos - t) + zPos + t) + xPos - t) + smSFac*cos(lgSFac*sin(cos(zPos - t) + xPos + t) + zPos - t));
	return wave+seaLev;
}

void main()
{
	// vpos is read only so whe have to create an augmented position first
	vec4 augPos = vPos;
		augPos.y += seaLev;
	float spacing = 0.25 * 2.0;
	float t = calcP(time, 60.0);

	// create some virtual vertices
	vec4 v_p1 = vec4(augPos.x, calcY(augPos.x, augPos.z + spacing, t), augPos.z + spacing, 1.0);
	vec4 v_p2 = vec4(augPos.x + spacing, calcY(augPos.x + spacing, augPos.z + spacing, t), augPos.z + spacing, 1.0);
	vec4 v_p3 = vec4(augPos.x + spacing, calcY(augPos.x + spacing, augPos.z, t), augPos.z, 1.0);
	vec4 v_p4 = vec4(augPos.x, calcY(augPos.x, augPos.z - spacing, t), augPos.z - spacing, 1.0);
	vec4 v_p5 = vec4(augPos.x - spacing, calcY(augPos.x - spacing, augPos.z - spacing, t), augPos.z - spacing, 1.0);
	vec4 v_p6 = vec4(augPos.x - spacing, calcY(augPos.x - spacing, augPos.z, t), augPos.z, 1.0);

	// create some virtual face normals 
	vec4 v_n1 = calcNorm(v_p6, augPos, v_p1);
	vec4 v_n2 = calcNorm(v_p1, v_p2, augPos);
	vec4 v_n3 = calcNorm(augPos, v_p3, v_p2);
	vec4 v_n4 = calcNorm(augPos, v_p3, v_p4);
	vec4 v_n5 = calcNorm(v_p5, v_p4, augPos);
	vec4 v_n6 = calcNorm(v_p6, augPos, v_p5);

	// average virtual normals together for smooth shading effect
	vec3 v_sn = normalize(v_n1.xyz + v_n2.xyz + v_n3.xyz + v_n4.xyz + v_n5.xyz + v_n6.xyz);

    // Transform vertex into clipping coordinates
	wPos = vec4(cPos.x + augPos.x, augPos.y, cPos.z + augPos.z, 1.0);
	//wPos.y = calcY(wPos.x, wPos.z, t);
	gl_Position = projection * view * wPos;
	cDir = vec4(normalize(cPos.xyz - wPos.xyz), 1.0);
	cDist = vec4(cPos.xyz - wPos.xyz, 1.0);
	wPos = gl_Position;

    // Compute light direction and transform to camera coordinates
    lDir = vec4(normalize(lPos.xyz), 1.0);

    // pass normal to fragment shader
	normal = vec4(v_sn, 1.0);

	// pass UV to fragment shader
	UV = vUV;

    // Set diffuse color to be blue
	diffuse = vec4(0.0, 0.0, 0.3, 1.0);
}
)zzz";

const char* wFragmentShaderSrc =
R"zzz(#version 330 core
in vec4 normal;
in vec4 lDir;
in vec4 cDir;
in vec4 cDist;
in vec2 UV;
in vec4 diffuse;
in vec4 wPos;
uniform float ambConst;
uniform vec4 ambient;
uniform vec4 sceneCol;
uniform float fogDist;
uniform vec4 specular;
uniform float shininess;
uniform int time;
uniform sampler2D texMap;
out vec4 fCol;

void main()
{
	vec3 N = normalize(normal.xyz);
	vec3 L = normalize(lDir.xyz);
	vec3 C = normalize(cDir.xyz);
	vec4 waterTex = texture2D(texMap, UV);
	vec4 tex = 0.5*waterTex + 0.5*diffuse;

	// calculate ambient component
	vec3 amb = ambConst * ambient.xyz + diffuse.xyz * (ambConst / 2.0);

	// calculate diffuse component
	vec3 diff = tex.rgb * max(dot(L,N), 0.0);
         diff = clamp(diff, 0.0, 1.0);

	// calculate specular component
    vec3 rflct = normalize(-reflect(L, N));
	vec3 spec = specular.xyz * pow(max(dot(rflct,C),0.0),0.3*shininess);
         spec = clamp(spec, 0.0, 1.0);

	float sceneFact = pow(length(cDist) / fogDist, 2);
	float waterFact = sqrt(length(cDist)) / 5.0;
	vec3 comboCol = (diff+amb+spec) * clamp(1.0 - sceneFact, 0.0, 1.0);
	vec3 fogCol = sceneCol.xyz * clamp(sceneFact, 0.0, 1.0);

	// combine components and set fully opaque
	fCol = vec4(comboCol + fogCol, clamp(waterFact, 0.25, 1.0));
}
)zzz";

// FIXME: Implement shader effects with an alternative shader.

void ErrorCallback(int error, const char* description)
{
	std::cerr << "GLFW Error: " << description << "\n";
}

void run_opengl() {
	if (!glfwInit()) exit(EXIT_FAILURE);
	glfwSetErrorCallback(ErrorCallback);

	// Ask an OpenGL 3.3 core profile context 
	// It is required on OSX and non-NVIDIA Linux
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	centerX = mode->width / 2.0;
	centerY = mode->height / 2.0;
	GLFWwindow* gl_window = glfwCreateWindow(winWidth, winHeight,
		&winTitle[0], nullptr, nullptr);
	
	glfwSetWindowPos(gl_window, centerX - winWidth / 2.0, centerY - winHeight / 2.0);

	CHECK_SUCCESS(gl_window != nullptr);
	glfwMakeContextCurrent(gl_window);
	glewExperimental = GL_TRUE;

	CHECK_SUCCESS(glewInit() == GLEW_OK);
	glGetError();  // clear GLEW's error for it
	glfwSetKeyCallback(gl_window, KeyCallback);
	glfwSetCursorPosCallback(gl_window, MousePosCallback);
	glfwSetMouseButtonCallback(gl_window, MouseButtonCallback);
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
			waterData[count] = data[i*width * 3 + j + +subW * 3];
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
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width / 4, height / 2, 0, GL_BGR, GL_UNSIGNED_BYTE, waterData);
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
	single.drawWater(wVertices, wUV, wFaces);


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
	GLint sceneColLoc = 0;
	CHECK_GL_ERROR(sceneColLoc =
		glGetUniformLocation(tProgram, "sceneCol"));
	GLint fogDistLoc = 0;
	CHECK_GL_ERROR(fogDistLoc =
		glGetUniformLocation(tProgram, "fogDist"));
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
	GLint seaLevLoc = 0;
	CHECK_GL_ERROR(seaLevLoc =
		glGetUniformLocation(tProgram, "seaLev"));


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

	CHECK_GL_ERROR(glBindBuffer(GL_ARRAY_BUFFER, gBufferObjects[kWaterVao][kUVBuffer]));
	// NOTE: We do not send anything right now, we just describe it to OpenGL.
	CHECK_GL_ERROR(glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * wUV.size() * 2, nullptr,
		GL_STATIC_DRAW));
	CHECK_GL_ERROR(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0));
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
	// Bind attributes.
	CHECK_GL_ERROR(glBindAttribLocation(wProgram, 0, "vPos"));
	CHECK_GL_ERROR(glBindAttribLocation(wProgram, 1, "vUV"));
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
	GLint sceneColLocW = 0;
	CHECK_GL_ERROR(sceneColLocW =
		glGetUniformLocation(wProgram, "sceneCol"));
	GLint fogDistLocW = 0;
	CHECK_GL_ERROR(fogDistLocW =
		glGetUniformLocation(wProgram, "fogDist"));
	GLint shinyLocW = 0;
	CHECK_GL_ERROR(shinyLocW =
		glGetUniformLocation(wProgram, "shininess"));
	GLint timeLocW = 0;
	CHECK_GL_ERROR(timeLocW =
		glGetUniformLocation(wProgram, "time"));
	GLint texLocW = 0;
	CHECK_GL_ERROR(texLocW =
		glGetUniformLocation(wProgram, "texMap"));
	GLint seaLevelW = 0;
	CHECK_GL_ERROR(seaLevelW =
		glGetUniformLocation(wProgram, "seaLev"));

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
	wUV.clear();
	wFaces.clear();
	single.draw(tVertices, tNormals, tUv, tTemps, tHeights, tFaces);
	single.drawWater(wVertices, wUV, wFaces);

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

	while (!glfwWindowShouldClose(gl_window)) {
		fps += 1;
		// Setup some basic window stuff.
		glfwGetFramebufferSize(gl_window, &winWidth, &winHeight);
		glViewport(0, 0, winWidth, winHeight);
		glClearColor(tSceneCol.x, tSceneCol.y, tSceneCol.z, tSceneCol.w);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_MULTISAMPLE);
		glDepthFunc(GL_LESS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		// Switch to the Geometry VAO.
		CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kGeometryVao]));

		if (!setRefresh) {
			single.update(camera.getEye(), tHeights, tNormals, tTemps, tFaces);
		}
		else {
			single.refresh();
			setRefresh = false;
		}

		// Compute the projection matrix.
		aspect = static_cast<float>(winWidth) / winHeight;

		mat4 projection_matrix =
			perspective(radians(45.0f), aspect, 1.0f, 1000.0f);

		// Compute the view matrix
		mat4 view_matrix = camera.getViewMatrix();

		// Increment water time
		wTime += 1;
		if (wTime > 60) {
			wTime = 0;
		}

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
		CHECK_GL_ERROR(glUniform4fv(sceneColLoc, 1, &tSceneCol[0]));
		CHECK_GL_ERROR(glUniform1f(fogDistLoc, tFogDist));
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
		CHECK_GL_ERROR(glUniform1f(seaLevLoc, seaLevel));


		// Draw our triangles.
		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, tFaces.size() * 3, GL_UNSIGNED_INT, 0));

		// Switch to water vao and then send everything to the GPU
		// Switch to the Water VAO.
		CHECK_GL_ERROR(glBindVertexArray(gArrayObjects[kWaterVao]));

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
		CHECK_GL_ERROR(glUniform4fv(sceneColLocW, 1, &tSceneCol[0]));
		CHECK_GL_ERROR(glUniform1f(fogDistLocW, tFogDist));
		CHECK_GL_ERROR(glUniform4fv(specLocW, 1, &wSpecular[0]));
		CHECK_GL_ERROR(glUniform1f(shinyLocW, wShininess));
		CHECK_GL_ERROR(glUniform1i(timeLocW, wTime));
		CHECK_GL_ERROR(glUniform1i(texLocW, 3));
		CHECK_GL_ERROR(glUniform1f(seaLevelW, seaLevel));

		CHECK_GL_ERROR(glDrawElements(GL_TRIANGLES, wFaces.size() * 3, GL_UNSIGNED_INT, 0));
		// END OF WATER SHADER STUFF

		// Poll and swap.
		glfwPollEvents();
		glfwSwapBuffers(gl_window);
	}
	running = false;
	glfwDestroyWindow(gl_window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

void fps_calc() {
	int t = time(0);
	cout << "Before Loop" << endl;
	while (running) {
		if (time(0) > t) {
			t = time(0);
			std::string ff = std::to_string(fps);

			box->label(ff.c_str());
			Fl::flush();
			fps = 0;
		}
	}
}

int main(int argc, char* argv[])
{
	std::thread gl_thread(run_opengl);
	std::thread fps_thread(fps_calc);
	//gl_thread.join();

	/**********************************************************************
	 * Set up main window
	 **********************************************************************/
	// Header for FPS box (label which says FPS)
	lbl_fps->box(FL_NO_BOX);
	lbl_fps->labelsize(20);
	lbl_fps->labelfont(FL_HELVETICA + FL_BOLD);
	lbl_fps->labeltype(FL_SHADOW_LABEL);

	// Box which contains FPS rate
	box->box(FL_UP_BOX);
	box->labelsize(20);
	box->labelfont(FL_HELVETICA);
	
	// Button which brings up sea level settings panel
	btnSeaLvl->image(imgSeaLvl);
	btnSeaLvl->align(FL_ALIGN_IMAGE_BACKDROP);
	btnSeaLvl->callback(showPanelCallback, seaLevelPanel);

	// button which brings up noise generation settings panel
	btnNoiseP->image(imgNoiseP);
	btnNoiseP->align(FL_ALIGN_IMAGE_BACKDROP);
	btnNoiseP->callback(showPanelCallback, adjustNoise);

	// Set up main window
	mainWindow->set_modal();
	mainWindow->add(lbl_fps);
	mainWindow->add(box);
	mainWindow->add(btnSeaLvl);
	mainWindow->add(btnNoiseP);
	mainWindow->remove(seaLevelPanel);
	mainWindow->remove(adjustNoise);
	mainWindow->end();
	mainWindow->show(argc, argv);

	/**********************************************************************
	* Set up sea level panel
	**********************************************************************/

	// Title text
	lblSeaLevel->box(FL_FLAT_BOX);
	lblSeaLevel->color(fl_rgb_color(220, 220, 220));
	lblSeaLevel->labelsize(20);
	lblSeaLevel->labelfont(FL_HELVETICA + FL_BOLD);
	lblSeaLevel->labeltype(FL_SHADOW_LABEL);

	// back button (needed since window is modal)
	btnSeaBack->box(FL_NO_BOX);
	btnSeaBack->labelsize(14);
	btnSeaBack->callback(hidePanelCallback, seaLevelPanel);
	btnSeaBack->labelcolor(fl_rgb_color(0, 170, 255));

	// slider for adjusting sea level settings
	seaSlider->box(FL_UP_BOX);
	seaSlider->range(-maxH/4.0, maxH*1.5);
	seaSlider->step(0.01);
	seaSlider->value(seaLevel);
	seaSlider->type(FL_HOR_NICE_SLIDER);
	seaSlider->textsize(12);
	seaSlider->callback(sliderCallback, seaSlider);
	seaSlider->show();

	// the sea level settings panel itself
	seaLevelPanel->set_modal();
	seaLevelPanel->add(lblSeaLevel);
	seaLevelPanel->add(seaSlider);
	seaLevelPanel->add(btnSeaBack);
	seaLevelPanel->remove(adjustNoise);
	seaLevelPanel->remove(mainWindow);
	seaLevelPanel->end();
	seaLevelPanel->show();
	seaLevelPanel->hide();

	/**********************************************************************
	* Set up noise settings panel
	**********************************************************************/

	// Title text
	lblNoiseGen->box(FL_FLAT_BOX);
	lblNoiseGen->color(fl_rgb_color(220, 220, 220));
	lblNoiseGen->labelsize(20);
	lblNoiseGen->labelfont(FL_HELVETICA + FL_BOLD);
	lblNoiseGen->labeltype(FL_SHADOW_LABEL);

	// back button (needed since window is modal)
	btnNoiseBack->box(FL_NO_BOX);
	btnNoiseBack->labelsize(14);
	btnNoiseBack->callback(hidePanelCallback, adjustNoise);
	btnNoiseBack->labelcolor(fl_rgb_color(0, 170, 255));

	// sliders for adjusting elevation settings
	noiseElevation1a->box(FL_UP_BOX);
	noiseElevation1a->range(0.0, 5.0);
	noiseElevation1a->step(0.01);
	noiseElevation1a->value(height1a);
	noiseElevation1a->type(FL_HOR_NICE_SLIDER);
	noiseElevation1a->textsize(12);
	noiseElevation1a->labelsize(14);
	noiseElevation1a->callback(sliderEH1ACallback, noiseElevation1a);
	noiseElevation1a->labelfont(FL_HELVETICA + FL_BOLD);
	noiseElevation1a->align(FL_ALIGN_TOP_LEFT);

	noiseElevation1b->box(FL_UP_BOX);
	noiseElevation1b->range(0.0, 50.0);
	noiseElevation1b->step(0.5);
	noiseElevation1b->value(height1b);
	noiseElevation1b->callback(sliderEH1BCallback, noiseElevation1b);
	noiseElevation1b->type(FL_HOR_NICE_SLIDER);
	noiseElevation1b->textsize(12);

	noiseElevation1c->box(FL_UP_BOX);
	noiseElevation1c->range(0.0, 50.0);
	noiseElevation1c->step(0.5);
	noiseElevation1c->value(height1c);
	noiseElevation1c->callback(sliderEH1CCallback, noiseElevation1c);
	noiseElevation1c->type(FL_HOR_NICE_SLIDER);
	noiseElevation1c->textsize(12);

	noiseElevation2a->box(FL_UP_BOX);
	noiseElevation2a->range(0.0, 5.0);
	noiseElevation2a->step(0.01);
	noiseElevation2a->value(height2a);
	noiseElevation2a->callback(sliderEH2ACallback, noiseElevation2a);
	noiseElevation2a->type(FL_HOR_NICE_SLIDER);
	noiseElevation2a->textsize(12);

	noiseElevation2b->box(FL_UP_BOX);
	noiseElevation2b->range(0.0, 50.0);
	noiseElevation2b->step(0.5);
	noiseElevation2b->value(height2b);
	noiseElevation2b->callback(sliderEH2BCallback, noiseElevation2b);
	noiseElevation2b->type(FL_HOR_NICE_SLIDER);
	noiseElevation2b->textsize(12);

	noiseElevation2c->box(FL_UP_BOX);
	noiseElevation2c->range(0.0, 50.0);
	noiseElevation2c->step(0.5);
	noiseElevation2c->value(height2c);
	noiseElevation2c->callback(sliderEH2CCallback, noiseElevation2c);
	noiseElevation2c->type(FL_HOR_NICE_SLIDER);
	noiseElevation2c->textsize(12);

	noiseElevation3a->box(FL_UP_BOX);
	noiseElevation3a->range(0.0, 5.0);
	noiseElevation3a->step(0.01);
	noiseElevation3a->value(height3a);
	noiseElevation3a->callback(sliderEH3ACallback, noiseElevation3a);
	noiseElevation3a->type(FL_HOR_NICE_SLIDER);
	noiseElevation3a->textsize(12);

	noiseElevation3b->box(FL_UP_BOX);
	noiseElevation3b->range(0.0, 50.0);
	noiseElevation3b->step(0.5);
	noiseElevation3b->value(height3b);
	noiseElevation3b->callback(sliderEH3BCallback, noiseElevation3b);
	noiseElevation3b->type(FL_HOR_NICE_SLIDER);
	noiseElevation3b->textsize(12);

	noiseElevation3c->box(FL_UP_BOX);
	noiseElevation3c->range(0.0, 50.0);
	noiseElevation3c->step(0.5);
	noiseElevation3c->value(height3c);
	noiseElevation3c->callback(sliderEH3CCallback, noiseElevation3c);
	noiseElevation3c->type(FL_HOR_NICE_SLIDER);
	noiseElevation3c->textsize(12);

	// Slider for noise power constant
	sldEPower->box(FL_UP_BOX);
	sldEPower->range(0.0, 5.0);
	sldEPower->step(0.01);
	sldEPower->value(heightPow);
	sldEPower->type(FL_HOR_NICE_SLIDER);
	sldEPower->textsize(12);
	sldEPower->labelsize(14);
	sldEPower->labelfont(FL_HELVETICA + FL_BOLD);
	sldEPower->callback(sldEPowerCCallback, sldEPower);
	sldEPower->align(FL_ALIGN_TOP_LEFT);

	// Sliders for sea bed adjustments
	sldSeaNoiseA->box(FL_UP_BOX);
	sldSeaNoiseA->range(0.0, 5.0);
	sldSeaNoiseA->step(0.01);
	sldSeaNoiseA->value(slHeighta);
	sldSeaNoiseA->type(FL_HOR_NICE_SLIDER);
	sldSeaNoiseA->textsize(12);
	sldSeaNoiseA->labelsize(14);
	sldSeaNoiseA->callback(sldSeaNoiseACallback, sldSeaNoiseA);
	sldSeaNoiseA->labelfont(FL_HELVETICA + FL_BOLD);
	sldSeaNoiseA->align(FL_ALIGN_TOP_LEFT);

	sldSeaNoiseB->box(FL_UP_BOX);
	sldSeaNoiseB->range(0.0, 50.0);
	sldSeaNoiseB->step(0.5);
	sldSeaNoiseB->value(slHeightb);
	sldSeaNoiseB->callback(sldSeaNoiseBCallback, sldSeaNoiseB);
	sldSeaNoiseB->type(FL_HOR_NICE_SLIDER);
	sldSeaNoiseB->textsize(12);

	sldSeaNoiseC->box(FL_UP_BOX);
	sldSeaNoiseC->range(0.0, 50.0);
	sldSeaNoiseC->step(0.5);
	sldSeaNoiseC->value(slHeightc);
	sldSeaNoiseC->callback(sldSeaNoiseCCallback, sldSeaNoiseC);
	sldSeaNoiseC->type(FL_HOR_NICE_SLIDER);
	sldSeaNoiseC->textsize(12);

	// Slider for height multiplier
	sldHMult->box(FL_UP_BOX);
	sldHMult->range(0.0, 50.0);
	sldHMult->step(0.5);
	sldHMult->value(heightPow);
	sldHMult->type(FL_HOR_NICE_SLIDER);
	sldHMult->textsize(12);
	sldHMult->labelsize(14);
	sldHMult->callback(sldHMultCallback, sldHMult);
	sldHMult->labelfont(FL_HELVETICA + FL_BOLD);
	sldHMult->align(FL_ALIGN_TOP_LEFT);

	// the noise settings panel itself
	adjustNoise->set_modal();
	adjustNoise->remove(seaLevel);
	adjustNoise->remove(mainWindow);
	adjustNoise->add(lblNoiseGen);
	adjustNoise->add(noiseElevation1a);
	adjustNoise->add(noiseElevation1b);
	adjustNoise->add(noiseElevation1c);
	adjustNoise->add(noiseElevation2a);
	adjustNoise->add(noiseElevation2b);
	adjustNoise->add(noiseElevation2c);
	adjustNoise->add(noiseElevation3a);
	adjustNoise->add(noiseElevation3b);
	adjustNoise->add(noiseElevation3c);
	adjustNoise->add(sldEPower);
	adjustNoise->add(sldSeaNoiseA);
	adjustNoise->add(sldSeaNoiseB);
	adjustNoise->add(sldSeaNoiseC);
	adjustNoise->add(sldHMult);
	adjustNoise->add(btnNoiseBack);
	adjustNoise->end();
	adjustNoise->show();
	adjustNoise->hide();

	return Fl::run();
	
}