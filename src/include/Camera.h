#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// 定义相机运动的几种可能选项。用于抽象化，以避免与窗口系统特定的输入方法相关联
enum Camera_Movement {
    FORWARD,     // 前进
    BACKWARD,    // 后退
    LEFT,        // 向左
    RIGHT,       // 向右
    STOP,        // 停止
    UP,          // 向上
    DOWN,        // 向下
    TURN_LEFT,   // 向左转
    TURN_RIGHT,  // 向右转
    TURN_UP,     // 向上转
    TURN_DOWN    // 向下转
};

// 默认相机参数
const float YAW = 0.0f; // 偏航角
const float PITCH = 0.0f;  // 俯仰角
const float SPEED = 0.5f;  // 移动速度
// const float SENSITIVITY = 0.1f;  // 鼠标灵敏度
const float SENSITIVITY = 1.0f;  // 鼠标灵敏度
const float ZOOM = 45.0f; // 视野范围（缩放）

// 一个抽象的相机类，处理输入并计算相应的欧拉角、向量和矩阵，以便在 OpenGL 中使用
class Camera {
public:
    // 相机属性
    glm::vec3 Position; // 相机位置
    glm::vec3 Front;    // 相机前方方向
    glm::vec3 Up;       // 相机上方方向
    glm::vec3 Right;    // 相机右方方向
    glm::vec3 WorldUp;  // 世界的上方方向

    glm::vec3 old_Position; // 原相机位置
    glm::vec3 old_Front;    // 原相机前方方向
    glm::vec3 old_Up;       // 原相机上方方向
    glm::vec3 old_Right;    // 原相机右方方向
    glm::vec3 old_WorldUp;  // 原世界的上方方向

    // 欧拉角
    float Yaw;          // 偏航角
    float Pitch;        // 俯仰角

    float old_Yaw;          // 偏航角
    float old_Pitch;        // 俯仰角

    // 相机选项
    float MovementSpeed; // 移动速度
    float MouseSensitivity; // 鼠标灵敏度
    float Zoom;          // 缩放值

    glm::mat4 viewMatrix; // 视图矩阵
    glm::mat4 projMatrix; // 投影矩阵

    // 投影参数
    float zNear = 0.01;   // 近裁剪面
    float zFar = 100.0;   // 远裁剪面

    // 透视投影参数
    float aspect = 1.0;   // 纵横比

    // 正交投影参数
    float scale = 1.5;    // 缩放因子

