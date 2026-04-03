#include "Angel.h"
#include "TriMesh.h"
#include "Camera.h"
#include "MeshPainter.h"
#include "BALLS.h"

#include <vector>
#include <string>
#include <cmath>
#include <map>
#include <algorithm>
#include <cfloat>
#include <GLFW/glfw3.h>

#define White	glm::vec3(1.0, 1.0, 1.0)
#define Yellow	glm::vec3(1.0, 1.0, 0.0)
#define Green	glm::vec3(0.0, 1.0, 0.0)
#define Cyan	glm::vec3(0.0, 1.0, 1.0)
#define Magenta	glm::vec3(1.0, 0.0, 1.0)
#define Red		glm::vec3(1.0, 0.0, 0.0)
#define Black	glm::vec3(0.0, 0.0, 0.0)
#define Blue	glm::vec3(0.0, 0.0, 1.0)
#define Brown	glm::vec3(0.5, 0.5, 0.5)

float lastX = 800 / 2.0f;
float lastY = 600 / 2.0f;
bool firstMouse = true;
float smoothX = 0.0f;
float smoothY = 0.0f;

int WIDTH = 600;
int HEIGHT = 600;

int mainWindow;

int control_mode = 1;
//1表示控制相机
//3表示击球模式
//4表示层级建模控制模式
//5光源移动模式


int testmove = false;

int marked_ball_index = -1;
bool ball_marked = false;
bool ball_removing = false;
double remove_start_time = 0.0;
bool showing_effect = false;
double effect_start_time = 0.0;

struct HoleEffect {
	glm::vec3 position;
	double start_time;
	bool active;
};

struct BallCollision {
	double start_time;
	glm::vec3 collision_direction; // 碰撞方向（从球心指向碰撞点）
	bool active;
};

std::vector<HoleEffect> hole_effects;
std::vector<bool> previous_ball_exists;
std::vector<BallCollision> ball_collisions;
std::vector<glm::vec3> previous_ball_locations;

// 相机初始位置：高于桌面（y=3.0），俯视桌面（pitch=-60度向下看）
Camera* camera = new Camera(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, -60.0f);
bool stop_camera = false; //是否停止鼠标移动视角
Light* light = new Light();
glm::vec3 lightLocation = glm::vec3(0.5, 2.5, 1.0);
// glm::vec3 lightLocation = glm::vec3(-0.58, 1.0, 0.0);
MeshPainter* painter = new MeshPainter();
MeshPainter* painter2 = new MeshPainter();

TriMesh* Ground = new TriMesh();
TriMesh* Table = new TriMesh();
//桌面长度-1.116 ~1.116
//上桌面高度0.743
//桌面宽度-0.574~0.566
// glm::vec3 testlocation = glm::vec3 (-1.116f, 0.743f, 0.0f); //此为桌面长度，高度，宽度
glm::vec3 testlocation = glm::vec3 (100.0f, 100.0f, 100.0f); //此为桌面长度，高度，宽度
TriMesh* Test = new TriMesh(); //平面测试

TriMesh* SkyboxBack = new TriMesh();
TriMesh* SkyboxFront = new TriMesh();
TriMesh* SkyboxLeft = new TriMesh();
TriMesh* SkyboxRight = new TriMesh();
TriMesh* SkyboxTop = new TriMesh();

TriMesh* WhiteBall = new TriMesh();
TriMesh* BlackBall = new TriMesh();
TriMesh* PinkBall = new TriMesh();
TriMesh* BlueBall = new TriMesh();
TriMesh* GreenBall = new TriMesh();
TriMesh* BrownBall = new TriMesh();
TriMesh* YellowBall = new TriMesh();
TriMesh* RedBall1 = new TriMesh();
TriMesh* RedBall2 = new TriMesh();
TriMesh* RedBall3 = new TriMesh();
TriMesh* RedBall4 = new TriMesh();
TriMesh* RedBall5 = new TriMesh();
TriMesh* RedBall6 = new TriMesh();
TriMesh* RedBall7 = new TriMesh();
TriMesh* RedBall8 = new TriMesh();
TriMesh* RedBall9 = new TriMesh();
TriMesh* RedBall10 = new TriMesh();

TriMesh* aim_square = new TriMesh();

TriMesh* RobotBase = new TriMesh();
TriMesh* RobotUpperArm = new TriMesh();
TriMesh* RobotLowerArm = new TriMesh();
TriMesh* RobotEnd = new TriMesh();
TriMesh* Bubble = new TriMesh();

enum {
	RobotBaseJoint = 0,
	RobotUpperArmJoint = 1,
	RobotLowerArmJoint = 2,
	RobotHandJoint = 3,
	RobotNumJoints = 4
};

int RobotAxis = RobotBaseJoint;
GLfloat RobotTheta[RobotNumJoints] = { 0.0, 0.0, 0.0, 0.0 };

const GLfloat ROBOT_BASE_HEIGHT = 0.05f;
const GLfloat ROBOT_BASE_WIDTH = 0.08f;
const GLfloat ROBOT_UPPER_ARM_HEIGHT = 0.1f;
const GLfloat ROBOT_UPPER_ARM_WIDTH = 0.03f;
const GLfloat ROBOT_LOWER_ARM_HEIGHT = 0.08f;
const GLfloat ROBOT_LOWER_ARM_WIDTH = 0.025f;
const GLfloat ROBOT_END_SIZE = 0.02f;

float robot_path_progress = 0.0f;
glm::vec3 robot_base_position = glm::vec3(0.0f, 0.743f, 0.0f);

// 回收和删除创建的物体对象
std::vector<TriMesh*> meshList;

// 计算水平投影并与 Z 轴的夹角
float calculateHorizontalProjectionAndAngle(glm::vec3 direction) {
	// 计算水平投影 (忽略 Y 分量)
	glm::vec3 horizontalProjection = glm::vec3(direction.x, 0.0f, direction.z);

	// 归一化水平投影
	horizontalProjection = glm::normalize(horizontalProjection);

	// 计算与 Z 轴的夹角
	glm::vec3 zAxis(0.0f, 0.0f, 1.0f); // Z 轴向量
	float dotProduct = glm::dot(horizontalProjection, zAxis);
	float lengthHorizontal = glm::length(horizontalProjection);

	// 计算夹角的余弦值
	float cosTheta = dotProduct / (lengthHorizontal * 1.0f);
	cosTheta = glm::clamp(cosTheta, -1.0f, 1.0f); // 确保在 [-1, 1] 范围内

	// 计算夹角（弧度）
	float angleInRadians = std::acos(cosTheta);

	// 转换为角度
	float angle = glm::degrees(angleInRadians);
	if (horizontalProjection.x < 0) {
		angle = 360 - angle;
	}
	return angle;
}

//地面绘制
void drawground(glm::mat4 modelMatrix, int mesh_idx) {
	// mesh_idx指painter中模型的编号，与addMesh的顺序对应
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(90.0f), glm::vec3(-1.0, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(5, 5, 0));

	painter->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0);
}

