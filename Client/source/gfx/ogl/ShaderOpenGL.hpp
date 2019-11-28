#pragma once
#include "../interface/Shader.hpp"
#include "util/IO.hpp"
#include "glad/glad.h"
#include <glm/gtc/type_ptr.hpp>

namespace gfx
{
	class ShaderOpenGL : public Shader
	{
	public:
		std::string vertexSource;
		std::string fragmentSource;

		void init(const std::string& fileName, bool source) override
		{
			if (!source) { throw std::invalid_argument("Precompiled shaders are not supported"); }

			auto vertData = readFile("shaders" + seperator() + fileName + ".vert.glsl");
			if (!vertData.empty())
			{
				vertexSource = std::string(vertData.begin(), vertData.end());
			}

			auto fragData = readFile("shaders" + seperator() + fileName + ".frag.glsl");
			if (!fragData.empty())
			{
				fragmentSource = std::string(fragData.begin(), fragData.end());
			}
		};

		void compile() override
		{
			if (vertexSource.empty()) { throw std::invalid_argument("Shader vertex source is empty"); }
			uint32 vertShader = glCreateShader(GL_VERTEX_SHADER);
			auto* source = (const GLchar*)vertexSource.c_str();
			glShaderSource(vertShader, 1, &source, 0);
			glCompileShader(vertShader);
			int compiled = 0;
			glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				int maxLength = 0;
				glGetShaderiv(vertShader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<char> log(maxLength);
				glGetShaderInfoLog(vertShader, maxLength, &maxLength, &log[0]);
				glDeleteShader(vertShader);
				throw std::invalid_argument(log.data());
			}

			if (fragmentSource.empty()) { throw std::invalid_argument("Shader fragment source is empty"); }
			uint32 fragShader = glCreateShader(GL_FRAGMENT_SHADER);
			source = (const GLchar*)fragmentSource.c_str();
			glShaderSource(fragShader, 1, &source, 0);
			glCompileShader(fragShader);
			compiled = 0;
			glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
			if (!compiled)
			{
				int maxLength = 0;
				glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<char> log(maxLength);
				glGetShaderInfoLog(fragShader, maxLength, &maxLength, &log[0]);
				glDeleteShader(fragShader);
				throw std::invalid_argument(log.data());
			}

			id = glCreateProgram();
			glAttachShader(id, vertShader);
			glAttachShader(id, fragShader);
			glLinkProgram(id);

			int linked = 0;
			glGetProgramiv(id, GL_LINK_STATUS, (int*)&linked);
			if (linked == GL_FALSE)
			{
				int maxLength = 0;
				glGetProgramiv(id, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<char> log(maxLength);
				glGetProgramInfoLog(id, maxLength, &maxLength, &log[0]);
				glDeleteProgram(id);
				glDeleteShader(vertShader);
				glDeleteShader(fragShader);
				throw std::invalid_argument(log.data());
			}

			glDetachShader(id, vertShader);
			glDetachShader(id, fragShader);
		};

		void cleanup() override
		{
			glDeleteProgram(id);
		};

		void bind() override
		{
			glUseProgram(id);
		};

		void unbind() override
		{
			glUseProgram(0);
		};

		void uploadMat4(const std::string& name, const mat4& matrix) override
		{
			int loc = glGetUniformLocation(id, (const GLchar*)name.c_str());
			glUniformMatrix4fv(loc, 1, false, glm::value_ptr(matrix));
		};
	};
};