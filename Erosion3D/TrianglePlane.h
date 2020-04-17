#pragma once
#include <glad/glad.h>
#include <iostream>

class TrianglePlane
{
	int width;
	int height;
	int indices_size_;
	int vertices_size_;
	unsigned int VBO, VAO, EBO;
	std::unique_ptr<unsigned int[]> indices;
	std::unique_ptr<float[]> vertices;
	double* heightpointer;

public:
	int size;

	TrianglePlane(int pwidth, int pheight)
	{
		width = pwidth;
		height = pheight;
		indices_size_ = getIndicesCount();
		vertices_size_ = getVerticesCount();
		indices.reset(new unsigned int[indices_size_]);
		vertices.reset(new float[vertices_size_]);

		getIndices();
		getVertices();
		Create();
	}

	TrianglePlane(int pwidth, int pheight, double* HeightPointer)
	{
		width = pwidth;
		height = pheight;
		indices_size_ = getIndicesCount();
		vertices_size_ = getVerticesCount();
		indices.reset(new unsigned int[indices_size_]);
		vertices.reset(new float[vertices_size_]);
		heightpointer = HeightPointer;

		getIndices();
		getVerticesPointer();

		Create();
	}

	void Draw()
	{
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices_size_, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void SetHeight(int index, float height)
	{
		vertices[index * 3 + 2] = height;
	}

	float GetHeight(int index)
	{
		return  vertices[index * 3 + 2];
	}

	void UpdateBuffer()
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_size_, &vertices[0], GL_DYNAMIC_DRAW);
	}

private:
	int getVerticesCount() {
		size = (width + 1) * (height + 1);
		return size * 3;
	}

	int getIndicesCount() {
		return width * height * 6;
	}

	void getVertices() {
		int i = 0;
		for (int row = 0; row < height + 1; row++) {
			for (int col = 0; col < width + 1; col++) {
				vertices[i++] = static_cast<float>(col);
				vertices[i++] = static_cast<float>(row);
				vertices[i++] = 0.0;
			}
		}
	}

	void getVerticesPointer() {
		int i = 0;
		int p = 0;

		for (int row = 0; row < height + 1; row++) {
			for (int col = 0; col < width + 1; col++) {
				vertices[i++] = static_cast<float>(col);
				vertices[i++] = static_cast<float>(row);
				vertices[i++] = static_cast<float>(heightpointer[p++] * 100.0);
			}
		}
	}

	void getIndices() {
		int a = 0;
		int i = 0;
		for (int h = 0; h < height; h++)
		{
			for (int w = 0; w < width; w++)
			{
				indices[i++] = h * width + w + a;
				indices[i++] = h * width + w + 1 + a;
				indices[i++] = (h + 1) * width + w + 1 + a;

				indices[i++] = h * width + w + 1 + a;
				indices[i++] = (h + 1) * width + w + 2 + a;
				indices[i++] = (h + 1) * width + w + 1 + a;
			}
			a++;
		}
	}

	void Create()
	{
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices_size_, &vertices[0], GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices_size_, &indices[0], GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);


		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(0);
	}
};
