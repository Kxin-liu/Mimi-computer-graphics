#version 330 core

// 给光源数据一个结构体
struct Light{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	vec3 position;
};

// 给物体材质数据一个结构体
struct Material{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;

	float shininess;
};

in vec3 position;
in vec3 normal;
in vec2 texCoord;

// 相机坐标
uniform vec3 eye_position;
// 光源
uniform Light light;
// 物体材质
uniform Material material;

uniform int isShadow;
uniform sampler2D texture;
uniform bool useTexture;

out vec4 fColor;

void main()
{
	if (isShadow == 1) {
		fColor = vec4(0.0, 0.0, 0.0, 1.0);
	}
	else {

		// 将顶点坐标、光源坐标和法向量转换到相机坐标系
		// vec3 norm = (vec4(normal, 0.0)).xyz;
		vec3 norm = normalize((vec4(normal, 0.0)).xyz);
		vec3 view_dir = normalize(eye_position - position);
		vec3 light_dir = normalize(light.position - position);
		// @TODO: 计算四个归一化的向量 N,V,L,R(或半角向量H)
		// vec3 N=
		// vec3 V=
		// vec3 L=
		// vec3 R=
		vec3 N = norm;
		vec3 V = view_dir;
		vec3 L = light_dir;
		vec3 R = normalize(V + L);


		// 环境光分量I_a
		vec4 I_a = light.ambient * material.ambient;

		// @TODO: Task2 计算系数和漫反射分量I_d
		float diffuse_dot = max(dot(N, L), 0.0);
		vec4 I_d = diffuse_dot * light.diffuse * material.diffuse;

		// @TODO: Task2 计算系数和镜面反射分量I_s
		float specular_dot = max(dot(N, R), 0.0);
		float specular_dot_pow = pow(specular_dot, material.shininess);
		vec4 I_s = specular_dot_pow * light.specular * material.specular;

		// 注意如果光源在背面则去除高光
		float specular_factor = 0.0;
		if( dot(L, N) >= 0.0 ) {
		    specular_factor = specular_dot_pow;
		}

		// 合并三个分量的颜色，修正透明度
		if (useTexture) {
			vec4 texColor = texture2D(texture, texCoord);
			// 计算光照强度（只使用光照的颜色，不使用材质颜色）
			vec4 lightIntensity = light.ambient + diffuse_dot * light.diffuse + specular_factor * light.specular;
			// 将光照强度应用到纹理颜色上，但保持纹理的原始颜色
			fColor = texColor * lightIntensity;
		} else {
			if( dot(L, N) < 0.0 ) {
			    I_s = vec4(0.0, 0.0, 0.0, 1.0);
			}
			vec4 materialColor = I_a + I_d + I_s;
			fColor = materialColor;
		}
		fColor.a = 1.0;
	}
}

