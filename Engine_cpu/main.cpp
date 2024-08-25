#include"MatrixVector.h"
#define NOMINMAX
#define PI 3.1415926
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <easyx.h>


int Screenwidth = 1440, Screenheight = 960;

struct point_2d {
	float x, y;
	point_2d(){}
	point_2d(Vector3 xx) {
		x = xx.v[0], y = xx.v[1]; return;
	}
	point_2d operator*(float xx) {
		point_2d ans = *this;
		x *= xx, y *= xx;
		return ans;
	}
	point_2d operator+(float xx) {
		point_2d ans = *this;
		x += xx, y += xx;
		return ans;
	}
	point_2d operator+(point_2d xx) {
		point_2d ans = *this;
		x += xx.x, y += xx.y;
		return ans;
	}
	Vector3 toVector3() {
		return Vector3{ x,y,0 };
	}
};

class Buffer{
	struct pixel {
		Vector3 p;
		point_2d pp;
		DWORD color;
		
		pixel() { color = WHITE; return; }
		pixel(Vector3 x, point_2d y, COLORREF c) { p = x, pp = y, color = c; return; }

		pixel operator*(float x) {
			pixel ans=*this;
			ans.p = ans.p * x;
			ans.pp = ans.pp * x;
			ans.color = ans.color * x;
			return ans;
		}
		pixel operator+(pixel x) {
			pixel ans = *this;
			ans.p = ans.p + x.p;
			ans.pp = ans.pp + x.pp;
			ans.color = ans.color + x.color;
			return ans;
		}
	};

	std::vector<pixel> buffer;

	DWORD* g_pBuf;
public:
	Buffer() { g_pBuf = GetImageBuffer(); }
	~Buffer(){}

	void fast_putpixel(int x, int y, COLORREF c) {
		x *= scaleX; y *= scaleY;
		x += screen_origin.x; y += screen_origin.y;
		g_pBuf[y * Screenwidth + x] = BGR(c);
	}

	COLORREF fast_getpixel(int x, int y) {
		x *= scaleX; y *= scaleY;
		x += screen_origin.x; y += screen_origin.y;
		COLORREF c = g_pBuf[y * Screenwidth + x];
		return BGR(c);
	}

	void Line(Vector3 p1, Vector3 p2) {
		if (p1.v[0] > p2.v[0])std::swap(p1, p2);

		if (fabs(p2.v[0] - p1.v[0]) < 0.001) {
			if (p1.v[1] > p2.v[1]) std::swap(p1, p2);
			for (float y = p2.v[1]; y >= p1.v[1]; y -= 1) {
				fast_putpixel((int)p1.v[0], (int)y, WHITE);
			}
			return;
		}

		float x = p1.v[0];
		float y = p1.v[1];
		float k = (p1.v[1] - p2.v[1]) / (p1.v[0] - p2.v[0]);
		for (; x < p2.v[0]; x++) {
			fast_putpixel((int)x, (int)y, WHITE);
			y += k;
		}
		return;
	}

