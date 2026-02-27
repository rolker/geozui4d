#include <gz4d/gPhoto/Camera.h>

int main(int argc, char * argv[])
{
    gz4d::gphoto::Camera camera;
    camera.Capture(argv[1]);
}