void drawskybox(glm::mat4 modelMatrix) {
	float skyboxSize = 5.0f;
	float skyboxHalf = skyboxSize / 2.0f;
	
	glm::mat4 backMatrix = glm::mat4(1.0);
	backMatrix = glm::translate(backMatrix, glm::vec3(0.0, skyboxHalf, -skyboxHalf));
	backMatrix = glm::rotate(backMatrix, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
	backMatrix = glm::scale(backMatrix, glm::vec3(skyboxSize, skyboxSize, 1.0));
	painter->drawMesh(4, backMatrix, light, camera, 0);

	glm::mat4 frontMatrix = glm::mat4(1.0);
	frontMatrix = glm::translate(frontMatrix, glm::vec3(0.0, skyboxHalf, skyboxHalf));
	frontMatrix = glm::scale(frontMatrix, glm::vec3(skyboxSize, skyboxSize, 1.0));
	painter->drawMesh(5, frontMatrix, light, camera, 0);

	glm::mat4 leftMatrix = glm::mat4(1.0);
	leftMatrix = glm::translate(leftMatrix, glm::vec3(-skyboxHalf, skyboxHalf, 0.0));
	leftMatrix = glm::rotate(leftMatrix, glm::radians(90.0f), glm::vec3(0.0, 1.0, 0.0));
	leftMatrix = glm::scale(leftMatrix, glm::vec3(skyboxSize, skyboxSize, 1.0));
	painter->drawMesh(6, leftMatrix, light, camera, 0);

	glm::mat4 rightMatrix = glm::mat4(1.0);
	rightMatrix = glm::translate(rightMatrix, glm::vec3(skyboxHalf, skyboxHalf, 0.0));
	rightMatrix = glm::rotate(rightMatrix, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
	rightMatrix = glm::scale(rightMatrix, glm::vec3(skyboxSize, skyboxSize, 1.0));
	painter->drawMesh(7, rightMatrix, light, camera, 0);

	glm::mat4 topMatrix = glm::mat4(1.0);
	topMatrix = glm::translate(topMatrix, glm::vec3(0.0, skyboxSize, 0.0));
	topMatrix = glm::rotate(topMatrix, glm::radians(-90.0f), glm::vec3(1.0, 0.0, 0.0));
	topMatrix = glm::scale(topMatrix, glm::vec3(skyboxSize, skyboxSize, 1.0));
	painter->drawMesh(8, topMatrix, light, camera, 0);
}


//台球桌绘制
void drawtable(glm::mat4 modelMatrix, int mesh_idx, bool is_shadow) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(0.0f), glm::vec3(-1.0, 0.0, 0.0));
	instance = glm::scale(instance, glm::vec3(1, 1, 1));
	// painter->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0); //无光照版本
	// /*
	if (is_shadow == false) {
		painter->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0);
	}
	else {
		glDepthMask(GL_FALSE); // 禁用深度写入，避免绘制的立体物体的阴影错误叠加
		glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
		painter->drawMesh(mesh_idx, shadowMatrix * (modelMatrix * instance), light, camera, 1);
		glDepthMask(GL_TRUE); // 恢复深度写入
	}
	// */
}

void drawtest(glm::mat4 modelMatrix, int mesh_idx) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(180.0f), glm::vec3(-1.0, 0.0, 1.0));
	instance = glm::scale(instance, glm::vec3(1, 1, 0));
	painter->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0);
}

void drawball(glm::mat4 modelMatrix, int mesh_idx, bool is_shadow) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(0.0f), glm::vec3(-1.0, 0.0, 0.0));
	
	// Apply jelly effect if ball is in collision
	bool has_collision = (mesh_idx >= 0 && mesh_idx < ball_collisions.size() && ball_collisions[mesh_idx].active);
	if (has_collision) {
		double currentTime = glfwGetTime();
		double elapsed = currentTime - ball_collisions[mesh_idx].start_time;
		
		if (elapsed < 0.6) { // Jelly effect duration: 0.6 seconds for 3 bounces
			// Calculate deformation: compress along collision direction, stretch perpendicular
			glm::vec3 collision_dir = ball_collisions[mesh_idx].collision_direction;
			collision_dir.y = 0.0f; // Keep collision in horizontal plane
			float dir_len = glm::length(collision_dir);
			if (dir_len < 0.001f) {
				collision_dir = glm::vec3(1.0f, 0.0f, 0.0f); // Default direction
			}
			else {
				collision_dir = glm::normalize(collision_dir);
			}
			
			// Three bounces with decreasing amplitude
			// First bounce: 0-0.2s, amplitude 0.4
			// Second bounce: 0.2-0.4s, amplitude 0.25
			// Third bounce: 0.4-0.6s, amplitude 0.15
			float compression = 0.0f;
			if (elapsed < 0.2f) {
				// First bounce: large deformation
				compression = sin(elapsed * M_PI / 0.2f) * 0.4f;
			}
			else if (elapsed < 0.4f) {
				// Second bounce: medium deformation
				compression = sin((elapsed - 0.2f) * M_PI / 0.2f) * 0.25f;
			}
			else {
				// Third bounce: small deformation
				compression = sin((elapsed - 0.4f) * M_PI / 0.2f) * 0.15f;
			}
			
			// Compress along collision direction, stretch perpendicular
			float compress_factor = 1.0f - compression;
			float stretch_factor = 1.0f + compression * 0.6f;
			float vertical_stretch = 1.0f + compression * 0.4f;
			
			// Rotate to align collision direction with X axis
			float angle = atan2(collision_dir.z, collision_dir.x);
			instance = glm::rotate(instance, angle, glm::vec3(0.0f, 1.0f, 0.0f));
			// Apply scale: compress X, stretch Z and Y
			instance = glm::scale(instance, glm::vec3(compress_factor, vertical_stretch, stretch_factor));
			// Rotate back
			instance = glm::rotate(instance, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		else {
			// Reset collision effect
			ball_collisions[mesh_idx].active = false;
			has_collision = false;
		}
	}
	
	// Apply base ball scale
	instance = glm::scale(instance, ballscale);
	// painter->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0);
	// /*
	if (is_shadow == false) {
		painter2->drawMesh(mesh_idx, modelMatrix * instance, light, camera, 0);
	}
	else if (is_shadow == true) {
		glDepthMask(GL_FALSE); // 禁用深度写入，避免绘制的立体物体的阴影错误叠加
		// 使用多边形偏移，让阴影稍微浮在桌面上方，避免被桌面遮挡
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -1.0f); // 负值让阴影更靠近相机（浮在桌面上方）
		glm::mat4 shadowMatrix = light->getShadowProjectionMatrix2();
		// 创建一个偏移矩阵，用于将阴影投影到y=0.743的平面上
		painter2->drawMesh2(mesh_idx, shadowMatrix * (modelMatrix * instance), light, camera, 1);
		glDisable(GL_POLYGON_OFFSET_FILL); // 恢复多边形偏移设置
		glDepthMask(GL_TRUE); // 恢复深度写入
	}
	// */
}

