// #include<gz4d/util/Cache.h>
// #include <string>
// #include <iostream>
// 
// class CacheTester
// {
//     public:
//         int reps;
//         std::string data;
//         gz4d::Cache * cache;
//         gz4d::Cache::Id id;
// 
//         CacheTester(std::string d, int r, gz4d::Cache * c)
//         {
//             data = d;
//             reps = r;
//             cache = c;
//             id = 0;
//         }
//         void Unload()
//         {
//             std::cerr << "Unload: (" << data << " x " << reps << ")\n" << std::endl;
//             if(!id)
//             {
//                 id = cache->Allocate((data.size()+1)*reps);
//             }
//             else
//                 cache->Seek(id);
//             for(int i = 0; i < reps; i++)
//             {
//                 fwrite(data.c_str(),data.size()+1,1,*cache);
//             }
//         }
//         void Load()
//         {
//             std::cerr << "Load: (" << data << " x " << reps << ")\n" << std::endl;
//             cache->Seek(id);
//             char *buffer = new char[data.size()+1];
//             for(int i = 0; i < reps; i++)
//             {
//                 int ret = fread(buffer,data.size()+1,1,*cache);
//                 if(ret != 1)
//                     std::cerr << "Read error! (" << data << " x " << reps << ")" << std::endl;
//                 if(data != buffer)
//                     std::cerr << "Data mismatch: " << buffer << " (" << data << " x " << reps << ")" << std::endl;
//             }
//             delete [] buffer;
//         }
//         void Release()
//         {
//             std::cerr << "Release: (" << data << " x " << reps << ")\n" << std::endl;
//             cache->Release(id);
//             id = 0;
//         }
// };
// 
int main(int argc, char * argv[])
{
//     gz4d::Cache c(64);
//     c.PrintDebug();
//     CacheTester ct1("one",10,&c);
//     ct1.Unload();
//     c.PrintDebug();
//     ct1.Load();
//     c.PrintDebug();
//     ct1.Release();
//     c.PrintDebug();
//     ct1.Unload();
//     c.PrintDebug();
// 
//     CacheTester ct2("*** two ***",30,&c);
//     ct2.Unload();
//     c.PrintDebug();
// 
//     CacheTester ct3("+three+",8,&c);
//     ct3.Unload();
//     c.PrintDebug();
// 
//     ct2.Load();
//     c.PrintDebug();
//     ct2.Release();
//     c.PrintDebug();
// 
//     CacheTester ct4("four",15,&c);
//     ct4.Unload();
//     c.PrintDebug();
//     ct4.Release();
//     c.PrintDebug();
// 
    return 0;
}
