import std;
import minifb;
import GLM;
import tinyobj;

using u32 = std::uint32_t;
using i32 = std::int32_t;

const float EPSILON = 0.0001f;

typedef struct _Vertex {
	vec3 pos;
	vec3 normal;
	vec3 color;
} Vertex;
struct Triangle {
	Vertex v0, v1, v2;
};

struct Light {
	float intensity;
	vec3 light;
	vec3 lightColor;
};

void loadObj(std::vector<Triangle>& scene_triangles, const std::string& inputfile, vec3 color);

auto closestHit(std::vector<Triangle>& triangles, vec3 O, vec3 D) {
	float minT = 100000000000.0f;
	float _u = 0.0f;
	float _v = 0.0f;
	bool hit = false;
	int index = -1;

	for (auto [idx, b] : std::views::enumerate(triangles))
	{
		vec3 V0 = b.v0.pos;
		vec3 V1 = b.v1.pos;
		vec3 V2 = b.v2.pos;

		vec3 S = O - V0;
		vec3 E1 = V1 - V0;
		vec3 E2 = V2 - V0;

		//std::cout << E2.x << "," << E2.y << "," << E2.z << "\n'";
		//std::cout << E1.x << "," << E1.y << "," << E1.z << "\n'";

		vec3 P = cross(D, E2);
		vec3 Q = cross(S, E1);

		float P_E1 = dot(P, E1);

		//std::cout << P_E1 << "\n'";

		float u = dot(P, S) / P_E1;
		if (u < 0.0f || u > 1.0f) continue;

		float v = dot(Q, D) / P_E1;
		if (v < 0.0f || u + v > 1.0f) continue;

		float t = dot(Q, E2) / P_E1;

		if (t < EPSILON) continue;

		minT = std::min(minT, t);

		if (t > minT) continue;

		hit = true;
		index = static_cast<int>(idx);
		_u = u;
		_v = v;
	}

	struct {
		float minT;
		float _u;
		float _v;
		bool hit;
		int index;
	} res{ minT, _u, _v, hit, index };

	return res;
}

auto anyHit(std::vector<Triangle>& triangles, int index, vec3 O, vec3 D, float max_t)
{
	bool shadow = false;

	for (auto [idx, b] : std::views::enumerate(triangles))
	{
		if (idx == index) continue;

		vec3 V0 = b.v0.pos;
		vec3 V1 = b.v1.pos;
		vec3 V2 = b.v2.pos;

		vec3 S = O - V0;
		vec3 E1 = V1 - V0;
		vec3 E2 = V2 - V0;

		//std::cout << E2.x << "," << E2.y << "," << E2.z << "\n'";
		//std::cout << E1.x << "," << E1.y << "," << E1.z << "\n'";

		vec3 P_in = cross(D, E2);
		vec3 Q = cross(S, E1);

		float P_E1 = dot(P_in, E1);
		if (std::abs(P_E1) < 0.0000001) continue;

		//std::cout << P_E1 << "\n'";

		float u = dot(P_in, S) / P_E1;
		if (u < 0.0f || u > 1.0f) continue;

		float v = dot(Q, D) / P_E1;
		if (v < 0.0f || u + v > 1.0f) continue;

		float t = dot(Q, E2) / P_E1;

		if (t < EPSILON || t > max_t - 0.0000001f) continue;

		shadow = true;
		break;
	}

	return shadow;
}

vec3 calculateNewP(vec3 O, vec3 D, float t, std::vector<Triangle>& triangles, float u, float v, int index);
void addLight(std::vector<Triangle>& scene_triangles, Light l);

