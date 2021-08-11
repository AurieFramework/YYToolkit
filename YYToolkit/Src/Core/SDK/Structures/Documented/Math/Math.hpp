#pragma once
struct Vector3D
{
private:
	union
	{
		float Values[3];
		struct
		{
			float x;
			float y;
			float z;
		};
	};
public:
	inline const float& x() const { return x; }
	inline const float& y() const { return y; }
	inline const float& z() const { return z; }

	inline float& x() { return x; }
	inline float& y() { return y; }
	inline float& z() { return z; }

	inline float* operator&() { return Values; }
	inline const float* operator&() const { return Values; }
};