	void line1(Vector3 p1, Vector3 p2, int screenwidth, int screenheight, point_2d p) {

		Vector3 pt = p.toVector3();
		auto inScreen = [&screenwidth, &screenheight](Vector3 p) { return fabs(p.v[0]) < (screenwidth + 1) / 2.0f && fabs(p.v[1]) < (screenheight + 1) / 2.0f; };

		if ((!inScreen(p1)) && (inScreen(p2)))std::swap(p1, p2);

		if (inScreen(p1)) {
			if (inScreen(p2)) {
				Line(p1 + pt, p2 + pt);
				return;
			}
			else {
				auto inScreen1 = [&screenwidth, &screenheight, p1, p2](Vector3 p) { Vector3 paa = p1, pbb = p2; if (paa.v[0] > pbb.v[0])std::swap(paa, pbb); return (fabs(p.v[0]) < (screenwidth + 1) / 2.0f && fabs(p.v[1]) < (screenheight + 1) / 2.0f) && (paa.v[0] < p.v[0] && p.v[0] < pbb.v[0]); };

				if (fabs(p1.v[1] - p2.v[1]) > 1 && fabs(p1.v[0] - p2.v[0]) < 0.001) {
					Vector3 res;

					res.v[0] = p1.v[0], res.v[1] = screenheight / 2.0f, res.v[2] = 0;
					if (inScreen1(res)) {
						Line(res + pt, p1 + pt);
					}
					else {
						res.v[0] = p1.v[0], res.v[1] = -screenheight / 2.0f, res.v[2] = 0;
						Line(res + pt, p1 + pt);
					}
					return;
				}

				float ky = (p1.v[1] - p2.v[1]) / (p1.v[0] - p2.v[0]);
				float kz = (p1.v[2] - p2.v[2]) / (p1.v[0] - p2.v[0]);

				float by = p1.v[1] - p1.v[0] * ky;
				float bz = p1.v[2] - p1.v[0] * kz;

				Vector3
					t(((-screenheight / 2.0f) - by) / ky, (-screenheight / 2.0f), kz * (((-screenheight / 2.0f) - by) / ky) + bz);
				if (inScreen1(t)) {
					Line(t + pt, p1 + pt); return;
				}
				t = { ((screenheight / 2.0f) - by) / ky, (+screenheight / 2.0f), kz * (((+screenheight / 2.0f) - by) / ky) + bz };
				if (inScreen1(t)) {
					Line(t + pt, p1 + pt); return;
				}
				t = { (-screenwidth / 2.0f), (ky * (-screenwidth / 2.0f) + by), (-screenwidth / 2.0f) * kz + bz };
				if (inScreen1(t)) {
					Line(t + pt, p1 + pt); return;
				}
				t = { (+screenwidth / 2.0f), (ky * (+screenwidth / 2.0f) + by), (+screenwidth / 2.0f) * kz + bz };
				if (inScreen1(t)) {
					Line(t + pt, p1 + pt); return;
				}
			}
		}
		else {
			if (fabs(p1.v[1] - p2.v[1]) > 1 && fabs(p1.v[0] - p2.v[0]) < 0.001) {
				Vector3 res1;
				Vector3 res2;

				res1.v[0] = p1.v[0], res1.v[1] = screenheight / 2.0f, res1.v[2] = 0;
				res2.v[0] = p1.v[0], res2.v[1] = -screenheight / 2.0f, res2.v[2] = 0;
				Line(res1 + pt, res2 + pt);
				return;
			}

			float ky = (p1.v[1] - p2.v[1]) / (p1.v[0] - p2.v[0]);
			float kz = (p1.v[2] - p2.v[2]) / (p1.v[0] - p2.v[0]);

			float by = p1.v[1] - p1.v[0] * ky;
			float bz = p1.v[2] - p1.v[0] * kz;

			std::pair<bool, Vector3> ps[4];

			Vector3
				t(((-screenheight / 2.0f) - by) / ky, (-screenheight / 2.0f), kz * (((-screenheight / 2.0f) - by) / ky) + bz);
			ps[0] = std::make_pair(inScreen(t), t);
			t = { ((+screenheight / 2.0f) - by) / ky, (+screenheight / 2.0f), kz * (((+screenheight / 2.0f) - by) / ky) + bz };
			ps[1] = std::make_pair(inScreen(t), t);
			t = { (-screenwidth / 2.0f), (ky * (-screenwidth / 2.0f) + by), (-screenwidth / 2.0f) * kz + bz };
			ps[2] = std::make_pair(inScreen(t), t);
			t = { (+screenwidth / 2.0f), (ky * (+screenwidth / 2.0f) + by), (+screenwidth / 2.0f) * kz + bz };
			ps[3] = std::make_pair(inScreen(t), t);

			int numTrue = 0;

			for (int i = 0; i < 4; i++) if (ps[i].first)numTrue++;

			if (numTrue == 2) {
				Vector3 res[2];
				int cnt = 0;
				for (auto item : ps) {
					if (item.first) {
						res[cnt++] = item.second;
					}
				}
				Line(res[0] + pt, res[1] + pt);
			}
			else if (numTrue == 3) {
				if (ps[0].first && ps[1].first)Line(ps[0].second + pt, ps[1].second + pt);
				else Line(ps[2].second + pt, ps[3].second + pt);
			}
			else if (numTrue == 4) {
				Line(ps[0].second + pt, ps[1].second + pt);
			}
			return;
		}

		return;
	}

