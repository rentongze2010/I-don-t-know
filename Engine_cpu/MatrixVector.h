#pragma once
#include<cmath>
#include<iostream>

struct Vector4;
struct Vector3 {
	float v[3];

	Vector3() { memset(v, 0, sizeof v); }
	Vector3(float x, float y, float z) {
		v[0] = x, v[1] = y, v[2] = z;
		return;
	}
	Vector3 operator=(const Vector4& x);
	Vector3 operator+(const Vector3& x) {
		Vector3 ans=*this;
		ans.v[0] += x.v[0]; ans.v[1] += x.v[1]; ans.v[2] += x.v[2];
		return ans;
	}
	Vector3 operator*(float x) {
		Vector3 ans = *this;
		ans.v[0] *= x, ans.v[1] *= x, ans.v[2] *= x;
		return ans;
	}
	void print() const {
		printf("%f %f %f\n", v[0], v[1], v[2]);
	}
};
struct Vector4 {
	float v[4];

	Vector4() { memset(v, 0, sizeof v); }
	Vector4(float x, float y, float z) {
		v[0] = x, v[1] = y, v[2] = z, v[3] = 1;
		return;
	}
	Vector4 operator=(const Vector3& x);
	void print() const {
		printf("%f %f %f %f\n", v[0], v[1], v[2], v[3]);
	}
};
Vector3 Vector3::operator=(const Vector4& x) {
	memcpy_s(v, 3 * sizeof(float), x.v, 3 * sizeof(float));
	return *this;
}
Vector4 Vector4::operator=(const Vector3& x) {
	memcpy_s(v, 3 * sizeof(float), x.v, 3 * sizeof(float));
	v[3] = 1;
	return *this;
}
struct Matrix4 {
	//zxy
	float m[4][4];
	Matrix4() { memset(m, 0, sizeof m); }
	//Matrix4 operator*(const Matrix4 x);
	Matrix4(char c, float x, float y, float z) {
		memset(m, 0, sizeof m);
		if (c == 'e') {			//empty
			m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1;
		}
		else if (c == 'm') {	//move
			m[0][3] = x; m[0][0] = 1;
			m[1][3] = y; m[1][1] = 1;
			m[2][3] = z; m[2][2] = 1;
			m[3][3] = 1;
		}
		else if (c == 'r') {	//rotate	
			Matrix4 t1, t2;
			//z
			t1.m[2][2] = 1;			t1.m[3][3] = 1;
			t1.m[0][0] = cos(z);	t1.m[0][1] = -sin(z);
			t1.m[1][0] = -t1.m[0][1];	t1.m[1][1] = t1.m[0][0];

			//x
			t2.m[0][0] = 1;			t2.m[3][3] = 1;
			t2.m[1][1] = cos(x);	t2.m[1][2] = -sin(x);
			t2.m[2][1] = -t2.m[1][2];	t2.m[2][2] = t2.m[1][1];
			t1 = t1 * t2;

			//y
			t2.m[1][1] = 1;			t2.m[3][3] = 1;
			t2.m[0][0] = cos(y);	t2.m[0][2] = sin(y);
			t2.m[2][0] = -t2.m[0][2];	t2.m[2][2] = t2.m[0][0];
			t1 = t1 * t2;
			memcpy_s(m, (16 * sizeof(float)), t1.m, (16 * sizeof(float)));
		}
		else if (c == 's') {	//size
			m[0][0] = x;
			m[1][1] = y;
			m[2][2] = z;
			m[3][3] = 1;
		}
		return;
	}
	Matrix4 operator*(const Matrix4 x) {
		Matrix4 ans;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				ans.m[i][j] = 0;
				for (int k = 0; k < 4; k++) {
					ans.m[i][j] += m[i][k] * x.m[k][j];
				}
			}
		}
		return ans;
	}
	Vector4 operator*(const Vector4 x) {
		Vector4 ans;
		for (int i = 0; i < 4; i++) {
			ans.v[i] = 0;
			for (int k = 0; k < 4; k++) {
				ans.v[i] += m[i][k] * x.v[k];
			}
		}
		return ans;
	}
	void print() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				printf("%f ", m[i][j]);
			}
			printf("\n");
		}
		return;
	}
};
