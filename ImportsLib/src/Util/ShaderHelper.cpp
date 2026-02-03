#include "ShaderHelper.h"

#define STB_IMAGE_IMPLEMENTATION
#include <codecvt>

#include "./stb/stb_image.h"

#include <windows.h>
#include <string>

namespace Util
{
    using namespace std;

    wstring ShaderHelper::ToWString(const string& str)
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
                                              (int)str.size(), nullptr, 0);
        wstring result(size_needed, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.c_str(),
                            (int)str.size(), result.data(), size_needed);
        return result;
    }

    string ShaderHelper::ToString(const wchar_t* wchar)
    {
        std::wstring ws(wchar);
        std::string s_str;
        s_str.reserve(ws.length());
        ranges::transform(ws, std::back_inserter(s_str),
                          [](wchar_t c) { return static_cast<char>(c); });
        return s_str;
    }

    string ShaderHelper::LoadShaderFromResource(const wchar_t* Name)
    {
        return LoadResourceIncludes(ResourceToString(Name));
    }
    
    string ShaderHelper::ResourceToString(const wchar_t* ResourcePath)
    {
        auto Tuple = ResourceToData(ResourcePath);
        auto Data = get<0>(Tuple);
        auto Size = get<1>(Tuple);
        
        // Strip UTF-8 BOM if present
        size_t offset = 0;
        if (Size >= 3 && Data[0] == 0xEF && Data[1] == 0xBB && Data[2] == 0xBF) {
            offset = 3;
        }

        return std::string(reinterpret_cast<const char*>(Data + offset),
                           Size - offset);
    }

    tuple<const unsigned char*, size_t> ShaderHelper::ResourceToData(const wchar_t* ResourcePath)
    {
        HRSRC rc = FindResourceW(nullptr, ResourcePath, RT_RCDATA);
        if (!rc) {
            wcerr << "ERROR::SHADER::FILE_MISSING: " << ResourcePath << "\n";
            return make_tuple(nullptr, 0);
        }

        HGLOBAL h = LoadResource(nullptr, rc);
        DWORD size = SizeofResource(nullptr, rc);
        const unsigned char* Data = static_cast<const unsigned char*>(LockResource(h));
        return make_tuple(Data, size);
    }

    string ShaderHelper::LoadResourceIncludes(const string& Code)
    {
        stringstream In(Code);
        stringstream Out;
        string Line;

        while (getline(In, Line))
        {
            string trimmed = Line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));

            if (trimmed.starts_with("#include"))
            {
                size_t firstQuote = trimmed.find('"');
                size_t lastQuote  = trimmed.find_last_of('"');
                if (firstQuote == string::npos || firstQuote == lastQuote)
                    throw runtime_error("Malformed #include line: " + Line);

                string Name = trimmed.substr(firstQuote + 1, lastQuote - firstQuote - 1);
                auto WName = ToWString(Name);
                Out << LoadShaderFromResource(WName.c_str()) << "\n";
            }
            else
            {
                Out << Line << "\n";
            }
        }
        return Out.str();
    };

    
    
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
    }

    

    
    unsigned int ShaderHelper::CompileShader(const string& Code, GLenum Type)
    {
        int Success;
        unsigned int ID = glCreateShader(Type);
        const char* SourceCode = Code.c_str();
        glShaderSource(ID, 1, &SourceCode, nullptr);
        glCompileShader(ID);
        glGetShaderiv(ID, GL_COMPILE_STATUS, &Success);
        GLint status = GL_FALSE;
        glGetShaderiv(ID, GL_COMPILE_STATUS, &status);

        GLint logLen = 0;
        glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &logLen);

        if (logLen > 0) {
            std::vector<char> log(logLen);
            glGetShaderInfoLog(ID, logLen, nullptr, log.data());
            std::cerr << log.data() << std::endl;
        }
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

        unsigned int ID = CreateTextureInternal(Width, Height, Data, Format);
        stbi_image_free(Data);
        return ID;
    }

    unsigned int ShaderHelper::CreateTexture(const wchar_t* FilePath, GLint Format)
    {
        auto StringPath = ToString(FilePath);
        auto FileContent = ResourceToString(FilePath);
        int Width, Height, CountChannels;
        unsigned char* Data = stbi_load_from_memory(
            reinterpret_cast<const unsigned char*>(FileContent.c_str()), 
        static_cast<int>(FileContent.length()),    
            &Width, &Height, &CountChannels, 0
        );
        if (!Data)
        {
            cerr << "ERROR::SHADER::TEX_FILE_MISSING: " << StringPath << "\n";
        }

        unsigned int ID = CreateTextureInternal(Width, Height, Data, Format);
        stbi_image_free(Data);
        return ID;
    }

    vector<GLuint> ShaderHelper::CreateTextureArray(vector<const wchar_t*>& FilePaths, GLsizei Width, GLsizei Height, GLint Format)
    {
        vector<GLuint> Texs;
        Texs.reserve(FilePaths.size() + 1);

        // create the array itself with global configs
        GLuint TempArrayID;
        glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &TempArrayID);
        glTextureStorage3D(
            TempArrayID, 1, GL_RGBA8, Width, Height, FilePaths.size()
        );
        glTextureParameteri(TempArrayID, GL_TEXTURE_MIN_FILTER, GL_POINT);
        glTextureParameteri(TempArrayID, GL_TEXTURE_MAG_FILTER, GL_POINT);
        glTextureParameteri(TempArrayID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(TempArrayID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        Texs.push_back(TempArrayID);

        // now try to load each tex and load it into the array
        unsigned int Layer = 0;
        for (const auto& FilePath : FilePaths)
        {
            GLuint TempID = CreateTexture(FilePath, Format);
            glCopyImageSubData(
                TempID, 
                GL_TEXTURE_2D, 0, 0, 0, 0,
                TempArrayID,
                GL_TEXTURE_2D_ARRAY, 0, 0, 0, Layer,  
                Width, Height, 1 
            );
            Texs.push_back(TempID);
            Layer++;
        }
        
        return Texs;
    }

    unsigned int ShaderHelper::CreateTextureInternal(int Width, int Height, const unsigned char* Data, GLint Format)
    {
        unsigned int ID;
        glGenTextures(1, &ID);
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, Format, Width, Height, 0, Format, GL_UNSIGNED_BYTE, Data);
        glGenerateMipmap(GL_TEXTURE_2D);
        return ID; 
    }

    void ShaderHelper::SetUniform1f(const string& UniformName, float Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1f(ID, Value);
    }

    void ShaderHelper::SetUniform1i(const string& UniformName, int Value, unsigned int Program)
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1i(ID, Value);
    }

    void ShaderHelper::SetUniform1ui(const string& UniformName, unsigned int Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1ui(ID, Value);
    }

    void ShaderHelper::SetUniformM4(const string& UniformName, const glm::mat4& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniformMatrix4fv(ID, 1, GL_FALSE, value_ptr(Value));
    }


    void ShaderHelper:: SetUniform3fv(const string& UniformName, const glm::vec3& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform3fv(ID, 1, value_ptr(Value));
    }
    
    void ShaderHelper:: SetUniform3fva(const string& UniformName, const vector<glm::vec3>& Values, int Count, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform3fv(ID, Count, value_ptr(Values[0]));
    }

    void ShaderHelper:: SetUniform2fv(const string& UniformName, const glm::vec2& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform2fv(ID, 1, value_ptr(Value));
    }

    void ShaderHelper:: SetUniform3iv(const string& UniformName, const glm::ivec3& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform3iv(ID, 1, value_ptr(Value));
    }

    void ShaderHelper:: SetUniform2iv(const string& UniformName, const glm::ivec2& Value, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform2iv(ID, 1, value_ptr(Value));
    }

    void ShaderHelper::SetUniformTexture(const string& UniformName, unsigned int TextureID, GLint Slot, unsigned int Program) 
    {
        const int ID = glGetUniformLocation(Program, UniformName.c_str());
        glUniform1i(ID, Slot);
        glActiveTexture(GL_TEXTURE0 + Slot);
        glBindTexture(GL_TEXTURE_2D, TextureID);
    }

    void ShaderHelper::ResetBufferCounter(GLuint Buffer)
    {
        // reset append counter
        GLuint zero = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, Buffer);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);
    }

    GLsizei ShaderHelper::ReadBufferCount(GLuint Buffer)
    {
        GLsizei value = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, Buffer);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &value);
        return value;
    }

    unsigned int ShaderHelper::CreateComputeProgram(const vector<const wchar_t*>& Resources)
    {
        vector<unsigned int> IDs;
        for (auto& Resource : Resources)
        {
            string ComputeCode = LoadShaderFromResource(Resource);
            unsigned int Compute = CompileShader(ComputeCode, GL_COMPUTE_SHADER);
            IDs.push_back(Compute);
        }

        auto Result = CreateProgram(IDs);
        for (auto& ID : IDs)
        {
            glDeleteShader(ID);
        }
        return Result;
    }
}