	void half_Line(Vector3 p1, Vector3 p2, int screenwidth, int screenheight, point_2d p) {

		Vector3 pt = p.toVector3();

		if (fabs(p1.v[0] - p2.v[0]) < 0.001) {
			p2.v[0] = p1.v[0];
			p2.v[1] = ((p2.v[1] < p1.v[1]) ? -1 : 1) * screenheight * 10;

			float kz = (p1.v[2] - p2.v[2]) / (p1.v[1] - p2.v[1]);
			float bz = p2.v[2] - p2.v[1] * kz;

			p2.v[2] = p2.v[2] * kz + bz;
			line1(p1 + pt, p2 + pt, screenwidth, screenheight, p);

			return;
		}

		float ky = (p1.v[1] - p2.v[1]) / (p1.v[0] - p2.v[0]);
		float kz = (p1.v[2] - p2.v[2]) / (p1.v[0] - p2.v[0]);

		float by = p1.v[1] - p1.v[0] * ky;
		float bz = p1.v[2] - p1.v[0] * kz;

		p2.v[0] = p1.v[0] + (!(p2.v[0] < p1.v[0]) ? 1 : -1) * screenwidth;
		p2.v[1] = (p1.v[0] + (!(p2.v[0] < p1.v[0]) ? 1 : -1) * screenwidth) * ky + by;
		p1.v[2] = p1.v[0] * kz + bz;
		p2.v[2] = p1.v[0] * kz + bz;
		line1(p1 + pt, p2 + pt, screenwidth, screenheight, p);
	}

	void Render(Object obj, int screenwidth, int screenheight, Vector3 atScreen) {
		std::vector<Vector3> p;
		std::vector<Triangle_md> f;
		obj.model.getBuffer(p, f);

		for (auto item : f) {
			Triangle temp = Triangle{ p[item.p[0]],p[item.p[1]],p[item.p[2]] };
			temp.p[0].v[0] *= screenwidth; temp.p[0].v[1] *= screenheight; temp.p[0].v[2] += atScreen.v[2];
			temp.p[1].v[0] *= screenwidth; temp.p[1].v[1] *= screenheight; temp.p[1].v[2] += atScreen.v[2];
			temp.p[2].v[0] *= screenwidth; temp.p[2].v[1] *= screenheight; temp.p[2].v[2] += atScreen.v[2];

			if (temp.p[0].v[2] > temp.p[1].v[2])std::swap(temp.p[0], temp.p[1]);
			if (temp.p[1].v[2] > temp.p[2].v[2])std::swap(temp.p[1], temp.p[2]);
			if (temp.p[0].v[2] > temp.p[1].v[2])std::swap(temp.p[0], temp.p[1]);

			if (temp.p[0].v[2] > 0) {
				line1(temp.p[0], temp.p[1], screenwidth, screenheight, atScreen);
				line1(temp.p[1], temp.p[2], screenwidth, screenheight, atScreen);
				line1(temp.p[2], temp.p[0], screenwidth, screenheight, atScreen);
				continue;
			}
			if (temp.p[1].v[2] > 0) {

				half_Line({})
				continue;
			}
			if (temp.p[2].v[2] > 0) {
				point_2d p1 = temp.p[2], pv = temp.p[0];
				float k = (p1.y - pv.y) / (p1.x - pv.x);
				float b = pv.y - pv.x * k;
				float x = ((pv.x > p1.x) ? -1 : 1) * ((float)screenwidth / 2);
				float y = k * x + b;
				if (fabs(y) > (float)screenheight / 2) {
					y = ((pv.y > p1.y) ? -1 : 1) * ((float)screenheight / 2);
					x = (y - b) / k;
				}
				line((int)(x + atScreen.v[0]), (int)(y + atScreen.v[1]), (int)(p1.x + atScreen.v[0]), (int)(p1.y + atScreen.v[1]));

				pv = temp.p[1];
				k = (p1.y - pv.y) / (p1.x - pv.x);
				b = pv.y - pv.x * k;
				x = ((pv.x > p1.x) ? -1 : 1) * ((float)screenwidth / 2);
				y = k * x + b;
				if (fabs(y) > (float)screenheight / 2) {
					y = ((pv.y > p1.y) ? -1 : 1) * ((float)screenheight / 2);
					x = (y - b) / k;
				}
				line((int)(x + atScreen.v[0]), (int)(y + atScreen.v[1]), (int)(p1.x + atScreen.v[0]), (int)(p1.y + atScreen.v[1]));

				continue;
			}

		}

		return;
	}

};

