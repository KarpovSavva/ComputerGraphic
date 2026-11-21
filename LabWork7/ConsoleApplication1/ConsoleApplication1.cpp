// ConsoleApplication1.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
#define GLEW_DLL
#define GLFW_DLL
#include <iostream>
#include "GL\glew.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace std;
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
};

class Mesh {
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	GLuint VAO, VBO, EBO;

	Mesh(vector<Vertex> vertices, vector<unsigned int> indices) {
		this->vertices = vertices;
		this->indices = indices;
		setupMesh();
	}

	void Draw(GLuint shader) {
		glUseProgram(shader);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

private:
	void setupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

		glBindVertexArray(0);
	}
};

class Model {
public:
	vector<Mesh> meshes;
	string directory;
	Model(string const& path) {
		loadModel(path);
	}
	void Draw(GLuint shader) {
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].Draw(shader);
	}
private:
	void loadModel(string const& path) {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}
		directory = path.substr(0, path.find_last_of('/'));
		processNode(scene->mRootNode, scene);
	}
	void processNode(aiNode* node, const aiScene* scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector;
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			if (mesh->HasNormals()) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}
			vertices.push_back(vertex);
		}
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		return Mesh(vertices, indices);
	}
};
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 256.0f;
float lastY = 256.0f;
bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);
	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
	yaw += xoffset;
	pitch += yoffset;
	if (pitch > 89.0f) pitch = 89.0f;
	if (pitch < -89.0f) pitch = -89.0f;
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
int main() {
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3.\n");
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window;
	window = glfwCreateWindow(512, 512, "Mainwindow", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glewExperimental = GL_TRUE;
	GLenum ret = glewInit();
	if (GLEW_OK != ret) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(ret));
		return 1;
	}
	glEnable(GL_DEPTH_TEST);
	const char* vert_shader =
		"#version 410 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec3 aNormal;\n"
		"uniform mat4 model;\n"
		"uniform mat4 view;\n"
		"uniform mat4 projection;\n"
		"uniform mat3 normalMatrix;\n"
		"out vec3 FragPos;\n"
		"out vec3 Normal;\n"
		"void main() {\n"
		"   FragPos = vec3(model * vec4(aPos, 1.0));\n"
		"   Normal = normalMatrix * aNormal;\n"
		"   gl_Position = projection * view * vec4(FragPos, 1.0);\n"
		"}\n";
	const char* frag_shader =
		"#version 410 core\n"
		"out vec4 frag_colour;\n"
		"in vec3 FragPos;\n"
		"in vec3 Normal;\n"
		"uniform vec3 viewPos;\n"
		"struct Material {\n"
		"   vec3 ambient;\n"
		"   vec3 diffuse;\n"
		"   vec3 specular;\n"
		"   float shininess;\n"
		"};\n"
		"uniform Material material;\n"
		"struct Light {\n"
		"   vec3 position;\n"
		"   vec3 ambient;\n"
		"   vec3 diffuse;\n"
		"   vec3 specular;\n"
		"};\n"
		"uniform Light light;\n"
		"void main() {\n"
		"   vec3 ambient = light.ambient * material.ambient;\n"
		"   vec3 norm = normalize(Normal);\n"
		"   vec3 lightDir = normalize(light.position - FragPos);\n"
		"   float diff = max(dot(norm, lightDir), 0.0);\n"
		"   vec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
		"   vec3 viewDir = normalize(viewPos - FragPos);\n"
		"   vec3 reflectDir = reflect(-lightDir, norm);\n"
		"   float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
		"   vec3 specular = light.specular * (spec * material.specular);\n"
		"   vec3 result = ambient + diffuse + specular;\n"
		"   frag_colour = vec4(result, 1.0);\n"
		"}\n";
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vert_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &frag_shader, NULL);
	glCompileShader(fs);
	GLuint shader_program = glCreateProgram();
	glAttachShader(shader_program, vs);
	glAttachShader(shader_program, fs);
	glLinkProgram(shader_program);
	int modelLoc = glGetUniformLocation(shader_program, "model");
	int viewLoc = glGetUniformLocation(shader_program, "view");
	int projLoc = glGetUniformLocation(shader_program, "projection");
	int colorLoc = glGetUniformLocation(shader_program, "objectColor");
	int normalMatrixLoc = glGetUniformLocation(shader_program, "normalMatrix");
	int viewPosLoc = glGetUniformLocation(shader_program, "viewPos");
	int matAmbientLoc = glGetUniformLocation(shader_program, "material.ambient");
	int matDiffuseLoc = glGetUniformLocation(shader_program, "material.diffuse");
	int matSpecularLoc = glGetUniformLocation(shader_program, "material.specular");
	int matShininessLoc = glGetUniformLocation(shader_program, "material.shininess");
	int lightPosLoc = glGetUniformLocation(shader_program, "light.position");
	int lightAmbientLoc = glGetUniformLocation(shader_program, "light.ambient");
	int lightDiffuseLoc = glGetUniformLocation(shader_program, "light.diffuse");
	int lightSpecularLoc = glGetUniformLocation(shader_program, "light.specular");
	Model ourModel("Lab_3_VAR_2.obj");
	glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
	float objectRotY = 0.0f;
	float objectScale = 1.0f;
	float lastFrame = 0.0f;
	float angle1 = 0.0f;
	float angle2 = 0.0f;
	float angle3 = 0.0f;
	float angle4 = 0.0f;
	glm::vec3 pivot1 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 pivot2 = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 pivot3 = glm::vec3(0.0f, 1.0f, -0.2f);
	glm::vec3 pivot4 = glm::vec3(0.0f, 0.87f, -0.9f);
	while (!glfwWindowShouldClose(window)) {
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float transformSpeed = 2.0f * deltaTime;
		glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float cameraSpeed = 0.05f;
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			cameraPos += cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			cameraPos -= cameraSpeed * cameraFront;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
			objectPos.z -= transformSpeed;
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
			objectPos.z += transformSpeed;
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			objectPos.x -= transformSpeed;
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			objectPos.x += transformSpeed;
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
			objectRotY -= 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			objectRotY += 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
			objectScale -= 0.5f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
			objectScale += 0.5f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) angle1 += 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) angle1 -= 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) angle2 += 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) angle2 -= 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) angle3 += 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) angle3 -= 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) angle4 += 50.0f * deltaTime;
		if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) angle4 -= 50.0f * deltaTime;
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, objectPos);
		model = glm::rotate(model, glm::radians(objectRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(objectScale));
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 512.0f / 512.0f, 0.1f, 100.0f);	
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
		glUseProgram(shader_program);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
		glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
		glUniform3f(matAmbientLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(matDiffuseLoc, 1.0f, 0.5f, 0.31f);
		glUniform3f(matSpecularLoc, 0.5f, 0.5f, 0.5f);
		glUniform1f(matShininessLoc, 32.0f);
		glUniform3f(lightPosLoc, cameraPos.x + 1.0f, cameraPos.y + 1.0f, cameraPos.z);
		glUniform3f(lightAmbientLoc, 0.2f, 0.2f, 0.2f);  
		glUniform3f(lightDiffuseLoc, 0.8f, 0.8f, 0.8f);  
		glUniform3f(lightSpecularLoc, 1.0f, 1.0f, 1.0f); 
		glm::mat4 globalTransform = glm::translate(glm::mat4(1.0f), objectPos);
		globalTransform = glm::rotate(globalTransform, glm::radians(objectRotY), glm::vec3(0.0f, 1.0f, 0.0f));
		globalTransform = glm::scale(globalTransform, glm::vec3(objectScale));
		glm::mat4 model1 = globalTransform;
		model1 = glm::translate(model1, pivot1);
		model1 = glm::rotate(model1, glm::radians(angle1), glm::vec3(0.0f, 1.0f, 0.0f));
		model1 = glm::translate(model1, -pivot1);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model1));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(model1)))));
		if (ourModel.meshes.size() > 0) ourModel.meshes[0].Draw(shader_program);
		glm::mat4 model2 = model1; 
		model2 = glm::translate(model2, pivot2);
		model2 = glm::rotate(model2, glm::radians(angle2), glm::vec3(0.0f, 1.0f, 0.0f));
		model2 = glm::translate(model2, -pivot2);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model2));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(model2)))));
		if (ourModel.meshes.size() > 1) ourModel.meshes[1].Draw(shader_program);
		glm::mat4 model3 = model2; 
		model3 = glm::translate(model3, pivot3);
		model3 = glm::rotate(model3, glm::radians(angle3), glm::vec3(1.0f, 0.0f, 0.0f));
		model3 = glm::translate(model3, -pivot3);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model3));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(model3)))));
		if (ourModel.meshes.size() > 2) ourModel.meshes[2].Draw(shader_program);
		glm::mat4 model4 = model3; 
		model4 = glm::translate(model4, pivot4);
		model4 = glm::rotate(model4, glm::radians(angle4), glm::vec3(1.0f, 0.0f, 0.0f));
		model4 = glm::translate(model4, -pivot4);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model4));
		glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(glm::transpose(glm::inverse(glm::mat3(model4)))));
		if (ourModel.meshes.size() > 3) ourModel.meshes[3].Draw(shader_program);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}