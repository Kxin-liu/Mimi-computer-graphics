#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <float.h>
#include <cmath>
#include <map>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/projection.hpp> // 包含投影相关的函数

const double eps = 1e-6;

std::vector<TriMesh*> Balls;
float scale_rate = 0.05f; //球的缩放比例
glm::vec3 ballscale(scale_rate, scale_rate, scale_rate);
float balls_r = 0.5f * scale_rate; //球的半径为缩放比例和其原off文件的0.5f半径乘积
float tables_height = 0.743f; //桌面上表面高度
float balls_bottom = 0.743f + balls_r; //球的最底部的高度
float balls_right = 0.566f - balls_r; //球的右边的碰撞位置
float balls_left = -0.574f + balls_r; //球的左边的碰撞位置
float balls_front = 1.116 - balls_r; //球的前边的碰撞位置
float balls_back = -1.116 + balls_r; //球的后边的碰撞位置

std::vector<std::string> Ballsname = {
	"WhiteBall",
	"BlackBall",
	"PinkBall",
	"BlueBall",
	"GreenBall",
	"BrownBall",
	"YellowBall",
	"RedBall1",
	"RedBall2",
	"RedBall3",
	"RedBall4",
	"RedBall5",
	"RedBall6",
	"RedBall7",
	"RedBall8",
	"RedBall9",
	"RedBall10",
};

std::vector<bool> Ballsexists = {
	true, true, true, true,
	true, true, true, true,
	true, true, true, true,
	true, true, true, true,
	true,
};

std::vector<bool> Ballsdroping = {
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false, false, false, false,
	false,
};

std::vector<glm::vec3> Ballshole = {
	glm::vec3 (1.116f, 0.768f, -0.574f),
	glm::vec3 (1.116f, 0.768f, 0.566f),
	glm::vec3 (-1.116f, 0.768f, -0.574f),
	glm::vec3 (-1.116f, 0.768f, 0.566f),
	glm::vec3 (0.0f, 0.768f, -0.574f),
	glm::vec3 (0.0f, 0.768f, 0.566f),
};

std::string split_color(std::string s) {
    std::string color_name = "";
    for (int i = 0; i < s.size(); i++) {
        if (s[i] == 'B' && s[i + 1] == 'a') {
            break;
        }
		else {
			color_name += s[i];
		}
    }
    return color_name;
}

std::map<std::string, int> color2idx = {
	{"whiteball", 0},
	{"blackball", 1},
	{"pinkball", 2},
	{"blueball", 3},
	{"greenball", 4},
	{"brownball", 5},
	{"yellowball", 6},
	{"redball1", 7},
	{"redball2", 8},
	{"redball3", 9},
	{"redball4", 10},
	{"redball5", 11},
	{"redball6", 12},
	{"redball7", 13},
	{"redball8", 14},
	{"redball9", 15},
	{"redball10", 16},
};

std::vector<glm::vec4> BallsAmbient = {
	glm::vec4(1.0, 1.0, 1.0, 1.0), //白球 - 纯白色
	glm::vec4(0.1, 0.1, 0.1, 1.0), //黑球
	glm::vec4(0.4, 0.1, 0.4, 1.0), //粉球
	glm::vec4(0.1, 0.2, 0.6, 1.0), //蓝球
	glm::vec4(0.0, 0.5, 0.0, 1.0), //绿球
	glm::vec4(0.3, 0.2, 0.1, 1.0), //粽球
	glm::vec4(0.5, 0.5, 0.0, 1.0), //黄球
	glm::vec4(0.5, 0.0, 0.0, 1.0), //红球
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0),
	glm::vec4(0.5, 0.0, 0.0, 1.0)
};