void draw_robot_base(glm::mat4 modelView) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, ROBOT_BASE_HEIGHT / 2, 0.0));
	instance = glm::scale(instance, glm::vec3(ROBOT_BASE_WIDTH, ROBOT_BASE_HEIGHT, ROBOT_BASE_WIDTH));
	painter->drawMesh(9, modelView * instance, light, camera, 0);
	
	glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
	painter->drawMesh(9, shadowMatrix * (modelView * instance), light, camera, 1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
}

void draw_robot_upper_arm(glm::mat4 modelView) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(90.0f), glm::vec3(1.0, 0.0, 0.0));
	instance = glm::rotate(instance, glm::radians(180.0f), glm::vec3(0.0, 1.0, 0.0));
	painter->drawMesh(10, modelView * instance, light, camera, 0);
	
	glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
	painter->drawMesh(10, shadowMatrix * (modelView * instance), light, camera, 1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
}

void draw_robot_lower_arm(glm::mat4 modelView) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, 0.01f, 0.0)); // 向Y轴正方向平移一点
	instance = glm::scale(instance, glm::vec3(ROBOT_LOWER_ARM_WIDTH, ROBOT_LOWER_ARM_WIDTH, ROBOT_LOWER_ARM_WIDTH));
	painter->drawMesh(11, modelView * instance, light, camera, 0);
	
	glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
	painter->drawMesh(11, shadowMatrix * (modelView * instance), light, camera, 1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
}

