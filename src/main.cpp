#include <array>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <optional>
#include <tuple>
#include <vector>

#include <stb_image.h>

#include <glad/glad.h>

#include <GLFW/glfw3.h>

GLuint create_shader( GLenum type, std::string_view src );
GLuint create_program( std::vector<GLuint> shaders );
GLuint create_texture( int w, int h, void * data );
GLuint create_draw_data();

GLuint load_texture( std::string_view path );

void render();

int main( int argc, char const * argv[] )
{
  ::glfwInit();
  ::glfwSetErrorCallback( []( auto... args ) { ( std::cout << ... << args ) << std::endl; } );

  ::glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );

  ::glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
  ::glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 5 );

  ::glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

  GLFWwindow * window = ::glfwCreateWindow( 800, 600, "window", {}, {} );
  ::glfwMakeContextCurrent( window );
  ::gladLoadGL();

  ::glDebugMessageCallback( []( auto... args ) { ( std::cout << ... << args ) << std::endl; }, {} );

  ::glEnable( GL_DEPTH_TEST );

  GLuint texture = load_texture( "assets/3plzl01l.jpg" );

  ::glActiveTexture( GL_TEXTURE0 );
  ::glBindTexture( GL_TEXTURE_2D, texture );

  GLuint vertex_shader = create_shader( GL_VERTEX_SHADER, R"(
      #version 450 core

      layout(location = 0) in vec4 position;
      layout(location = 1) in vec2 texcoord;

      out vec2 st;

      void main()
      {
        gl_Position = position;
        st = texcoord;
      }
  )" );
  
  GLuint fragment_shader = create_shader( GL_FRAGMENT_SHADER, R"(
      #version 450 core

      layout(location = 0) out vec4 color;

      layout(location = 1) in vec2 st;
      layout(binding = 0) uniform sampler2D tex0;

      void main()
      {
        color = texture(tex0, st);
      }
  )" );

  GLuint vao = create_draw_data();
  GLuint program = create_program( {fragment_shader, vertex_shader} );

  while ( !::glfwWindowShouldClose( window ) )
  {
    {
      ::glUseProgram( program );
      ::glBindVertexArray( vao );
      render();
    }

    ::glfwPollEvents();
    ::glfwSwapBuffers( window );
  }

  return 0;
}

GLuint create_program( std::vector<GLuint> shaders )
{
  GLuint program = ::glCreateProgram();
  
  for ( GLuint shader : shaders )
    glAttachShader( program, shader );

  glLinkProgram( program );

  {
    GLchar log[2048]{};
    GLsizei length = 0;
    glGetProgramInfoLog(program, std::size(log), &length, std::data(log) );

    if ( length )
      std::cout << log << std::endl;
  }

  return program;
}

GLuint create_shader( GLenum type, std::string_view src )
{
    GLuint shader = glCreateShader( type );

    {
      GLint  length = std::size( src );
      auto * data   = std::data( src );

      glShaderSource( shader, 1, &data, &length );
      glCompileShader( shader );
    }

    {
      GLchar  log[2048]{};
      GLsizei length = 0;
      glGetShaderInfoLog( shader, std::size( log ), &length, std::data( log ) );

      if ( length )
        std::cout << log << std::endl;
    }

    return shader;
}

GLuint create_texture( int w, int h, void * data )
{
  GLuint texture = 0;
  glGenTextures( 1, &texture );
  glBindTexture( GL_TEXTURE_2D, texture );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data );
  glGenerateMipmap( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0 );

  return texture;
}

GLuint load_texture( std::string_view path )
{
  int w = 0;
  int h = 0;
  int n = 0;
  auto   data    = stbi_load( std::data(path), &w, &h, &n, 0 );
  GLuint texture = create_texture( w, h, data );
  stbi_image_free( data );

  return texture;
}


GLuint create_draw_data()
{
  using vec4 = std::array<GLfloat, 4>;
  using vec2 = std::array<GLfloat, 2>;
  using vertex = struct { vec4 v; vec2 st; };
  
  constexpr vertex vertices[] = {
    { -1, -1, 0, 1, {0, 1} },
    {  1, -1, 0, 1, {1, 1} },
    {  0,  1, 0, 1, {0, 0} },
  };

  GLuint buffer = 0;
  GLsizei buffer_size = std::size(vertices) * sizeof(vertex);
  glCreateBuffers( 1, &buffer );
  glNamedBufferData( buffer, buffer_size, nullptr, GL_STATIC_DRAW );

  auto data = (vertex*)glMapNamedBufferRange( buffer, 0, buffer_size, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT );
  std::copy_n( vertices, std::size(vertices), data );
  glUnmapNamedBuffer( buffer );

  GLuint vao = 0;
  glCreateVertexArrays( 1, &vao );

  // vertices
  GLuint index = 0;
  GLuint binding = 0;
  glEnableVertexArrayAttrib( vao, index );
  glVertexArrayAttribFormat( vao, index, 4, GL_FLOAT, GL_FALSE, 0 );
  glVertexArrayAttribBinding( vao, index, binding );
  glVertexArrayVertexBuffer( vao, binding, buffer, offsetof(vertex, v), sizeof(vertex) );

  // texture coords
  index = 1;
  binding = 1;
  glEnableVertexArrayAttrib( vao, index );
  glVertexArrayAttribFormat( vao, index, 2, GL_FLOAT, GL_FALSE, 0 );
  glVertexArrayAttribBinding( vao, index, binding );
  glVertexArrayVertexBuffer( vao, binding, buffer, offsetof(vertex, st), sizeof(vertex) );

  return vao;
}

void render()
{
  using vec4 = std::array<GLfloat, 4>;

  vec4 color{0.75, 0, 0, 0};
  glClearBufferfv( GL_COLOR, 0, std::data( color ) );

  GLfloat depth = 1.f;
  glClearBufferfv( GL_DEPTH, 0, &depth );

  glDrawArrays( GL_TRIANGLES, 0, 3 );
}
