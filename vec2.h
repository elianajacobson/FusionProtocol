#pragma once

struct vec2 {
	vec2() : x(0.0), y(0.0) {}
	vec2(double x, double y) : x(x), y(y) {}

	vec2 operator*(double scalar) const {
		return vec2(scalar * x, scalar * y);
	}
	vec2 operator+(const vec2 & other) const {
		return vec2(x + other.x, y + other.y);
	}
	vec2 operator-(const vec2 & other) const {
		return vec2(x - other.x, y - other.y);
	}

	double modulus() const {
		return sqrt(x * x + y * y);
	}

	union {
		double x, x1;
	};
	union {
		double y, x2;
	};
};

vec2 operator*(double scalar, const vec2 & vec) {
	return vec * scalar;
}