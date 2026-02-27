#include <gz4d/gz4d.h>
#ifdef GZ4D_XINPUT
#include <gz4d/XInput.h>

int main(int argc, char * argv[])
{
    gz4d::XInput x;
    x.Connect();
    x.Print();
    x.OpenDevice(0);
}
#else
int main(){}
#endif
