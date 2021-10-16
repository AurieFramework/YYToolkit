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
	inline const float& X() const { return x; }
	inline const float& Y() const { return y; }
	inline const float& Z() const { return z; }

	inline float& X() { return x; }
	inline float& Y() { return y; }
	inline float& Z() { return z; }

	inline float* operator&() { return Values; }
	inline const float* operator&() const { return Values; }
};