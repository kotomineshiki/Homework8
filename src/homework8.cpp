#include <iostream>           // basic input output
#include <memory>             // for make_shared

#include <homework8.hpp>      // header file
#include <opengl_helper.hpp>  // helper library
#include<vector>
// glm library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace std;
#define DEBUG

// screen size
constexpr unsigned int kScreenWidth = 800;
constexpr unsigned int kScreenHeight = 600;
float animator = 0.4f;//用于控制动画进度的参数
// one has vec3 and total  4 points
int kEachPointsSize = 3;
int kMoursePointsSize = kEachPointsSize * 100;

// Mouse Input
GLfloat mouse_x, mouse_y;
int points_count = 0;
std::unique_ptr<GLfloat[]> mouse_points(new GLfloat[kMoursePointsSize]);
// ImGui
int total_points = 1000;
float color[3] { 1.0f, 0.5f, 0.2f };

// global values
GLuint points_vao[3];
// store window size
int width = kScreenWidth, height = kScreenHeight;

int C(int m, int n) {//n中选m
  int up = 1;
  int down = 1;
  for (int i = 1; i <= n; ++i) {
    up = up * (m - i+1);
    down = down * i;
  }

  return up / down;
}



int main() {
  // the GLFW window
  GLFWwindow* window;
  vector<GLfloat> lineResult;//储存线段过程
  auto getTPoint=[](glm::vec2 pointA, glm::vec2 pointB, float t) {
    return pointA + t * (pointB - pointA);//得到t等分点
  };

  std::function<void(vector<glm::vec2> InputPoints, float t)> CalculatePath
    =[&lineResult,getTPoint,&CalculatePath](vector<glm::vec2> InputPoints, float t) {
    //进行画线


    vector<glm::vec2> newPoints;
    for (int i = 0; i < InputPoints.size() - 1; ++i) {//
      glm::vec2 newPoint = getTPoint(InputPoints[i], InputPoints[i + 1], t);
      lineResult.push_back(InputPoints[i].x);
      lineResult.push_back(InputPoints[i].y);
      lineResult.push_back(0);

      lineResult.push_back(InputPoints[i + 1].x);
      lineResult.push_back(InputPoints[i + 1].y);
      lineResult.push_back(0);

      newPoints.push_back(newPoint);//得到了新的新点组合
    }
    if (newPoints.size() >= 2) {
      CalculatePath(newPoints, t);//进行下一轮的计算
    }
    else {
    }
  };
  
  // dirty work initial
  auto initial_window = [&window] {
    window = glfwCreateWindow(width, height, "homework8", NULL, NULL);
    helper::assert_true(window != NULL, "Failed to create GLFW windows");
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
  };

  // imgui
  auto create_imgui = []() {
    ImGui::Begin("Menu");
    ImGui::Text("Welcome");
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    ImGui::InputInt("Total Points", &points_count);

    ImGui::SliderFloat("t:", &animator, 0.0f, 1.0f);
    ImGui::InputFloat3("First  Point", mouse_points.get() + kEachPointsSize * 0);
    ImGui::InputFloat3("Second Point", mouse_points.get() + kEachPointsSize * 1);
    ImGui::InputFloat3("Third  Point", mouse_points.get() + kEachPointsSize * 2);
    ImGui::InputFloat3("Fourth Point", mouse_points.get() + kEachPointsSize * 3);
    ImGui::InputFloat3("Fifth Point" , mouse_points.get() + kEachPointsSize * 4);
    ImGui::InputFloat3("Sixth Point" , mouse_points.get() + kEachPointsSize * 5);
    ImGui::End();
  };


  //
  // main part
  //

  // initial opengl
  helper::InitialOpenGL(initial_window, window);
  // enable depth
  glEnable(GL_DEPTH_TEST);

  glfwSetMouseButtonCallback(window, MouseButtonCallback);

  // create shader program
  GLuint simple_shader_program = 
      helper::CreatProgramWithShader("E:\\GitHub\\Computer-Graphics\\8 - Bezier Curve\\resources\\shaders\\simple.vs",
                                     "E:\\GitHub\\Computer-Graphics\\8 - Bezier Curve\\resources\\shaders\\simple.fs");
  helper::SetShaderVec3(simple_shader_program, "inColor", color[0], color[1], color[2]);
  // vertices
  vector<GLfloat> vertices{};

  auto bezier_points = [](int i,int pickedCount) {//pickedCount=5
    float t = float(i) / total_points;
    vector<float> b(pickedCount,0);//有i个元素，且初始化为0

    for (int j = 0; j < b.size(); ++j) {

      b[j] = C(pickedCount-1,j)*pow(1 - t, pickedCount-1-j)* pow(t,j);
    }
    float x = 0;
    float y = 0;

    for (int j = 0; j < pickedCount; ++j) {
      x += mouse_points.get()[kEachPointsSize * j] * b[j];
      y += mouse_points.get()[kEachPointsSize * j + 1] * b[j];
    }

    float z = 0.0;
    return std::vector<GLfloat> {x, y, z};
  };



  auto initial_vertices = [&vertices, bezier_points,&lineResult,CalculatePath]() {
    // clear
    vertices.clear();
    lineResult.clear();
    vector<glm::vec2> tpPoints;
    for (int i = 0; i < points_count; ++i) {
      float x = *(mouse_points.get() + kEachPointsSize * i + 0);
      float y = *(mouse_points.get() + kEachPointsSize * i + 1);
      float z = *(mouse_points.get() + kEachPointsSize * i + 2);
      glm::vec2 temp = glm::vec2(x, y);
      tpPoints.push_back(temp);
    }
    CalculatePath(tpPoints, animator);//计算路径点

    vertices.insert(vertices.end(), lineResult.begin(), lineResult.end());//添加了各个线段的

    for (int i = 0; i < total_points; i++) {
      auto each_point = bezier_points(i,points_count);
      vertices.insert(vertices.end(), each_point.begin(), each_point.end());
    }
    for (int i = 0; i < 10; i++) {
      cout << vertices[i*3]<<" "<<vertices[i*3+1]<<" "<<vertices[i*3+2] << endl;
    }
    cout << endl << endl << endl;
  };


  // vaos
  auto set_plane =[&vertices, initial_vertices](GLuint VAO, GLuint VBO, GLuint EBO) {
    initial_vertices();
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(),
                 vertices.data(), GL_STATIC_DRAW);
    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
  };


  // main part
  while (!glfwWindowShouldClose(window)) {

    animator += 0.01;
    if (animator >= 0.95)animator = 0.01;

    glfwPollEvents();
    ProcessInput(window);
    glfwGetWindowSize(window, &width, &height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ImGui_ImplGlfwGL3_NewFrame();
    create_imgui();

    // can draw line
    if (points_count /*== kTotalPointsNum*/>=2) {
      glUseProgram(simple_shader_program);
      helper::SetVAO(points_vao[0], points_vao[1], points_vao[2], set_plane);
      helper::SetShaderVec3(simple_shader_program, "inColor", color[0], color[1], color[2]);
    //  glDrawArrays(GL_POINTS, 0, 2);
    //  glDrawArrays(GL_LINES, 0, 2);
      for (int i = 0; i < points_count*(points_count-1) / 2; ++i) {
        glDrawArrays(GL_LINES, 2 * i, 2);
      }
      glDrawArrays(GL_POINTS, lineResult.size(), total_points );
      
     /* glDrawArrays(GL_LINES, 0, 2);
      glDrawArrays(GL_LINES, 1, 2);
      glDrawArrays(GL_LINES, 2, 2);
      glDrawArrays(GL_LINES, 3, 2);
      glDrawArrays(GL_POINTS, kTotalPointsNum, total_points + kTotalPointsNum);*/
    }

    ImGui::Render();
    ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  helper::exit_program();
}

// process all input: query GLFW whether relevant keys are pressed/released this
// frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void ProcessInput(GLFWwindow* window) {
}

// glfw: whenever the window size changed (by OS or user resize) this callback
// function executes
// ---------------------------------------------------------------------------------------------
void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
  // pass
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
  mouse_x = xpos;
  mouse_y = ypos;
}


void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)//此时需要增加一个点
  {
    if (points_count >= 0 && points_count < /*kTotalPointsNum*/100)
    {
      mouse_points.get()[points_count * kEachPointsSize + 0] = float(mouse_x - (width / 2)) / (width / 2);
      mouse_points.get()[points_count * kEachPointsSize + 1] = float((height / 2) - mouse_y) / (height / 2);
      mouse_points.get()[points_count * kEachPointsSize + 2] = 0.0;
      points_count++;
    }
  }
  if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {//此时需要减少一个点
    points_count--;
  }
}