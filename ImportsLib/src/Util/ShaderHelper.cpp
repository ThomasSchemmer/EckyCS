#include "ShaderHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include "./stb/stb_image.h"

namespace Util
{
    using namespace std;
    
    string ShaderHelper::LoadShader(const string& FilePath) {
        ifstream ShaderFile;
        string ShaderCode;

        ShaderFile.exceptions(ifstream::failbit | ifstream::badbit);

        try {
            auto AbsolutePath = filesystem::absolute(FilePath);
            cout << "Loading Shader at: " << AbsolutePath << "\n";
    
            ShaderFile.open(FilePath);
            stringstream ShaderStream;
            ShaderStream << ShaderFile.rdbuf();
            ShaderFile.close();
            ShaderCode = ShaderStream.str();
        } catch (ifstream::failure& e) {
            cerr << "ERROR::SHADER::FILE_MISSING: " << e.what() << "\n";
        }
        return ShaderCode;
    };

    
    unsigned int ShaderHelper::CompileShader(const string& Code, GLenum Type)
    {
        int Success;
        unsigned int ID = glCreateShader(Type);
        const char* SourceCode = Code.c_str();
        glShaderSource(ID, 1, &SourceCode, nullptr);
        glCompileShader(ID);
        glGetShaderiv(ID, GL_COMPILE_STATUS, &Success);
        if (!Success)
        {
            char InfoLog[512];
            glGetShaderInfoLog(ID, 512, nullptr, InfoLog);
            cout << "ERROR::SHADER::COMPILATION_FAILED\n" << InfoLog << "\n";
        }
        return ID;
    }

    unsigned int ShaderHelper::CreateProgram(const vector<unsigned int>& ShaderIDs)
    {
        unsigned int ID = glCreateProgram();
        for (auto ShaderID : ShaderIDs)
        {
            glAttachShader(ID, ShaderID);
        }
        glLinkProgram(ID);

        int Success;
        glGetProgramiv(ID, GL_COMPILE_STATUS, &Success);
        if (!Success)
        {
            char InfoLog[512];
            glGetProgramInfoLog(ID, 512, nullptr, InfoLog);
            cout << "ERROR::SHADER::PROGRAM_CREATION_FAILED\n" << InfoLog << "\n";
        }
        
        glValidateProgram(ID);
        glGetProgramiv(ID, GL_VALIDATE_STATUS, &Success);
        if (!Success) {
            char infoLog[512];
            glGetProgramInfoLog(ID, 512, nullptr, infoLog);
            cerr << "ERROR::SHADER::PROGRAM_VALIDATION_FAILED\n" << infoLog << "\n";
        }
        
        return ID;
    }

    unsigned int ShaderHelper::CreateTexture(const string& FilePath, GLint Format)
    {
        int Width, Height, CountChannels;
        unsigned char* Data = stbi_load(FilePath.c_str(), &Width, &Height, &CountChannels, 0);
        if (!Data)
        {
            cerr << "ERROR::SHADER::TEX_FILE_MISSING: " << FilePath << "\n";
        }

        unsigned int ID;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(Data);
        return ID;
    }

    void ShaderHelper:: SetUniform1f(const string& UniformName, float Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1f(ID, Value);
    }

    void ShaderHelper:: SetUniformM4(const string& UniformName, const glm::mat4& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniformMatrix4fv(ID, 1, GL_FALSE, value_ptr(Value));
    }


    void ShaderHelper:: SetUniform3fv(const string& UniformName, const glm::vec3& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform3fv(ID, 1, value_ptr(Value));
    }

    void ShaderHelper:: ShaderHelper::SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1i(ID, Slot);
        glActiveTexture(GL_TEXTURE0 + Slot);
        glBindTexture(GL_TEXTURE_2D, TextureID);
    }
}