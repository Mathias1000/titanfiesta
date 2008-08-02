/* A 3D point for positions */
const float epsilon = 0.001f;
const double pi = 3.1415926535;
const double half_pi = pi / 2;
const double rad2deg = 180 / pi;
const double deg2rad = pi / 180;
struct TitanFPoint
{
	//Constructor/Destructor
	TitanFPoint( ) : x(0.0f), y(0.0f), z(0.0f) { }
	TitanFPoint( float _x, float _y, float _z ) : x(_x), y(_y), z(_z) { }
	//Operators
	TitanFPoint& operator = ( const TitanFPoint& otherpoint )
	{
		this->x = otherpoint.x;
		this->y = otherpoint.y;
		this->z = otherpoint.z;
		return *this;
	}
	TitanFPoint& operator += ( const TitanFPoint& otherpoint )
	{
		this->x += otherpoint.x;
		this->y += otherpoint.y;
		this->z += otherpoint.z;
		return *this;
	}
	TitanFPoint& operator -= ( const TitanFPoint& otherpoint )
	{
		this->x -= otherpoint.x;
		this->y -= otherpoint.y;
		this->z -= otherpoint.z;
		return *this;
	}
	TitanFPoint& operator *= ( const TitanFPoint& otherpoint )
	{
		this->x *= otherpoint.x;
		this->y *= otherpoint.y;
		this->z *= otherpoint.z;
		return *this;
	}
	TitanFPoint& operator /= ( const TitanFPoint& otherpoint )
	{
		this->x /= otherpoint.x;
		this->y /= otherpoint.y;
		this->z /= otherpoint.z;
		return *this;
	}
	TitanFPoint& operator *= ( float mul )
	{
		this->x *= mul;
		this->y *= mul;
		this->z *= mul;
		return *this;
	}
	bool operator == ( const TitanFPoint& otherpoint )
	{
		return (fabs(this->x-otherpoint.x)<epsilon && fabs(this->y-otherpoint.y)<epsilon && fabs(this->z-otherpoint.z)<epsilon);
	}
	bool operator != ( const TitanFPoint& otherpoint )
	{
		return !(*this==otherpoint);
	}
	//Functions
	float angleRadian( const TitanFPoint& otherpoint )
	{
		double res = 0.0f;
		if( this->x == otherpoint.x ) { 
			if( this->z == otherpoint.z ) 
				res = half_pi; 
			else 
				res = -half_pi; 
		}else{
			res = atan2( (otherpoint.z-this->z),(otherpoint.x-this->x) ) + half_pi; 
		}
		return (float)res;
	}
	float angleDegree( const TitanFPoint& otherpoint )
	{
		return (float)(this->angleRadian(otherpoint) * rad2deg);
	}
	float distance2D( const TitanFPoint& otherpoint )
	{
		float dx = this->x - otherpoint.x;
		float dz = this->z - otherpoint.z;
		#ifdef TITAN_USING_ASM_MATH
			return asm_sqrt( (dx*dx) + (dz*dz) );
		#else
			return sqrt( (dx*dx) + (dz*dz) );
		#endif
	}
	float distance3D( const TitanFPoint& otherpoint )
	{
		float dx = this->x - otherpoint.x;
		float dy = this->y - otherpoint.y;
		float dz = this->z - otherpoint.z;
		#ifdef TITAN_USING_ASM_MATH
			return asm_sqrt( (dx*dx) + (dz*dz) );
		#else
			return sqrt( (dx*dx) + (dy*dy) + (dz*dz) );
		#endif
	}
	float distance2D_NSQ( const TitanFPoint& otherpoint )
	{
		float dx = this->x - otherpoint.x;
		float dz = this->z - otherpoint.z;
		return (dx*dx) + (dz*dz);
	}
	float distance3D_NSQ( const TitanFPoint& otherpoint )
	{
		float dx = this->x - otherpoint.x;
		float dy = this->y - otherpoint.y;
		float dz = this->z - otherpoint.z;
		return (dx*dx) + (dy*dy) + (dz*dz);
	}
	void moveByAngleRadian( float _angle, float _distance )
	{
		#ifdef TITAN_USING_ASM_MATH
			this->x += (float)asm_sin( _angle ) * _distance;
			this->z += (float)-asm_cos( _angle ) * _distance
		#else
			this->x += (float)sinf( _angle ) * _distance;
			this->z += (float)-cosf( _angle ) * _distance;
		#endif
	}
	void moveByAngleDegree( float _angle, float _distance )
	{
		#ifdef TITAN_USING_ASM_MATH
			this->x += (float)asm_sin( (float)(_angle * deg2rad) ) * _distance;
			this->z += (float)-asm_cos( (float)(_angle * deg2rad) ) * _distance;
		#else
			this->x += (float)sinf( (float)(_angle * deg2rad) ) * _distance;
			this->z += (float)-cosf( (float)(_angle * deg2rad) ) * _distance;
		#endif
	}
	void moveByPos( const TitanFPoint& otherpoint, float _distance )
	{
		this->moveByAngleRadian( this->angleRadian( otherpoint ), _distance );
	}
	bool isInCircle( const TitanFPoint& otherpoint, float _radius )
	{
		float dx = this->x - otherpoint.x;
		float dz = this->z - otherpoint.z;
		return ( (dx * dx + dz * dz) <= (_radius * _radius) );
	}
	bool isInSphere( const TitanFPoint& otherpoint, float _radius )
	{
		float dx = this->x - otherpoint.x;
		float dy = this->y - otherpoint.y;
		float dz = this->z - otherpoint.z;
		return ( (dx * dx + dy * dy + dz * dz) <= (_radius * _radius) );		
	}
	//Members
	float x;
	float y;
	float z;
};