int main() {
	mfb_window_ window(mfb_open_ex("my display", 800, 600, mfb_window_flags::WF_RESIZABLE));
	if (window == nullptr)
		return 0;

	u32 width = 800;
	u32 height = 600;

	std::vector<Triangle> triangles(0);
	loadObj(triangles, "ball.obj", vec3(0.2f, 0.2f, 0.0f));
	loadObj(triangles, "box.obj", vec3(0.0f, 0.0f, 0.2f));
	loadObj(triangles, "plane.obj", vec3(1.0f, 1.0f, 1.0f));
	loadObj(triangles, "plane2.obj", vec3(0.0f, 0.2f, 0.0f));
	loadObj(triangles, "plane3.obj", vec3(0.2f, 0.0f, 0.0f));

	auto buffer = std::vector<u32>(width * height);

	float fov = radians(60.0f);
	float d = 1.0f;
	float shadowFactor = 0.5f;

	//vec3 light = vec3(-0.05f, 0.2f, 0.0f);
	//float intensity = 20000.0f;
	//vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);

	std::vector<Light> lights;
	lights.push_back(Light{ .intensity = 1.0f, .light = vec3(-0.05f, 0.2f, 0.0f), .lightColor = vec3(1.0f, 1.0f, 1.0f) });
	lights.push_back(Light{ .intensity = 1.0f, .light = vec3(0.0f, 0.2f, 0.0f), .lightColor = vec3(1.0f, 1.0f, 1.0f) });
	lights.push_back(Light{ .intensity = 1.0f, .light = vec3(-0.5f, 0.2f, 0.0f), .lightColor = vec3(1.0f, 1.0f, 1.0f) });
	lights.push_back(Light{ .intensity = 2.0f, .light = vec3(0.1f, -0.2f, -0.1f), .lightColor = vec3(1.0f, 0.0f, 1.0f) });

	//for (auto l : lights) {
	//	addLight(triangles, l);
	//}

	int reflect = 2;

	vec3 O = vec3(0.0, 0.0, 1.0);
	vec3 cameraUp = vec3(0.0, 1.0, 0.0);
	vec3 cameraLook = vec3(0.0, 0.0, 0.0);

	vec3 w = normalize(O - cameraLook);
	vec3 u = normalize(cross(cameraUp, w));
	vec3 v = cross(w, u);

	float V_h = 2.0f * tan(fov / 2.0f) * d;
	float V_w = 800.0f / 600.0f * V_h;

	vec3 U = V_w * u;
	vec3 V = V_h * v;

	vec3 Q_bl = O - (U / 2.0f) - V / 2.0f - d * w;
	std::cout << w.x << "," << w.y << "," << w.z << "\n'";

	const uint8_t* keys = mfb_get_key_buffer(window.get());

	mfb_update_state state;
	do {

		if (std::abs(dot(w, cameraUp)) > 0.999) {
			// 临时将向上方向改为 Z 轴 (0, 0, 1)
			// 这样当相机直上直下看时，相机的“头顶”会指向 Z 轴方向
			cameraUp = vec3(0.0, 0.0, 1.0);
		}
		else {
			cameraUp = vec3(0.0, 1.0, 0.0);
		}

		if (keys[mfb_key::KB_KEY_W]) {
			O.z -= 0.1f;
			cameraLook.z -= 0.1f;
		}
		if (keys[mfb_key::KB_KEY_S]) {
			O.z += 0.1f;
			cameraLook.z += 0.1f;
		}
		if (keys[mfb_key::KB_KEY_A]) {
			O.x -= 0.1f;
			cameraLook.x -= 0.1f;
		}
		if (keys[mfb_key::KB_KEY_D]) {
			O.z += 0.1f;
			cameraLook.x += 0.1f;
		}

		w = normalize(O - cameraLook);
		u = normalize(cross(cameraUp, w));
		v = cross(w, u);

		U = V_w * u;
		V = V_h * v;

		Q_bl = O - (U / 2.0f) - V / 2.0f - d * w;

#pragma omp parallel for
		for (i32 i = 0; i < static_cast<i32>(height); i++)
		{
			for (i32 j = 0; j < static_cast<i32>(width); j++)
			{
				float u_i = (float(j) + 0.5f) / float(width);
				float v_j = 1.0f - (float(i) + 0.5f) / float(height);

				vec3 p = Q_bl + u_i * U + v_j * V;

				vec3 D = normalize(p - O);
				//std::cout << D.x << "," << D.y << "," << D.z << "\n'";
				vec3 totalLight = {};

				auto [minT, _u, _v, hit, index] = closestHit(triangles, O, D);

				if (hit) {
					auto P_new = calculateNewP(O, D, minT, triangles, _u, _v, index);

					vec3 normal = normalize(triangles[index].v0.normal * (1.0f - _u - _v) +
						triangles[index].v1.normal * _u +
						triangles[index].v2.normal * _v);

					if (dot(D, normal) > 0.0f) {
						normal = -normal;
					}

					for (auto lightPack : lights)
					{
						vec3 light = lightPack.light;
						float intensity = lightPack.intensity;
						vec3 lightColor = lightPack.lightColor;

						float T = distance(light, P_new);

						D = normalize(light - P_new);

						//vec3 faceNormal = vec3(0.0);
						//{
						//	vec3 V0 = triangles[index].v0.pos;
						//	vec3 V1 = triangles[index].v1.pos;
						//	vec3 V2 = triangles[index].v2.pos;

						//	vec3 E1 = V1 - V0;
						//	vec3 E2 = V2 - V0;

						//	faceNormal = normalize(cross(E1, E2));
						//}

						vec3 P_2 = P_new;
						//float flip = dot(D, normal) ? -1.0f : 1.0f;
						vec3 D_2 = D - 2 * dot(D, normal) * normal;
						std::vector<vec4> color(reflect);
						for (int r = 0; r < reflect; r++)
						{
							auto [minT_2, _u_2, _v_2, hit_2, index_2] = closestHit(triangles, P_2, D_2);

							if (!hit_2) break;

							auto P_2_new = calculateNewP(P_2, D_2, minT_2, triangles, _u_2, _v_2, index_2);

							float T_2 = distance(light, P_2_new);

							vec3 normal_2 = normalize(triangles[index_2].v0.normal * (1.0f - _u_2 - _v_2) +
								triangles[index_2].v1.normal * _u_2 +
								triangles[index_2].v2.normal * _v_2);

							if (dot(D_2, normal_2) > 0.0f) {
								normal_2 = -normal_2;
							}

							D_2 = normalize(light - P_2_new);

							//vec3 faceNormal_2 = vec3(0.0);
							//{
							//	vec3 V0 = triangles[index_2].v0.pos;
							//	vec3 V1 = triangles[index_2].v1.pos;
							//	vec3 V2 = triangles[index_2].v2.pos;

							//	vec3 E1 = V1 - V0;
							//	vec3 E2 = V2 - V0;

							//	faceNormal_2 = normalize(cross(E1, E2));
							//}

							bool shadow_2 = anyHit(triangles, index_2, P_2_new, D_2, T_2);

							float Light = 0.0f;
							float ambient = 0.2f;

							float l1 = std::max(0.0f, 1 - pow(T_2 / 100.0f, 4.0f));
							Light += intensity / (T_2 * T_2 + EPSILON) * l1 * l1;

							float diffuse = std::max(0.0f, dot(normal_2, D_2));

							Light *= diffuse;

							Light += ambient;

							if (shadow_2) {
								Light *= shadowFactor;
							}

							vec3 fc = triangles[index_2].v0.color * Light;

							color[r].x = fc.x;
							color[r].y = fc.y;
							color[r].z = fc.z;

							color[r].w = 0.9f;

							P_2 = P_2_new;
							D_2 = D_2 - 2 * dot(D_2, normal_2) * normal_2;
						}

						for (int ci = static_cast<int>(color.size() - 2); ci > 0; ci -= 1) {
							color[ci].x += color[ci + 1].x * color[ci].w;
							color[ci].y += color[ci + 1].y * color[ci].w;
							color[ci].z += color[ci + 1].z * color[ci].w;
						}

						bool shadow = anyHit(triangles, index, P_new, D, T);
						//bool shadow = false;

						float Light = 0.0f;
						float ambient = 1.0f;

						float l1 = std::max(0.0f, 1 - pow(T / 100.0f, 4.0f));
						Light += intensity / (T * T + EPSILON) * l1 * l1;

						float diffuse = std::max(0.0f, dot(normal, D));

						vec3 L = normalize(light - P_new); // 光源方向
						vec3 V_s = normalize(O - P_new); // 视线方向
						vec3 H = normalize(L + V_s); // 半角向量

						vec3 specular_color = {};

						if (!shadow) {
							// Ns 是插值得到的平滑着色法线
							float specular_intensity = pow(std::max(dot(normal, H), 0.0f), 64.0f); // 64 为高光指数，数值越大点越小越亮
							specular_color = lightColor * specular_intensity * 0.9f;
						}

						Light *= diffuse;

						Light += ambient;

						vec3 reflectColor = {};
						if (color.size() > 0) {
							reflectColor = vec3(color[0]);
						}

						vec3 finalColor = triangles[index].v0.color * Light * 0.1f + 0.9f * reflectColor + specular_color;

						if (shadow) {
							finalColor *= shadowFactor;
						}

						totalLight += finalColor;
					}

					totalLight = totalLight / (totalLight + 1.0f);

					totalLight.x = pow(totalLight.x, 1.0f / 2.2f);
					totalLight.y = pow(totalLight.y, 1.0f / 2.2f);
					totalLight.z = pow(totalLight.z, 1.0f / 2.2f);

					totalLight *= 255.0f;

					u32 color_u32 = (static_cast<uint32_t>(255) << 24) | (static_cast<uint32_t>(totalLight.r) << 16) |
						(static_cast<uint32_t>(totalLight.g) << 8) | (static_cast<uint32_t>(totalLight.b) << 0);

					buffer[i * width + j] = color_u32;
				}
			}
		}
		std::cout << "a\n";

		state = mfb_update_ex(window.get(), buffer.data(), 800, 600);

		if (state != mfb_update_state::STATE_OK)
			break;

		std::fill(buffer.begin(), buffer.end(), 0);

	} while (mfb_wait_sync(window.get()));

	return 0;
}

