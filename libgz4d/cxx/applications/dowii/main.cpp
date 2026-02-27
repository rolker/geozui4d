#include "DowiiWindow.h"
#include "Dowii.h"

int main(int argc, char * argv[])
{
    gz4d::Dowii dowii;
    DowiiWindow d("Dowii",boost::filesystem::path("/home/roland/work/models/auvs/Fetch/Fetch.x3d"),&dowii);
    d.mainLoop();
    return 0;
}
