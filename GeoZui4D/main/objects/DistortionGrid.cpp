// #include "DistortionGrid.h"
// 
// REGISTER_OBJECT(DistortionGrid,"");
// 
// void DistortionGrid::Idle(double time)
// {
//     //Lock();
//     iterator c = GetLastTime(time);
//     if (c != end())
//         currentRaster = c->second;
//     else
//         currentRaster.reset();
//     //Unlock();
// }
// 
// void DistortionGrid::DrawLowRes(float degradation, Window3D *win)
// {
//     if(currentRaster)
//     {
//         dg.SetData(currentRaster);
//         glPushMatrix();
//         glRotatef(90.0,1.0,0.0,0.0);
//         EnableTexture(TEXTURE_COORD_NONE);
//         dg.Draw();
//         DisableTexture();
//         glPopMatrix();
//     }
// }
// 
// int DistortionGrid::perform(int argc, const char **argv, Tcl_Interp *interp)
// {
//     if (argc)
//     {
//         std::string cmd(argv[0]);
//         if (cmd == "connect")
//         {
//             int ret = Object::perform(argc, argv, interp);
//             socket.SetOption(gz4d::Socket::RecvBuffer,524288);
//             if (ret == TCL_OK)
//             {
//                 Start(socket);
//             }
//             return ret;
//         }
//     }
//     return Object::perform(argc,argv,interp);
// }
