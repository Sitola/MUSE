#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

typedef struct
{
	int x;
	int y;
} pixel;

typedef struct
{
	char appname[40];
	int appID,
		sailID,
		zValue;

	pixel bottom_left; // sustava v SAGE je cislovana zdola nahor zlava doprava
	pixel top_right;

} sage_window;

typedef struct
{
	int id;
	bool alive;
	pixel start;
	pixel end;
	int appID;
} musa_touch;
#endif