point_2d screen_origin;
float scaleX;
float scaleY;

struct Triangle_md {
	int p[3];
};
struct Triangle {
	Vector3 p[3];
};

class Camera {
public:
	Vector3 at, direction;
	float h,heigh,width;
	Camera(Vector3 x, Vector3 y,float xh, float xwidth,float xheight)
	{ at = x, direction = y, h = xh, heigh = xheight, width = xwidth; }
	void Goto(Vector3 x) {
		at = x; return;
	}
	void rotate(Vector3 x) {
		direction = x; return;
	}
	void addat(Vector3 x) {
		at = at + x; return;
	}
	void adddir(Vector3 x) {
		direction = direction + x;
		return;
	}
	void print() {
		printf("at:");
		at.print();
		printf("direction:");
		direction.print();
		printf("\n"); return;
	}
};
class CollisionBox {
public:
	Vector3 maxp, minp;

	static bool Collision(CollisionBox& box1, CollisionBox& box2) {
		for (int i = 0; i < 3; i++) {
			float t = std::max(fabs(box1.maxp.v[i] - box2.minp.v[i]), fabs(box2.maxp.v[i] - box1.minp.v[i]));
			float tadd = box1.maxp.v[i] - box1.minp.v[i] + box2.maxp.v[i] - box1.minp.v[i];
			if (t > tadd)return false;
		}
		return true;
	}

};
class Model {
	Vector3 point_change(Vector3 x) {
		Vector3 ans; x.v[2] = -x.v[2];
		float k = camerah / x.v[2];
		ans.v[0] = x.v[0] * k;
		ans.v[1] = x.v[1] * k;
		ans.v[2] = x.v[2];
		return ans;
	}
	float camerah=1;
	std::vector<Vector3> points;
public:
	std::vector<Vector3> point_2d;
	std::vector<Triangle_md> faces;
	void init_model(std::string filename) {
		std::ifstream file(filename);
		if (!file.is_open()) {
			std::cerr << "Error opening file: " << filename << std::endl;
			return;
		}
		std::string line;
		while (getline(file, line)) {
			std::stringstream ss(line);
			std::string prefix;
			ss >> prefix;

			if (prefix == "v") {
				Vector3 vertex;
				ss >> vertex.v[0] >> vertex.v[1] >> vertex.v[2];
				points.push_back(vertex);
			}
			else if (prefix == "f") {
				std::vector<int> face;
				int index;
				Vector3 indices;

				while (ss >> index) {
					// 在OBJ文件中，索引是从1开始的，将其调整为从0开始
					face.push_back(index - 1);
					// 忽略 '/' 后面的内容
					ss.ignore();
				}
				if (face.size() >= 3) {
					for (size_t i = 1; i < face.size() - 1; ++i) {
						Triangle_md t = { face[0], face[i], face[i + 1] };
						faces.push_back(t);
					}
				}
			}
		}
		file.close();
	}
	void Flush2dBuffer(Camera camera, Vector3 at, Vector3 direction) {
		camerah = camera.h;
		Matrix4 otherChange('e', 0, 0, 0), objectRotate('r', direction.v[0], direction.v[1], direction.v[2]);
		Matrix4 move('m', at.v[0] - camera.at.v[0], at.v[1] - camera.at.v[1], at.v[2] - camera.at.v[2]);
		Matrix4	rotate('r', -camera.direction.v[0], -camera.direction.v[1], -camera.direction.v[2]), changeMatrix;

		changeMatrix = rotate * move * objectRotate * otherChange;
		//changeMatrix = otherChange * objectRotate * move * rotate;

		Vector4 temp; point_2d.clear();
		for (auto item : points) {
			temp = item;
			temp = changeMatrix * temp;
			item = temp;
			item = point_change(item);
			item.v[0] = item.v[0] / camera.width;
			item.v[1] = item.v[1] / camera.heigh;
			point_2d.push_back(item);
		}

		return;
	}
	void initCollisionBox(CollisionBox& box) {
		for (int i = 0; i < 3; i++) {
			box.maxp.v[i] = FLT_MIN;
			box.minp.v[i] = FLT_MAX;
		}

		for (auto item : points) {
			for (int i = 0; i < 3; i++) {
				box.maxp.v[i] = std::max(box.maxp.v[i], item.v[i]);
				box.minp.v[i] = std::min(box.minp.v[i], item.v[i]);
			}
		}

		return;
	}