void loadObj(std::vector<Triangle>& scene_triangles, const std::string& inputfile, vec3 color) {
	ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./";
	reader_config.triangulate = true;

	tinyobj::ObjReader reader;


	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	//auto& materials = reader.GetMaterials();

	for (size_t s = 0; s < shapes.size(); s++) {
		size_t index_offset = 0;

		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			if (fv == 3) {
				Triangle tri;
				Vertex vertices[3];

				for (size_t v = 0; v < 3; v++) {
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					int v_idx = idx.vertex_index;
					vertices[v].pos.x = attrib.vertices[3 * v_idx + 0];
					vertices[v].pos.y = attrib.vertices[3 * v_idx + 1];
					vertices[v].pos.z = attrib.vertices[3 * v_idx + 2];

					vertices[v].color = color;

					if (idx.normal_index >= 0) {
						int n_idx = idx.normal_index;
						vertices[v].normal.x = attrib.normals[3 * n_idx + 0];
						vertices[v].normal.y = attrib.normals[3 * n_idx + 1];
						vertices[v].normal.z = attrib.normals[3 * n_idx + 2];
					}
					else {
						vertices[v].normal = { 0, 0, 0 };
					}

				}

				tri.v0 = vertices[0];
				tri.v1 = vertices[1];
				tri.v2 = vertices[2];

				scene_triangles.push_back(tri);
			}

			index_offset += fv;
		}
	}
}

vec3 calculateNewP(vec3 O, vec3 D, float t, std::vector<Triangle>& triangles, float u, float v, int index) {
	vec3 P = O + t * D;

	vec3 P_new = {};
	{
		vec3 V0 = triangles[index].v0.pos;
		vec3 V1 = triangles[index].v1.pos;
		vec3 V2 = triangles[index].v2.pos;

		vec3 N0 = triangles[index].v0.normal;
		vec3 N1 = triangles[index].v1.normal;
		vec3 N2 = triangles[index].v2.normal;

		float d_a = dot((P - V0), N0);
		float d_b = dot((P - V1), N1);
		float d_c = dot((P - V2), N2);

		vec3 P_a = P - d_a * N0;
		vec3 P_b = P - d_b * N1;
		vec3 P_c = P - d_c * N2;

		P_new = (1.0f - u - v) * P_a +
			u * P_b + v * P_c;
	}

	return P_new;
}

void addLight(std::vector<Triangle>& scene_triangles, Light l) {
	Triangle t;

	t.v0 = Vertex{ .pos =l.light, .normal = vec3(0.0f, 1.0f, 0.0f),.color = l.lightColor};
	t.v1 = t.v0;
	t.v2 = t.v0;

	scene_triangles.push_back(t);
}