void draw_robot_end(glm::mat4 modelView) {
	glm::mat4 instance = glm::mat4(1.0);
	instance = glm::translate(instance, glm::vec3(0.0, -0.02f, 0.0)); // 向Y轴负方向平移
	// 将圆锥从Z轴正方向旋转到Y轴正方向（绕X轴旋转-90度）
	instance = glm::rotate(instance, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	painter->drawMesh(12, modelView * instance, light, camera, 0);
	
	glDepthMask(GL_FALSE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0f, -1.0f);
	glm::mat4 shadowMatrix = light->getShadowProjectionMatrix();
	painter->drawMesh(12, shadowMatrix * (modelView * instance), light, camera, 1);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDepthMask(GL_TRUE);
}

void draw_robot(glm::mat4 modelMatrix) {
	float x = 0.0f, z = 0.0f;
	float expand = 2.0f * balls_r;
	float front_bound = balls_front + expand;
	float back_bound = balls_back - expand;
	float right_bound = balls_right + expand;
	float left_bound = balls_left - expand;
	float length = right_bound - left_bound;
	float width = front_bound - back_bound;
	float perimeter = 2.0f * (length + width);
	float current_dist = robot_path_progress * perimeter;
	
	if (current_dist < width) {
		x = front_bound - current_dist;
		z = right_bound;
	}
	else if (current_dist < width + length) {
		x = back_bound;
		z = right_bound - (current_dist - width);
	}
	else if (current_dist < 2.0f * width + length) {
		x = back_bound + (current_dist - width - length);
		z = left_bound;
	}
	else {
		x = front_bound;
		z = left_bound + (current_dist - 2.0f * width - length);
	}
	
	robot_base_position = glm::vec3(x, 0.743f + ROBOT_BASE_HEIGHT / 2 + balls_r, z);
	
	glm::mat4 robotView = glm::mat4(1.0);
	robotView = glm::translate(robotView, robot_base_position);
	robotView = glm::rotate(robotView, glm::radians(RobotTheta[RobotBaseJoint]), glm::vec3(0.0, 1.0, 0.0));
	draw_robot_base(robotView);
	
	glm::mat4 upperView = robotView;
	upperView = glm::translate(upperView, glm::vec3(0.0, ROBOT_BASE_HEIGHT, 0.0));
	upperView = glm::rotate(upperView, glm::radians(RobotTheta[RobotUpperArmJoint]), glm::vec3(0.0, 0.0, 1.0));
	draw_robot_upper_arm(upperView);
	
	glm::mat4 lowerView = upperView;
	lowerView = glm::translate(lowerView, glm::vec3(0.0, ROBOT_UPPER_ARM_HEIGHT / 2.0f, 0.0));
	lowerView = glm::rotate(lowerView, glm::radians(RobotTheta[RobotLowerArmJoint]), glm::vec3(0.0, 1.0, 0.0)); // 改为绕Y轴旋转
	draw_robot_lower_arm(lowerView);
	
	glm::mat4 endView = lowerView;
	endView = glm::translate(endView, glm::vec3(0.0, ROBOT_LOWER_ARM_HEIGHT / 2.0f, 0.0));
	endView = glm::rotate(endView, glm::radians(RobotTheta[RobotHandJoint]), glm::vec3(0.0, 0.0, 1.0));
	draw_robot_end(endView);
}

void test_kick(int idx, float kick_v) { //通过直接给球一个速度来进行测试
	glm::vec3 kick_point1 = camera->getposition();
	glm::vec3 kick_point2 = Ballslocation[idx];
	glm::vec3 temp = kick_point2 - kick_point1;
	Ballsvelocity[idx].x = kick_v * temp.x / sqrt(temp.x * temp.x + temp.z * temp.z);
	Ballsvelocity[idx].z = kick_v * temp.z / sqrt(temp.x * temp.x + temp.z * temp.z);
	Ballsaccelerate[idx].x = -0.05 * Ballsvelocity[idx].x;
	Ballsaccelerate[idx].z = -0.05 * Ballsvelocity[idx].z;
	// /* 输出调试白球和相机当前位置
	std::cout << kick_point1.x << " " << kick_point1.y << " " << kick_point1.z << std::endl;
	std::cout << kick_point2.x << " " << kick_point2.y << " " << kick_point2.z << std::endl;
	std::cout << temp.x << " " << temp.y << " " << temp.z << std::endl;
	std::cout << Ballsvelocity[idx].x << " " << Ballsvelocity[idx].y << " " << Ballsvelocity[idx].z << std::endl;
	std::cout << Ballsaccelerate[idx].x << " " << Ballsaccelerate[idx].y << " " << Ballsaccelerate[idx].z << std::endl;
	// */
}

glm::vec3 getRobotPosition() {
	float x = 0.0f, z = 0.0f;
	float expand = 2.0f * balls_r;
	float front_bound = balls_front + expand;
	float back_bound = balls_back - expand;
	float right_bound = balls_right + expand;
	float left_bound = balls_left - expand;
	float length = right_bound - left_bound;
	float width = front_bound - back_bound;
	float perimeter = 2.0f * (length + width);
	float current_dist = robot_path_progress * perimeter;
	
	if (current_dist < width) {
		x = front_bound - current_dist;
		z = right_bound;
	}
	else if (current_dist < width + length) {
		x = back_bound;
		z = right_bound - (current_dist - width);
	}
	else if (current_dist < 2.0f * width + length) {
		x = back_bound + (current_dist - width - length);
		z = left_bound;
	}
	else {
		x = front_bound;
		z = left_bound + (current_dist - 2.0f * width - length);
	}
	
	return glm::vec3(x, 0.743f + ROBOT_BASE_HEIGHT / 2 + balls_r, z);
}

int findNearestBall() {
	glm::vec3 robotPos = getRobotPosition();
	int nearestIdx = -1;
	float minDist = FLT_MAX;
	
	for (int i = 0; i < Balls.size(); i++) {
		if (!Ballsexists[i]) continue;
		if (i == 0) continue;
		float dist = glm::length(Ballslocation[i] - robotPos);
		if (dist < minDist) {
			minDist = dist;
			nearestIdx = i;
		}
	}
	return nearestIdx;
}

void updateBallTexture(int ballIdx, const std::string& texturePath) {
	painter2->updateTexture(ballIdx, texturePath);
}

// 计算小臂起点在世界坐标系中的位置
glm::vec3 getLowerArmStartPosition() {
	glm::vec3 basePos = robot_base_position;
	
	// 基座旋转矩阵（绕Y轴）
	glm::mat4 baseRot = glm::rotate(glm::mat4(1.0), glm::radians(RobotTheta[RobotBaseJoint]), glm::vec3(0.0, 1.0, 0.0));
	
	// 大臂起点在基座局部坐标系中的位置
	glm::vec4 upperStartLocal = glm::vec4(0.0, ROBOT_BASE_HEIGHT, 0.0, 1.0);
	
	// 大臂旋转矩阵（绕Z轴）
	glm::mat4 upperRot = glm::rotate(glm::mat4(1.0), glm::radians(RobotTheta[RobotUpperArmJoint]), glm::vec3(0.0, 0.0, 1.0));
	
	// 小臂起点在大臂局部坐标系中的位置
	glm::vec4 lowerStartLocal = glm::vec4(0.0, ROBOT_UPPER_ARM_HEIGHT / 2.0f, 0.0, 1.0);
	
	// 转换到世界坐标系
	glm::vec4 lowerStartWorld = baseRot * upperRot * lowerStartLocal;
	
	return basePos + glm::vec3(lowerStartWorld);
}

// 计算小臂应该指向目标球的角度（相对于大臂的局部坐标系，绕Y轴旋转，指向Y轴正方向）
float calculateLowerArmAngleToTarget(glm::vec3 targetPos) {
	// 获取小臂起点位置
	glm::vec3 lowerStart = getLowerArmStartPosition();
	
	// 计算从小臂起点指向目标球的方向向量（世界坐标系）
	glm::vec3 direction = targetPos - lowerStart;
	
	// 基座旋转矩阵（绕Y轴）
	glm::mat4 baseRot = glm::rotate(glm::mat4(1.0), glm::radians(RobotTheta[RobotBaseJoint]), glm::vec3(0.0, 1.0, 0.0));
	
	// 大臂旋转矩阵（绕Z轴）
	glm::mat4 upperRot = glm::rotate(glm::mat4(1.0), glm::radians(RobotTheta[RobotUpperArmJoint]), glm::vec3(0.0, 0.0, 1.0));
	
	// 将方向向量转换到大臂的局部坐标系（先反向旋转大臂，再反向旋转基座）
	glm::mat4 invUpperRot = glm::rotate(glm::mat4(1.0), glm::radians(-RobotTheta[RobotUpperArmJoint]), glm::vec3(0.0, 0.0, 1.0));
	glm::mat4 invBaseRot = glm::rotate(glm::mat4(1.0), glm::radians(-RobotTheta[RobotBaseJoint]), glm::vec3(0.0, 1.0, 0.0));
	
	glm::vec4 directionLocal = invBaseRot * invUpperRot * glm::vec4(direction, 0.0);
	
	// 投影到XZ平面（因为小臂是绕Y轴旋转的）
	glm::vec2 directionXZ = glm::vec2(directionLocal.x, directionLocal.z);
	
	// 归一化
	float length = glm::length(directionXZ);
	if (length < 0.001f) {
		return 0.0f; // 如果方向向量太小，返回0度
	}
	directionXZ = directionXZ / length;
	
	// 计算与Y轴正方向的夹角
	// 小臂的Y轴正方向在XZ平面的投影是Z轴正方向(0, 1)
	// 目标方向在XZ平面的投影是(directionXZ.x, directionXZ.y)，其中directionXZ.y是z分量
	// 从Z轴正方向(0, 1)到目标方向(x, z)的夹角：atan2(x, z)
	// 注意：directionXZ.y实际上是z分量
	float angle = glm::degrees(std::atan2(directionXZ.x, directionXZ.y));
	
	return angle;
}

void bindLightAndMaterial(TriMesh* mesh, openGLObject& object, Light* light, Camera* camera) {

	// 传递相机的位置
	// glUniform3fv( glGetUniformLocation(object.program, "eye_position"), 1, &camera->eye[0] );
	glUniform3fv( glGetUniformLocation(object.program, "eye_position"), 1, &camera->Position[0]);

	// 传递物体的材质
	glm::vec4 meshAmbient = mesh->getAmbient();
	glm::vec4 meshDiffuse = mesh->getDiffuse();
	glm::vec4 meshSpecular = mesh->getSpecular();
	float meshShininess = mesh->getShininess();

	glUniform4fv(glGetUniformLocation(object.program, "material.ambient"), 1, &meshAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.diffuse"), 1, &meshDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "material.specular"), 1, &meshSpecular[0]);
	glUniform1f(glGetUniformLocation(object.program, "material.shininess"), meshShininess);

	// 传递光源信息
	glm::vec4 lightAmbient = light->getAmbient();
	glm::vec4 lightDiffuse = light->getDiffuse();
	glm::vec4 lightSpecular = light->getSpecular();
	glm::vec3 lightPosition = light->getTranslation();
	glUniform4fv(glGetUniformLocation(object.program, "light.ambient"), 1, &lightAmbient[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.diffuse"), 1, &lightDiffuse[0]);
	glUniform4fv(glGetUniformLocation(object.program, "light.specular"), 1, &lightSpecular[0]);
	glUniform3fv(glGetUniformLocation(object.program, "light.position"), 1, &lightPosition[0]);

}

void init() {
	// 启用混合
	glEnable(GL_BLEND);
	// 设置混合函数
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//设置光源位置
	light->setTranslation(lightLocation);
	light->setAmbient(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 环境光
	light->setDiffuse(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 漫反射
	light->setSpecular(glm::vec4(1.0, 1.0, 1.0, 1.0)); // 镜面反射

	std::string vshader, fshader, fshader2;
	// 读取着色器并使用
	vshader = "shaders/vshader.glsl";
	fshader = "shaders/fshader.glsl";
	fshader2 = "shaders/fshader2.glsl";

	//地面绘制
	Ground->setNormalize(false);
	Ground->generateSquare(Brown);
	painter->addMesh(Ground, "Ground", "./assets/ground.jpg", vshader, fshader);
	meshList.push_back(Ground);

	Table->setNormalize(false);
	Table->readObj("./assets/table.obj");
	painter->addMesh(Table, "Table", "./assets/table.png", vshader, fshader);
	meshList.push_back(Table);

	Test->setNormalize(false);
	Test->generateSquare(Black);
	painter->addMesh(Test, "Test", "./assets/black.png", vshader, fshader);
	meshList.push_back(Test);

	Balls.push_back(WhiteBall);
	Balls.push_back(BlackBall);
	Balls.push_back(PinkBall);
	Balls.push_back(BlueBall);
	Balls.push_back(GreenBall);
	Balls.push_back(BrownBall);
	Balls.push_back(YellowBall);
	Balls.push_back(RedBall1);
	Balls.push_back(RedBall2);
	Balls.push_back(RedBall3);
	Balls.push_back(RedBall4);
	Balls.push_back(RedBall5);
	Balls.push_back(RedBall6);
	Balls.push_back(RedBall7);
	Balls.push_back(RedBall8);
	Balls.push_back(RedBall9);
	Balls.push_back(RedBall10);

	for (int i = 0; i < Balls.size(); i++) {
		Balls[i]->setNormalize(false);
		Balls[i]->readOff("./assets/sphere.off");
		Balls[i]->generateSphereTexture();
		Balls[i]->setAmbient(BallsAmbient[i]);
		Balls[i]->setDiffuse(BallsDiffuse[i]);
		Balls[i]->setSpecular(BallsSpecular[i]);
		Balls[i]->setShininess(BallsShininess[i]);
		if (i == 0) {
			painter2->addMesh2(Balls[i], Ballsname[i], vshader, fshader2); // 白球使用Phong反射
		}
		else {
			painter2->addMesh2(Balls[i], Ballsname[i], vshader, fshader, "./assets/cat.png"); // 其他球不使用Phong反射
		}
		meshList.push_back(Balls[i]);
	}

	aim_square->setNormalize(false);
	aim_square->generateSquare(Black);
	painter->addMesh(aim_square, "aim_square", "./assets/black.png", vshader, fshader);
	meshList.push_back(aim_square);

	SkyboxBack->setNormalize(false);
	SkyboxBack->generateSquare(White);
	painter->addMesh(SkyboxBack, "SkyboxBack", "./assets/skybox/back.png", vshader, fshader);
	meshList.push_back(SkyboxBack);

	SkyboxFront->setNormalize(false);
	SkyboxFront->generateSquare(White);
	painter->addMesh(SkyboxFront, "SkyboxFront", "./assets/skybox/front.png", vshader, fshader);
	meshList.push_back(SkyboxFront);

	SkyboxLeft->setNormalize(false);
	SkyboxLeft->generateSquare(White);
	painter->addMesh(SkyboxLeft, "SkyboxLeft", "./assets/skybox/left.png", vshader, fshader);
	meshList.push_back(SkyboxLeft);

	SkyboxRight->setNormalize(false);
	SkyboxRight->generateSquare(White);
	painter->addMesh(SkyboxRight, "SkyboxRight", "./assets/skybox/right.png", vshader, fshader);
	meshList.push_back(SkyboxRight);

	SkyboxTop->setNormalize(false);
	SkyboxTop->generateSquare(White);
	painter->addMesh(SkyboxTop, "SkyboxTop", "./assets/skybox/top.png", vshader, fshader);
	meshList.push_back(SkyboxTop);

	RobotBase->setNormalize(false);
	RobotBase->generateCube(Red);
	painter->addMesh(RobotBase, "RobotBase", "./assets/arm.png", vshader, fshader);
	meshList.push_back(RobotBase);

	RobotUpperArm->setNormalize(false);
	RobotUpperArm->generateCylinder(32, ROBOT_UPPER_ARM_WIDTH / 2.0f, ROBOT_UPPER_ARM_HEIGHT / 2.0f);
	painter->addMesh(RobotUpperArm, "RobotUpperArm", "./assets/arm.png", vshader, fshader);
	meshList.push_back(RobotUpperArm);

	RobotLowerArm->setNormalize(false);
	RobotLowerArm->readOff("./assets/sphere.off");
	RobotLowerArm->generateSphereTexture();
	painter->addMesh(RobotLowerArm, "RobotLowerArm", "./assets/arm.png", vshader, fshader);
	meshList.push_back(RobotLowerArm);

	RobotEnd->setNormalize(false);
	RobotEnd->generateCone(32, 0.015f, 0.05f);
	RobotEnd->setAmbient(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	RobotEnd->setDiffuse(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	RobotEnd->setSpecular(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
	painter->addMesh(RobotEnd, "RobotEnd", "./assets/arm.png", vshader, fshader);
	meshList.push_back(RobotEnd);

	Bubble->setNormalize(false);
	Bubble->readOff("./assets/sphere.off");
	Bubble->generateSphereTexture();
	Bubble->setDiffuse(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
	Bubble->setAmbient(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
	Bubble->setSpecular(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
	painter->addMesh(Bubble, "Bubble", "./assets/pao.png", vshader, fshader);
	meshList.push_back(Bubble);

	glClearColor(0.7, 0.9, 1.0, 1.0);
	
	// Initialize previous_ball_exists and ball_collisions
	previous_ball_exists.resize(Balls.size());
	ball_collisions.resize(Balls.size());
	previous_ball_locations.resize(Balls.size());
	for (int i = 0; i < Balls.size(); i++) {
		previous_ball_exists[i] = Ballsexists[i];
		ball_collisions[i].active = false;
		previous_ball_locations[i] = Ballslocation[i];
	}
}

void display() {
	// 清除颜色缓冲和深度缓冲
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 modelMatrix = glm::mat4(1.0);

	// 天空盒绘制
	drawskybox(modelMatrix);

	// 地面绘制
	modelMatrix = glm::mat4(1.0);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.001, 0.0));
	drawground(modelMatrix, 0);

	modelMatrix = glm::mat4(1.0);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0, -0.001, 0.0));
	drawtable(modelMatrix, 1, false);
	drawtable(modelMatrix, 1, true);

	modelMatrix = glm::mat4(1.0);
	modelMatrix = glm::translate(modelMatrix, testlocation);
	drawtest(modelMatrix, 2);

	if (showing_effect && marked_ball_index >= 0) {
		double currentTime = glfwGetTime();
		double elapsed = currentTime - effect_start_time;
		
		if (elapsed < 1.5) {
			glm::vec3 effectPos = Ballslocation[marked_ball_index];
			for (int j = 0; j < 50; j++) {
				float angle = j * 7.2f + elapsed * 100.0f;
				float radius = 0.1f + elapsed * 0.2f + (j % 3) * 0.05f;
				float heightOffset = sin(glm::radians(angle * 2.5f)) * radius * 0.6f + elapsed * 0.27f;
				glm::vec3 particlePos = effectPos + glm::vec3(
					cos(glm::radians(angle)) * radius,
					heightOffset,
					sin(glm::radians(angle)) * radius
				);
				modelMatrix = glm::mat4(1.0);
				modelMatrix = glm::translate(modelMatrix, particlePos);
				float bubbleScale = 0.015f + sin(elapsed * 13.3f + j) * 0.005f;
				modelMatrix = glm::scale(modelMatrix, glm::vec3(bubbleScale, bubbleScale, bubbleScale));
				painter->drawMesh(13, modelMatrix, light, camera, 0);
			}
		}
		else {
			showing_effect = false;
			ball_removing = false;
			ball_marked = false;
			marked_ball_index = -1;
		}
	}
	else if (ball_removing && marked_ball_index >= 0 && Ballsexists[marked_ball_index]) {
		double currentTime = glfwGetTime();
		double elapsed = currentTime - remove_start_time;
		
		if (elapsed >= 1.5) {
			Ballsexists[marked_ball_index] = false;
			showing_effect = true;
			effect_start_time = currentTime;
		}
		else {
			float scale = 1.0f + sin(elapsed * 10.0f) * 0.2f;
			modelMatrix = glm::mat4(1.0);
			modelMatrix = glm::translate(modelMatrix, Ballslocation[marked_ball_index]);
			modelMatrix = glm::scale(modelMatrix, glm::vec3(scale, scale, scale) * ballscale);
			painter2->drawMesh2(marked_ball_index, modelMatrix, light, camera, 0);
			painter2->drawMesh2(marked_ball_index, modelMatrix, light, camera, 1);
		}
	}
	
	for (int i = 0; i < Balls.size(); i++) {
		if (!Ballsexists[i]) continue;
		if (ball_removing && i == marked_ball_index) continue;
		modelMatrix = glm::mat4(1.0);
		modelMatrix = glm::translate(modelMatrix, Ballslocation[i]);
		drawball(modelMatrix, i, false);
		drawball(modelMatrix, i, true);
	}

	Ballsmove();
	
	// Detect ball collisions for jelly effect
	double currentTime = glfwGetTime();
	for (int i = 0; i < Balls.size(); i++) {
		if (!Ballsexists[i]) continue;
		
		// Check collision with other balls
		for (int j = i + 1; j < Balls.size(); j++) {
			if (!Ballsexists[j]) continue;
			
			float current_dist = glm::length(Ballslocation[i] - Ballslocation[j]);
			float previous_dist = glm::length(previous_ball_locations[i] - previous_ball_locations[j]);
			
			// If balls are colliding now but weren't before (or were further apart)
			if (current_dist <= 2.0f * balls_r && previous_dist > 2.0f * balls_r) {
				// Calculate collision direction
				glm::vec3 collision_dir = glm::normalize(Ballslocation[i] - Ballslocation[j]);
				if (glm::length(collision_dir) < 0.001f) {
					collision_dir = glm::vec3(1.0f, 0.0f, 0.0f); // Default direction if balls overlap
				}
				
				// Trigger collision effect for both balls
				ball_collisions[i].start_time = currentTime;
				ball_collisions[i].collision_direction = collision_dir;
				ball_collisions[i].active = true;
				
				ball_collisions[j].start_time = currentTime;
				ball_collisions[j].collision_direction = -collision_dir;
				ball_collisions[j].active = true;
			}
		}
		
		// Update previous location
		previous_ball_locations[i] = Ballslocation[i];
	}
	
	// Check for balls that just fell into holes
	for (int i = 0; i < Balls.size(); i++) {
		if (previous_ball_exists[i] && !Ballsexists[i]) {
			// Skip if this ball was removed by robot arm (marked ball)
			if (i == marked_ball_index) {
				previous_ball_exists[i] = Ballsexists[i];
				continue;
			}
			// Ball just fell into hole, find nearest hole position
			glm::vec3 ballPos = Ballslocation[i];
			glm::vec3 nearestHole = Ballshole[0];
			float minDist = glm::length(ballPos - Ballshole[0]);
			for (int j = 1; j < 6; j++) {
				float dist = glm::length(ballPos - Ballshole[j]);
				if (dist < minDist) {
					minDist = dist;
					nearestHole = Ballshole[j];
				}
			}
			// Add effect at nearest hole position
			HoleEffect effect;
			effect.position = nearestHole;
			effect.start_time = currentTime;
			effect.active = true;
			hole_effects.push_back(effect);
		}
		previous_ball_exists[i] = Ballsexists[i];
	}
	
	// Render hole effects
	for (auto it = hole_effects.begin(); it != hole_effects.end();) {
		if (!it->active) {
			it = hole_effects.erase(it);
			continue;
		}
		
		double elapsed = currentTime - it->start_time;
		if (elapsed < 1.5) {
			// Render irregular, dense, colorful star particles floating upward
			glDepthMask(GL_FALSE);
			for (int j = 0; j < 100; j++) {
				// Irregular distribution using hash-like function
				float seed = j * 17.3f + elapsed * 7.0f;
				float angle1 = fmod(seed * 137.5f, 360.0f) + elapsed * (30.0f + sin(seed) * 20.0f);
				float angle2 = fmod(seed * 97.3f, 360.0f);
				
				// Irregular radius and height
				float radius = (0.03f + fmod(seed * 0.1f, 0.15f)) * (1.0f + elapsed * 0.8f);
				float height = elapsed * (0.25f + fmod(seed * 0.05f, 0.15f)) + sin(glm::radians(angle1 * 3.0f)) * 0.05f;
				
				// Irregular horizontal offset
				float offsetX = cos(glm::radians(angle1)) * radius * (0.5f + fmod(seed * 0.3f, 0.5f));
				float offsetZ = sin(glm::radians(angle2)) * radius * (0.5f + fmod(seed * 0.7f, 0.5f));
				
				// Varying sizes
				float baseScale = 0.005f + fmod(seed * 0.002f, 0.01f);
				float alpha = 1.0f - (elapsed / 1.5f);
				float scale = baseScale * (0.7f + fmod(seed * 0.5f, 0.6f)) * alpha;
				
				// Colorful particles - cycle through colors
				float colorPhase = fmod(seed * 0.7f + elapsed * 2.0f, 6.0f);
				glm::vec4 particleColor;
				if (colorPhase < 1.0f) {
					particleColor = glm::vec4(1.0f, colorPhase, 0.0f, alpha * 0.8f); // Red to Yellow
				}
				else if (colorPhase < 2.0f) {
					particleColor = glm::vec4(2.0f - colorPhase, 1.0f, 0.0f, alpha * 0.8f); // Yellow to Green
				}
				else if (colorPhase < 3.0f) {
					particleColor = glm::vec4(0.0f, 1.0f, colorPhase - 2.0f, alpha * 0.8f); // Green to Cyan
				}
				else if (colorPhase < 4.0f) {
					particleColor = glm::vec4(0.0f, 4.0f - colorPhase, 1.0f, alpha * 0.8f); // Cyan to Blue
				}
				else if (colorPhase < 5.0f) {
					particleColor = glm::vec4(colorPhase - 4.0f, 0.0f, 1.0f, alpha * 0.8f); // Blue to Magenta
				}
				else {
					particleColor = glm::vec4(1.0f, 0.0f, 6.0f - colorPhase, alpha * 0.8f); // Magenta to Red
				}
				
				// Temporarily set material color
				meshList[13]->setDiffuse(particleColor);
				meshList[13]->setAmbient(particleColor);
				meshList[13]->setSpecular(particleColor);
				
				glm::mat4 instance = glm::mat4(1.0);
				instance = glm::translate(instance, it->position + glm::vec3(offsetX, height, offsetZ));
				instance = glm::scale(instance, glm::vec3(scale, scale, scale));
				instance = glm::rotate(instance, glm::radians(angle1 + (float)(elapsed * 40.0)), glm::vec3(0.0f, 1.0f, 0.0f));
				
				painter->drawMesh(13, instance, light, camera, 0);
			}
			// Restore original bubble color
			meshList[13]->setDiffuse(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
			meshList[13]->setAmbient(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
			meshList[13]->setSpecular(glm::vec4(1.0f, 1.0f, 1.0f, 0.3f));
			glDepthMask(GL_TRUE);
			++it;
		}
		else {
			it->active = false;
			++it;
		}
	}
	
	// 如果球被标记，实时更新小臂角度指向目标球
	if (control_mode == 4 && ball_marked && marked_ball_index >= 0 && Ballsexists[marked_ball_index]) {
		float angle = calculateLowerArmAngleToTarget(Ballslocation[marked_ball_index]);
		RobotTheta[RobotLowerArmJoint] = angle;
	}
	
	modelMatrix = glm::mat4(1.0);
	draw_robot(modelMatrix);
}

void printHelp() {
	std::cout << "================================================" << std::endl;
	std::cout << "Pool Game - Keyboard Controls" << std::endl;
	std::cout << "================================================" << std::endl << std::endl;

	std::cout << "[Mode Selection]" << std::endl;
	std::cout << "Shift+1:	Mode 1 - Camera Control" << std::endl;
	std::cout << "Shift+2:	Mode 2 - Test Plane Control" << std::endl;
	std::cout << "Shift+3:	Mode 3 - Ball Aiming Mode" << std::endl;
	std::cout << "Shift+4:	Mode 4 - Robot Arm Control" << std::endl;
	std::cout << "Shift+5:	Mode 5 - Light Source Control" << std::endl;
	std::cout << std::endl;

	std::cout << "[Mode 1 - Camera Control]" << std::endl;
	std::cout << "W/A/S/D:	Move Forward/Left/Backward/Right" << std::endl;
	std::cout << "Z/X:		Move Up/Down" << std::endl;
	std::cout << "Q/E:		Turn Right/Left" << std::endl;
	std::cout << "R/F:		Turn Up/Down" << std::endl;
	std::cout << "Mouse:		Look Around (Smooth)" << std::endl;
	std::cout << "Scroll:		Zoom In/Out" << std::endl;
	std::cout << std::endl;

	std::cout << "[Mode 3 - Ball Aiming Mode]" << std::endl;
	std::cout << "I:		Reset White Ball" << std::endl;
	std::cout << "Shift+I:	Reset All Balls" << std::endl;
	std::cout << "P:		Test Kick" << std::endl;
	std::cout << std::endl;

	std::cout << "[Mode 4 - Robot Arm Control]" << std::endl;
	std::cout << "1/2/3/4:	Select Joint (Base/UpperArm/LowerArm/Hand)" << std::endl;
	std::cout << "A/S:		Increase/Decrease Joint Angle" << std::endl;
	std::cout << "W/D:		Move Robot Around Table (Forward/Backward)" << std::endl;
	std::cout << std::endl;

	std::cout << "[Mode 5 - Light Source Control]" << std::endl;
	std::cout << "W/S:		Move Light Forward/Backward (Z-axis)" << std::endl;
	std::cout << "A/D:		Move Light Left/Right (X-axis)" << std::endl;
	std::cout << "R/F:		Move Light Up/Down (Y-axis)" << std::endl;
	std::cout << std::endl;

	std::cout << "[General]" << std::endl;
	std::cout << "ESC:		Exit Program" << std::endl;
	std::cout << "T:		Toggle Camera Stop (Freeze mouse control)" << std::endl;
	std::cout << "H:		Print This Help Message" << std::endl;
	std::cout << std::endl;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (action == GLFW_PRESS || action == GLFW_REPEAT) {
		//控制按键模式
		if (key == GLFW_KEY_1 && mode == GLFW_MOD_SHIFT) {
			control_mode = 1; //控制相机
		}
		else if (key == GLFW_KEY_2 && mode == GLFW_MOD_SHIFT) {
			control_mode = 2; //控制测试平面
		}
		else if (key == GLFW_KEY_3 && mode == GLFW_MOD_SHIFT) {
			control_mode = 3; //控制击球相关动作
		}
		else if (key == GLFW_KEY_4 && mode == GLFW_MOD_SHIFT) {
			control_mode = 4; //控制层级建模
			std::cout << "Switch to Robot Arm Control Mode" << std::endl;
		}
		else if (key == GLFW_KEY_5 && mode == GLFW_MOD_SHIFT) {
			control_mode = 5; //控制光源位置
		}
		// 机械手臂控制：选择关节
		else if (key == GLFW_KEY_1 && control_mode == 4 && mode != GLFW_MOD_SHIFT) {
			RobotAxis = RobotBaseJoint;
			std::cout << "Select: Base Joint" << std::endl;
		}
		else if (key == GLFW_KEY_2 && control_mode == 4 && mode != GLFW_MOD_SHIFT) {
			RobotAxis = RobotUpperArmJoint;
			std::cout << "Select: Upper Arm Joint" << std::endl;
		}
		else if (key == GLFW_KEY_3 && control_mode == 4 && mode != GLFW_MOD_SHIFT) {
			RobotAxis = RobotLowerArmJoint;
			std::cout << "Select: Lower Arm Joint" << std::endl;
		}
		else if (key == GLFW_KEY_4 && control_mode == 4 && mode != GLFW_MOD_SHIFT) {
			RobotAxis = RobotHandJoint;
			std::cout << "Select: Hand Joint" << std::endl;
		}
		//台球相关以及调试
		else if (key == GLFW_KEY_T) { 
			if (stop_camera) stop_camera = false;
			else stop_camera = true;
		}
		else if (key == GLFW_KEY_I) {
			if (mode != GLFW_MOD_SHIFT) {
				Ballslocation[0] = glm::vec3 (0.585f, balls_bottom, -0.10f);
				Ballsvelocity[0] = glm::vec3 (0.0f, 0.0f, 0.0f);
				Ballsaccelerate[0] = glm::vec3 (0.0f, 0.0f, 0.0f);
				Ballsexists[0] = true;
				Ballsdroping[0] = false;
				previous_ball_exists[0] = true;
			}
		else {
			Ballsinit();
			for (int i = 0; i < Balls.size(); i++) {
				if (i == 0) {
					updateBallTexture(i, "");
				}
				else {
					updateBallTexture(i, "./assets/cat.png");
				}
				Ballsdroping[i] = false;
				previous_ball_exists[i] = true;
				previous_ball_locations[i] = Ballslocation[i];
			}
		}
		}
		//模型移动，相机，人物，测试平面
		else if (key == GLFW_KEY_W) {
			if (control_mode == 4) {
				robot_path_progress += 0.01f;
				if (robot_path_progress >= 1.0f) robot_path_progress -= 1.0f;
			}
			else if (control_mode == 1 && mode != GLFW_MOD_SHIFT) {
				camera->keyboard(FORWARD); //相机向前走
			}
			else if (control_mode == 2 && mode != GLFW_MOD_SHIFT) {
				testlocation.y += 0.01;
			}
			else if (control_mode == 5) {
				lightLocation.z += 0.1;
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_S) {
			if (control_mode == 4) {
				if (RobotAxis == RobotLowerArmJoint) {
					// 小臂绕Y轴逆时针旋转
					RobotTheta[RobotAxis] -= 5.0f;
					if (RobotTheta[RobotAxis] < 0.0f)
						RobotTheta[RobotAxis] += 360.0f;
				}
				else if (RobotAxis == RobotHandJoint) {
					RobotTheta[RobotAxis] -= 5.0f;
					if (RobotTheta[RobotAxis] < 0.0f)
						RobotTheta[RobotAxis] += 360.0f;
				}
				else {
					RobotTheta[RobotAxis] -= 5.0f;
					if (RobotTheta[RobotAxis] < 0.0f)
						RobotTheta[RobotAxis] += 360.0f;
				}
			}
			else if (control_mode == 1 && mode != GLFW_MOD_SHIFT) {
				camera->keyboard(BACKWARD); //相机向后走
			}
			else if (control_mode == 2 && mode != GLFW_MOD_SHIFT) {
				testlocation.y -= 0.01;
			}
			else if (control_mode == 5) {
				lightLocation.z -= 0.1;
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_A) {
			if (control_mode == 4) {
				if (RobotAxis == RobotLowerArmJoint) {
					// 小臂绕Y轴顺时针旋转
					RobotTheta[RobotAxis] += 5.0f;
					if (RobotTheta[RobotAxis] > 360.0f)
						RobotTheta[RobotAxis] -= 360.0f;
				}
				else if (RobotAxis == RobotHandJoint) {
					RobotTheta[RobotAxis] += 5.0f;
					if (RobotTheta[RobotAxis] > 360.0f)
						RobotTheta[RobotAxis] -= 360.0f;
				}
				else {
					RobotTheta[RobotAxis] += 5.0f;
					if (RobotTheta[RobotAxis] > 360.0f)
						RobotTheta[RobotAxis] -= 360.0f;
				}
			}
			else if (control_mode == 1) {
				camera->keyboard(LEFT); //相机向左走
			}
			if (control_mode == 2) {
				testlocation.x += 0.001;
			}
			else if (control_mode == 5) {
				lightLocation.x += 0.1;
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_D) {
			if (control_mode == 4) {
				robot_path_progress -= 0.01f;
				if (robot_path_progress < 0.0f) robot_path_progress += 1.0f;
			}
			else if (control_mode == 1) {
				camera->keyboard(RIGHT); //相机向右走
			}
			if (control_mode == 2) {
				testlocation.x -= 0.001;
			}
			else if (control_mode == 5) {
				lightLocation.x -= 0.1;
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_Z) {
			if (control_mode == 1) {
				camera->keyboard(UP); //相机向上走
			}
		}
		else if (key == GLFW_KEY_X) {
			if (control_mode == 1) {
				camera->keyboard(DOWN); //相机向下走
			}
		}
		else if (key == GLFW_KEY_Q) {
			if (control_mode == 4) {
				if (ball_marked) {
					ball_removing = true;
					remove_start_time = glfwGetTime();
				}
				// 恢复小臂角度为默认值（与大臂对齐）
				RobotTheta[RobotLowerArmJoint] = RobotTheta[RobotUpperArmJoint];
			}
			else if (control_mode == 1) {
				camera->keyboard(TURN_RIGHT); //相机向右转
			}
			if (control_mode == 2) {
				testlocation.y += 0.1;
			}
		}
		else if (key == GLFW_KEY_5 && control_mode == 4 && mode != GLFW_MOD_SHIFT) {
			if (!ball_removing) {
				if (ball_marked && marked_ball_index >= 0 && Ballsexists[marked_ball_index]) {
					if (marked_ball_index == 0) {
						updateBallTexture(marked_ball_index, "");
					}
					else {
						updateBallTexture(marked_ball_index, "./assets/cat.png");
					}
				}
				int nearestIdx = findNearestBall();
				if (nearestIdx >= 0 && Ballsexists[nearestIdx]) {
					marked_ball_index = nearestIdx;
					ball_marked = true;
					ball_removing = false;
					updateBallTexture(nearestIdx, "./assets/pao.png");
					
					// 计算小臂指向目标球的角度
					if (Ballsexists[marked_ball_index]) {
						float angle = calculateLowerArmAngleToTarget(Ballslocation[marked_ball_index]);
						RobotTheta[RobotLowerArmJoint] = angle;
					}
				}
			}
		}
		else if (key == GLFW_KEY_E) {
			if (control_mode == 1) {
				camera->keyboard(TURN_LEFT); //相机向左转
			}
			if (control_mode == 2) {
				testlocation.y += 0.001;
			}
		}
		else if (key == GLFW_KEY_R) {
			if (control_mode == 1) {
				camera->keyboard(TURN_UP); //相机向上转
			}
			if (control_mode == 5) {
				lightLocation.y += 0.1;
				//设置光源位置
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_F) {
			if (control_mode == 1) {
				camera->keyboard(TURN_DOWN); //相机向下转
			}
			if (control_mode == 5) {
				lightLocation.y -= 0.1;
				//设置光源位置
				light->setTranslation(lightLocation);
			}
		}
		else if (key == GLFW_KEY_P) {
			test_kick(0, 1.0f);
		}
		else if (key == GLFW_KEY_O) {
			test_kick(0, 0.3f);
		}
	}
	std::cout << testlocation.x << std::endl;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
		smoothX = 0.0f;
		smoothY = 0.0f;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	if (control_mode == 3) return; //击球模式禁用鼠标移动视角（防止晃动）
	if (stop_camera == true) return; //后续新增的手动控制鼠标禁用（方便展示和截图）
	
	float smoothFactor = 0.15f;
	smoothX = smoothX * (1.0f - smoothFactor) + xoffset * smoothFactor;
	smoothY = smoothY * (1.0f - smoothFactor) + yoffset * smoothFactor;
	
	camera->ProcessMouseMovement(smoothX, smoothY);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}


void cleanData() {
	// 释放内存

	delete camera;
	camera = NULL;

	delete light;
	light = NULL;

	painter->cleanMeshes();
	painter2->cleanMeshes();

	delete painter;
	painter = NULL;

	for (int i = 0; i < meshList.size(); i++) {
		delete meshList[i];
	}
	meshList.clear();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

int main(int argc, char** argv) {
	// 初始化GLFW库，必须是应用程序调用的第一个GLFW函数
	glfwInit();

	// 配置GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// 配置窗口属性
	GLFWwindow* window = glfwCreateWindow(1200, 1200, "2023150182_lkx_dazuoye", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	//新增:调用鼠标回调函数
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 调用任何OpenGL的函数之前初始化GLAD
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Init mesh, shaders, buffer
	init();
	// 输出帮助信息
	printHelp();
	// 启用深度测试
	glEnable(GL_DEPTH_TEST);
	// 启用混合
	glEnable(GL_BLEND);
	while (!glfwWindowShouldClose(window))
	{
		display();

		// 交换颜色缓冲 以及 检查有没有触发什么事件（比如键盘输入、鼠标移动等）
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	cleanData();


	return 0;
}

// 每当窗口改变大小，GLFW会调用这个函数并填充相应的参数供你处理。
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}
