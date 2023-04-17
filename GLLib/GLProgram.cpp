/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "stdafx.h"
#include "GLProgram.h"
#include <FSCore/FSLogger.h>
#include <GL/glew.h>

GLProgram::GLProgram()
{
	m_progId = 0;
}

bool compileShader(unsigned int shaderid, const char* shadersrc)
{
	// set the shader text
	glShaderSource(shaderid, 1, &shadersrc, NULL);

	// compile the shader
	int success;
	glCompileShader(shaderid);
	glGetShaderiv(shaderid, GL_COMPILE_STATUS, &success);
	if (success == 0) FSLogger::Write("shader compilation failed:\n");
	else FSLogger::Write("shader compilation succeeded.\n");
	GLint length = 0;
	glGetShaderiv(shaderid, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		GLchar* buf = new GLchar[length + 1];
		glGetShaderInfoLog(shaderid, length + 1, NULL, buf);
		FSLogger::Write(buf);
		delete[] buf;
	}

	return (success != 0);
}

bool GLProgram::Create(const char* szvert, const char* szfrag)
{
	// create the fragment shader
	GLuint vertShader = 0, fragShader = 0;

	if (szvert)
	{
		vertShader = glCreateShader(GL_VERTEX_SHADER);
		compileShader(vertShader, szvert);
	}

	if (szfrag)
	{
		fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragShader, szfrag);
	}

	// create the program
	m_progId = glCreateProgram();

	if (vertShader > 0) glAttachShader(m_progId, vertShader);
	if (fragShader > 0) glAttachShader(m_progId, fragShader);

	// link program
	int success = 0;
	glLinkProgram(m_progId);
	glGetProgramiv(m_progId, GL_LINK_STATUS, &success);
	if (success == 0) FSLogger::Write("Failed linking program.");
	GLint length = 0;
	glGetProgramiv(m_progId, GL_INFO_LOG_LENGTH, &length);
	if (length > 0)
	{
		GLchar* buf = new GLchar[length + 1];
		glGetProgramInfoLog(m_progId, length + 1, NULL, buf);
		FSLogger::Write(buf);
		delete[] buf;
	}

	// cleanup
	if (vertShader > 0) glDeleteShader(vertShader);
	if (fragShader > 0) glDeleteShader(fragShader);

	return (success != 0);
}

void GLProgram::Use()
{
	if (m_progId > 0) glUseProgram(m_progId);
}

void GLProgram::SetInt(const char* szparam, int val)
{
	GLint id = glGetUniformLocation(m_progId, szparam);
	glUniform1i(id, val);
}

void GLProgram::SetFloat(const char* szparam, float val)
{
	GLint id = glGetUniformLocation(m_progId, szparam);
	glUniform1f(id, val);
}

void GLProgram::SetFloat3(const char* szparam, float v[3])
{
	GLint id = glGetUniformLocation(m_progId, szparam);
	glUniform3f(id, v[0], v[1], v[2]);
}