std::vector<glm::vec4> BallsDiffuse = {
	glm::vec4(1.0, 1.0, 1.0, 1.0),
	glm::vec4(0.2, 0.2, 0.2, 1.0),
	glm::vec4(0.8, 0.2, 0.8, 1.0),
	glm::vec4(0.2, 0.3, 0.9, 1.0),
	glm::vec4(0.1, 0.9, 0.1, 1.0),
	glm::vec4(0.6, 0.4, 0.2, 1.0),
	glm::vec4(1.0, 1.0, 0.2, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
	glm::vec4(0.9, 0.1, 0.1, 1.0),
};

std::vector<glm::vec4> BallsSpecular = {
	glm::vec4(1.0, 1.0, 1.0, 1.0),
	glm::vec4(0.4, 0.4, 0.4, 1.0),
	glm::vec4(1.0, 0.3, 1.0, 1.0),
	glm::vec4(0.3, 0.4, 1.0, 1.0),
	glm::vec4(0.2, 1.0, 0.2, 1.0),
	glm::vec4(0.8, 0.6, 0.3, 1.0),
	glm::vec4(1.0, 1.0, 0.5, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
	glm::vec4(1.0, 0.2, 0.2, 1.0),
};

std::vector<float> BallsShininess = {
	60.0f, 50.0f, 35.0f, 45.0f,
	30.0f, 25.0f, 40.0f, 40.0f,
	40.0f, 40.0f, 40.0f, 40.0f,
	40.0f, 40.0f, 40.0f, 40.0f,
	40.0f,
};


std::vector<glm::vec3> Ballslocation = {
	//第一维是桌上长向
	//第二维是高度
	//第三维是桌上宽向
	//球的半径为0.05（原半径为0.5，但是scale=0.1）
	glm::vec3 (0.585f, 1.00f, -0.10f), //白球
	glm::vec3 (-0.925f, 1.00f, 0.00f), //黑球
	glm::vec3 (-0.565f, 1.00f, 0.00f), //粉球
	glm::vec3 (0.00f, 1.00f, 0.00f), //蓝球
	glm::vec3 (0.585f, 1.00f, 0.20f), //绿球
	glm::vec3 (0.585f, 1.00f, 0.00f), //粽球
	glm::vec3 (0.585f, 1.00f, -0.20f), //黄球
	glm::vec3 (-0.615f, 1.00f, 0.00f), //红1
	glm::vec3 (-0.6583f, 1.00f, 0.025f), //红2
	glm::vec3 (-0.6583f, 1.00f, -0.025f), //红3
	glm::vec3 (-0.7016f, 1.00f, 0.05f), //红4
	glm::vec3 (-0.7016f, 1.00f, 0.00f), //红5
	glm::vec3 (-0.7016f, 1.00f, -0.05f), //红6
	glm::vec3 (-0.7449f, 1.00f, 0.075f), //红7
	glm::vec3 (-0.7449f, 1.00f, 0.025f), //红8
	glm::vec3 (-0.7449f, 1.00f, -0.025f), //红9
	glm::vec3 (-0.7449f, 1.00f, -0.075f), //红10
};

std::vector<glm::vec3> Ballsvelocity = {
	glm::vec3 (0.0f, -0.01f, 0.0f), //白球
	glm::vec3 (0.0f, -0.01f, 0.0f), //黑球
	glm::vec3 (0.0f, -0.01f, 0.0f), //粉球
	glm::vec3 (0.0f, -0.01f, 0.0f), //蓝球
	glm::vec3 (0.0f, -0.01f, 0.0f), //绿球
	glm::vec3 (0.0f, -0.01f, 0.0f), //粽球
	glm::vec3 (0.0f, -0.01f, 0.0f), //黄球
	glm::vec3 (0.0f, -0.01f, 0.0f), //红1
	glm::vec3 (0.0f, -0.01f, 0.0f), //红2
	glm::vec3 (0.0f, -0.01f, 0.0f), //红3
	glm::vec3 (0.0f, -0.01f, 0.0f), //红4
	glm::vec3 (0.0f, -0.01f, 0.0f), //红5
	glm::vec3 (0.0f, -0.01f, 0.0f), //红6
	glm::vec3 (0.0f, -0.01f, 0.0f), //红7
	glm::vec3 (0.0f, -0.01f, 0.0f), //红8
	glm::vec3 (0.0f, -0.01f, 0.0f), //红9
	glm::vec3 (0.0f, -0.01f, 0.0f), //红10
};

std::vector<glm::vec3> Ballsaccelerate = {
	glm::vec3 (0.0f, -0.01f, 0.0f), //白球
	glm::vec3 (0.0f, -0.01f, 0.0f), //黑球
	glm::vec3 (0.0f, -0.01f, 0.0f), //粉球
	glm::vec3 (0.0f, -0.01f, 0.0f), //蓝球
	glm::vec3 (0.0f, -0.01f, 0.0f), //绿球
	glm::vec3 (0.0f, -0.01f, 0.0f), //粽球
	glm::vec3 (0.0f, -0.01f, 0.0f), //黄球
	glm::vec3 (0.0f, -0.01f, 0.0f), //红1
	glm::vec3 (0.0f, -0.01f, 0.0f), //红2
	glm::vec3 (0.0f, -0.01f, 0.0f), //红3
	glm::vec3 (0.0f, -0.01f, 0.0f), //红4
	glm::vec3 (0.0f, -0.01f, 0.0f), //红5
	glm::vec3 (0.0f, -0.01f, 0.0f), //红6
	glm::vec3 (0.0f, -0.01f, 0.0f), //红7
	glm::vec3 (0.0f, -0.01f, 0.0f), //红8
	glm::vec3 (0.0f, -0.01f, 0.0f), //红9
	glm::vec3 (0.0f, -0.01f, 0.0f), //红10
};


void Ballsinit() {
    for (int i = 0; i < Balls.size(); i++) {
        Ballsvelocity[i] = glm::vec3 (0.0f, -0.01f, 0.0f);
        Ballsaccelerate[i] = glm::vec3 (0.0f, -0.01f, 0.0f);
		Ballsdroping[i] = false;
		Ballsexists[i] = true;
    }
	// 所有球重置时都在桌面上（使用balls_bottom作为y坐标）
	Ballslocation[0] = glm::vec3 (0.585f, balls_bottom, -0.10f); //白球
	Ballslocation[1] = glm::vec3 (-0.925f, balls_bottom, 0.00f); //黑球
	Ballslocation[2] = glm::vec3 (-0.565f, balls_bottom, 0.00f); //粉球
	Ballslocation[3] = glm::vec3 (0.00f, balls_bottom, 0.00f); //蓝球
	Ballslocation[4] = glm::vec3 (0.585f, balls_bottom, 0.20f); //绿球
	Ballslocation[5] = glm::vec3 (0.585f, balls_bottom, 0.00f); //粽球
	Ballslocation[6] = glm::vec3 (0.585f, balls_bottom, -0.20f); //黄球
	Ballslocation[7] = glm::vec3 (-0.615f, balls_bottom, 0.00f); //红1
	Ballslocation[8] = glm::vec3 (-0.6583f, balls_bottom, 0.025f); //红2
	Ballslocation[9] = glm::vec3 (-0.6583f, balls_bottom, -0.025f); //红3
	Ballslocation[10] = glm::vec3 (-0.7016f, balls_bottom, 0.05f); //红4
	Ballslocation[11] = glm::vec3 (-0.7016f, balls_bottom, 0.00f); //红5
	Ballslocation[12] = glm::vec3 (-0.7016f, balls_bottom, -0.05f); //红6
	Ballslocation[13] = glm::vec3 (-0.7449f, balls_bottom, 0.075f); //红7
	Ballslocation[14] = glm::vec3 (-0.7449f, balls_bottom, 0.025f); //红8
	Ballslocation[15] = glm::vec3 (-0.7449f, balls_bottom, -0.025f); //红9
	Ballslocation[16] = glm::vec3 (-0.7449f, balls_bottom, -0.075f); //红10
}


//目前完成了
//用速度更新位置
//用加速度更新速度
//垂直高度下的与桌面碰撞
void Ballsmove() {
	for (int i = 0; i < Balls.size(); i++) {
		// std::cout << Ballslocation[i].y << std::endl;
		Ballslocation[i] += 0.01f * Ballsvelocity[i];
		glm::vec3 prev_velocity = Ballsvelocity[i];
		Ballsvelocity[i] += 0.01f * Ballsaccelerate[i];

		glm::vec3 temp = -glm::normalize(Ballsvelocity[i]);
		if (prev_velocity.x * Ballsvelocity[i].x <= eps) { //x轴方向判断运动停止
			Ballsvelocity[i].x = 0.0f;
			Ballsaccelerate[i].x = 0.0f;
		}
		if (prev_velocity.z * Ballsvelocity[i].z <= eps) { //z轴方向判断运动停止
			Ballsvelocity[i].z = 0.0f;
			Ballsaccelerate[i].z = 0.0f;
		}
		if (Ballslocation[i].y <= balls_bottom && !Ballsdroping[i]) { //垂直方向（y轴）碰撞判定
            Ballslocation[i].y = balls_bottom; //避免速度过快直接穿墙卡进去
			Ballsvelocity[i].y = -0.1f * Ballsvelocity[i].y; //反弹
		}
		if (Ballslocation[i].z >= balls_right) { //宽度方向（z轴）碰撞判定
			Ballslocation[i].z = balls_right;
			Ballsvelocity[i].z = -Ballsvelocity[i].z;
			Ballsaccelerate[i].z = -Ballsaccelerate[i].z;
		}
		if (Ballslocation[i].z <= balls_left) { //宽度方向（z轴）碰撞判定
			Ballslocation[i].z = balls_left;
			Ballsvelocity[i].z = -Ballsvelocity[i].z;
			Ballsaccelerate[i].z = -Ballsaccelerate[i].z;
		}
		if (Ballslocation[i].x >= balls_front) { //长度方向（x轴）碰撞判定
			Ballslocation[i].x = balls_front;
			Ballsvelocity[i].x = -Ballsvelocity[i].x;
			Ballsaccelerate[i].x = -Ballsaccelerate[i].x;
		}
		if (Ballslocation[i].x <= balls_back) { //长度方向（x轴）碰撞判定
			Ballslocation[i].x = balls_back;
			Ballsvelocity[i].x = -Ballsvelocity[i].x;
			Ballsaccelerate[i].x = -Ballsaccelerate[i].x;
		}

		// /* 判断球和其他球之间的碰撞
		for (int j = i + 1; j < Balls.size(); j++) {
			if (!Ballsexists[i]) continue;
			float distance_ij = glm::length(Ballslocation[i] - Ballslocation[j]);
			if (distance_ij <= 2.0f * balls_r) {
				//更新两个小球的位置，防止卡住
				float len = glm::length(Ballslocation[i] - Ballslocation[j]);
				float plus_len = (balls_r * 2.0f - len) / 2.0f;
				glm::vec3 dir = Ballslocation[i] - Ballslocation[j];
				//i球沿着这个方向进行更新
				Ballslocation[i] += dir * (plus_len / len);
				Ballslocation[j] -= dir * (plus_len / len);

				//先考虑i的速度变化
				glm::vec3 ipengj = Ballslocation[j] - Ballslocation[i];
				float anglei = glm::angle(Ballsvelocity[i], ipengj);
				// float dotProduct = glm::dot(Ballsvelocity[i], ipengj); // a和b的点积
				// float bSquaredLength = glm::dot(ipengj, ipengj); // b的模长的平方
				// glm::vec3 duichongi = (dotProduct / bSquaredLength) * ipengj;
				glm::vec3 duichongi = glm::proj(Ballsvelocity[i], ipengj);
				glm::vec3 shuipingi = Ballsvelocity[i] - duichongi;

				//考虑j的速度变化
				glm::vec3 jpengi = Ballslocation[i] - Ballslocation[j];
				float anglej = glm::angle(Ballsvelocity[j], jpengi);
				// dotProduct = glm::dot(Ballsvelocity[j], jpengi); // a和b的点积
				// bSquaredLength = glm::dot(jpengi, jpengi); // b的模长的平方
				// glm::vec3 duichongj = (dotProduct / bSquaredLength) * jpengi;
				glm::vec3 duichongj = glm::proj(Ballsvelocity[j], jpengi);
				glm::vec3 shuipingj = Ballsvelocity[j] - duichongj;

				//速度交换
				Ballsvelocity[i] = duichongj + shuipingi;
				Ballsvelocity[j] = duichongi + shuipingj;
				Ballsvelocity[i] *= 0.95f;
				Ballsvelocity[j] *= 0.95f;

				//更新加速度
				Ballsaccelerate[i].x = -0.05 * Ballsvelocity[i].x;
				Ballsaccelerate[i].z = -0.05 * Ballsvelocity[i].z;
				Ballsaccelerate[j].x = -0.05 * Ballsvelocity[j].x;
				Ballsaccelerate[j].z = -0.05 * Ballsvelocity[j].z;
			}
		}
		// */
		if (Ballsdroping[i]) {
			if (Ballslocation[i].y <= 0.743f - balls_r) {
				Ballsexists[i] = false;
			}
		}

		for (int j = 0; j < 6; j++) {
			float ball_hole_d = glm::length(Ballslocation[i] - Ballshole[j]);
			if (ball_hole_d <= balls_r * 1.8f) {
				Ballsdroping[i] = true;
				Ballsvelocity[i].x /= 2.0f;
				Ballsvelocity[i].z /= 2.0f;
				Ballsaccelerate[i].x /= 10.0f;
				Ballsaccelerate[i].z /= 10.0f;
			}
		}

	}
}