	void getBuffer(std::vector<Vector3>& p, std::vector<Triangle_md>& f) {
		p = point_2d; f = faces; return;
	}
};
class Object {
public:
	Vector3 at, direction;
	void Goto(Vector3 x) {
		at = x; return;
	}
	void rotate(Vector3 x) {
		direction = x; return;
	}
	void addat(Vector3 x) {
		at = at + x; return;
	}
	void adddir(Vector3 x) {
		direction = direction + x;
		return;
	}

	Model model;
	CollisionBox collisionBox;

	void init_model(std::string filename){
		model.init_model(filename);
		return;
	}

	void Flush2dBuffer(Camera camera) {
		model.Flush2dBuffer(camera, at, direction);
		return;
	}

	void initCollisionBox(CollisionBox& box){
		model.initCollisionBox(box);
		return;
	}

	static bool Collision(CollisionBox& box1, CollisionBox& box2){
		return CollisionBox::Collision(box1, box2);
	}

	
	
	

};

void Render_init(int screenwidth, int screenheight, int x = 0) {
	initgraph(screenwidth, screenheight, x);
	setorigin(screenwidth / 2, screenheight / 2);
	screen_origin = { screenwidth / 2.0f, screenheight / 2.0f };
	setaspectratio(1, -1);
	scaleX = 1.0f; scaleY = -1.0f;
	

	return;
}

Object obj;
Vector3 cameraPos(0, 0, 30);
Vector3 cameraRot(0, 0, 0);
Camera camera(cameraPos, cameraRot, 10, 1.44, 0.96);

void input() {
	ExMessage msg;
	while (true) {
		if (peekmessage(&msg)) {
			switch (msg.vkcode) {
			case 'W':
				camera.addat({ -sin(camera.direction.v[1]), 0, -cos(camera.direction.v[1]) });
				break;
			case 'S':
				camera.addat({ sin(camera.direction.v[1]), 0, cos(camera.direction.v[1]) });
				break;
			case 'A':
				camera.addat({ -cos(camera.direction.v[1]), 0, sin(camera.direction.v[1]) });
				break;
			case 'D':
				camera.addat({ cos(camera.direction.v[1]), 0, -sin(camera.direction.v[1]) });
				break;
			case 'Q':
				camera.adddir({ 0, 0.03, 0 });
				break;
			case 'E':
				camera.adddir({ 0, -0.03, 0 });
				break;

			case 'R':

				break;
			case 'F':

				break;
			case VK_ESCAPE:
				closegraph();
				exit(0);
			default:
				break;
			}
		}
		Sleep(10);
		//camera.print();
	}
}

int main() {
	bool run = true;
	
	obj.Goto(Vector3{ 0,0,0 });
	obj.rotate(Vector3{ 0,0,0 });
	obj.init_model("8f.obj");
	Render_init(Screenwidth, Screenheight, 1);
	BeginBatchDraw();

	std::thread inputt(input);

	while (run) {
		obj.Flush2dBuffer(camera);
		cleardevice();
		obj.Render(Screenwidth,Screenheight, Vector3{ 0,0,0 });

		FlushBatchDraw();
	}

	inputt.join();
	char c = getchar();
	EndBatchDraw();
	return 0;
}