    // 使用向量的构造函数
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.5f, 1.5f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH)
        : Front(glm::vec3(0.0f, -0.3f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCamera(); // 更新相机方向
    }

    // 使用标量值的构造函数
    Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
    {
        Position = glm::vec3(posX, posY, posZ);
        WorldUp = glm::vec3(upX, upY, upZ);
        Yaw = yaw;
        Pitch = pitch;
        updateCamera(); // 更新相机方向
    }

    // 初始化
    void init() {
        Position = glm::vec3(0.0f, 0.5f, 1.5f);
        WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        Front = glm::vec3(0.0f, -0.3f, -1.0f);
    }

    // 返回使用欧拉角和 LookAt 矩阵计算的视图矩阵
    glm::mat4 getViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up); // 生成视图矩阵
    }

    // 根据是否为正交投影返回投影矩阵
    glm::mat4 getProjectionMatrix(bool isOrtho) {
        if (isOrtho) {
            return this->ortho(-scale, scale, -scale, scale, this->zNear, this->zFar); // 返回正交投影矩阵
        }
        else {
            return this->perspective(this->Zoom, this->aspect, this->zNear, this->zFar); // 返回透视投影矩阵
        }
    }

    // 生成正交投影矩阵
    glm::mat4 ortho(const GLfloat left, const GLfloat right, const GLfloat bottom, const GLfloat top, const GLfloat zNear, const GLfloat zFar) {
        glm::mat4 c = glm::mat4(1.0f);
        c[0][0] = 2.0 / (right - left);
        c[1][1] = 2.0 / (top - bottom);
        c[2][2] = -2.0 / (zFar - zNear);
        c[3][3] = 1.0;
        c[0][3] = -(right + left) / (right - left);
        c[1][3] = -(top + bottom) / (top - bottom);
        c[2][3] = -(zFar + zNear) / (zFar - zNear);

        c = glm::transpose(c); // 转置矩阵
        return c;
    }

    // 生成透视投影矩阵
    glm::mat4 perspective(const GLfloat fovy, const GLfloat aspect, const GLfloat zNear, const GLfloat zFar) {
        GLfloat top = tan(fovy * M_PI / 180 / 2) * zNear;
        GLfloat right = top * aspect;

        glm::mat4 c = glm::mat4(1.0f);
        c[0][0] = zNear / right;
        c[1][1] = zNear / top;
        c[2][2] = -(zFar + zNear) / (zFar - zNear);
        c[2][3] = -(2.0 * zFar * zNear) / (zFar - zNear);
        c[3][2] = -1.0;
        c[3][3] = 0.0;

        c = glm::transpose(c); // 转置矩阵
        return c;
    }

    // 处理来自任何键盘输入系统的输入。接受的输入参数为相机定义的枚举（以抽象化窗口系统）
    void keyboard(Camera_Movement direction) {
        if (direction == FORWARD) {
            Position += Front * MovementSpeed * 0.3f; // 前进
        }
        if (direction == BACKWARD) {
            Position -= Front * MovementSpeed * 0.3f; // 后退
        }
        if (direction == LEFT) {
            Position -= Right * MovementSpeed * 0.3f; // 向左移动
        }
        if (direction == RIGHT) {
            Position += Right * MovementSpeed * 0.3f; // 向右移动
        }
        if (direction == UP) {
            Position += Up * MovementSpeed * 0.3f; //向上移动
        }
        if (direction == DOWN) {
            Position -= Up * MovementSpeed * 0.3f; //向上移动
        }
        if (direction == STOP) {
            Position += 0; // 停止移动（无效操作）
        }
        if (direction == TURN_LEFT) {
            Yaw += 20.0 * MovementSpeed; // 向左转
        }
        if (direction == TURN_RIGHT) {
            Yaw -= 20.0 * MovementSpeed; // 向右转
        }
        if (direction == TURN_UP) {
            Pitch += 10.0 * MovementSpeed; // 向上转
        }
        if (direction == TURN_DOWN) {
            Pitch -= 10.0 * MovementSpeed; // 向下转
        }
        
        float skyboxSize = 5.0f;
        float skyboxHalf = skyboxSize / 2.0f;
        float margin = 0.1f;
        Position.x = glm::clamp(Position.x, -skyboxHalf + margin, skyboxHalf - margin);
        Position.y = glm::clamp(Position.y, 0.0f + margin, skyboxSize - margin);
        Position.z = glm::clamp(Position.z, -skyboxHalf + margin, skyboxHalf - margin);
    }

    // 处理来自鼠标输入系统的输入。只需要 x 和 y 方向的偏移值
    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset; // 更新偏航角
        Pitch += yoffset; // 更新俯仰角

        // 确保当俯仰角超出范围时，屏幕不会翻转
        if (constrainPitch) {
            if (Pitch > 89.0f)
                Pitch = 89.0f;
            if (Pitch < -89.0f)
                Pitch = -89.0f;
        }

        // 使用更新的欧拉角更新前方、右方和上方向量
        updateCamera();
    }

    // 处理鼠标滚轮事件。只需要垂直滚轮轴的输入
    void ProcessMouseScroll(float yoffset) {
        Zoom -= (float)yoffset; // 更新缩放值
        if (Zoom < 1.0f)
            Zoom = 1.0f; // 限制最小缩放值
        if (Zoom > 45.0f)
            Zoom = 45.0f; // 限制最大缩放值
    }

    // 根据相机的（更新后的）欧拉角计算前方向量
    void updateCamera() {
        // 计算新的前方向量
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front); // 归一化前方向量

        // 重新计算右方和上方向量
        Right = glm::normalize(glm::cross(Front, WorldUp)); // 归一化右方向量
        Up = glm::normalize(glm::cross(Right, Front)); // 归一化上方向量
    }

    glm::vec3 getposition() {
        return Position;
    }

    void setposition(glm::vec3 p) {
        Position = p;
    }

    void return_old() {
        Position = old_Position; // 相机位置
        Front = old_Front;    // 相机前方方向
        Up = old_Up;       // 相机上方方向
        Right = old_Right;    // 相机右方方向
        WorldUp = old_WorldUp;  // 世界的上方方向
        Yaw = old_Yaw;
        Pitch = old_Pitch;
        updateCamera();
    }

    void aim_camera(glm::vec3 aim) {
        old_Position = Position;
        old_Front = Front;    // 相机前方方向
        old_Up = Up;       // 相机上方方向
        old_Right = Right;    // 相机右方方向
        old_WorldUp = WorldUp;  // 世界的上方方向
        old_Yaw = Yaw;
        old_Pitch = Pitch;

        glm::vec3 temp = aim - Position; //init_front
        Position.y = 1.0f;
        glm::vec3 des;
        des.x = 0.5 * temp.x / sqrt(temp.x * temp.x + temp.z * temp.z);
        des.z = 0.5 * temp.z / sqrt(temp.x * temp.x + temp.z * temp.z);
        Position.x = aim.x - des.x;
        Position.z = aim.z - des.z;
        Front = aim - Position;
        Pitch = asin(Front.y);
        Yaw = atan2(Front.z, Front.x);
        Pitch *= (180.0f / M_PI);
        Yaw *= (180.0f / M_PI);
        Right = glm::normalize(glm::cross(Front, WorldUp)); // 归一化右方向量
        Up = glm::normalize(glm::cross(Right, Front)); // 归一化上方向量
    }

};
#endif