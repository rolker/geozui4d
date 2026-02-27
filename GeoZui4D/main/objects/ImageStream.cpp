// #include "ImageStream.h"
// #include "MainWindow.h"
// #include "engine/Scene3D.h"
// 
// REGISTER_OBJECT(ImageStream,"");
// 
// extern MainWindow * mainWindow;
// 
// 
// ImageStream::ImageStream()
// {
//     mainWindow->getScene3D()->AddTexture(this);
// }
// 
// void ImageStream::DrawLowRes(float degradation, Window3D *win)
// {
// }
// 
// void ImageStream::Idle(double time)
// {
//     //Lock();
//     //RasterStream<gz4d::Raster<gz4d::Pixel> >::
//     iterator c = GetLastTime(time);
//     if (c != end())
//         currentImage = c->second;
//     else
//         currentImage.reset();
//     //Unlock();
// }
// 
// 
// void ImageStream::Enable()
// {
//     if(currentImage)
//     {
//         glEnable(GL_TEXTURE_2D);
//         glEnable(GL_ALPHA_TEST);
//         glAlphaFunc(GL_GREATER, transparentBlendThreshold);
//         glEnable(GL_BLEND);
//         glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
// 
//         glDisable(GL_TEXTURE_GEN_S);
//         glDisable(GL_TEXTURE_GEN_T);
// 
//         if(!texture)
//             texture.Generate(*currentImage);
//         else
//             texture.Update(*currentImage);
//         texture.Bind();
//     }
// }
// 
// void ImageStream::Disable()
// {
//     texture.Unbind();
// 
//     glDisable(GL_TEXTURE_2D);
//     glDisable(GL_ALPHA_TEST);
//     glAlphaFunc(GL_ALWAYS, 0);
//     glBlendFunc(GL_ONE, GL_ZERO);
//     glMatrixMode(GL_TEXTURE);
//     glPopMatrix();
// 
//     glMatrixMode(GL_MODELVIEW);
// 
// }
// 
// int ImageStream::perform(int argc, const char **argv, Tcl_Interp *interp)
// {
// 
//     if (argc)
//     {
//         std::string cmd(argv[0]);
//         if(cmd == "connect")
//         {
//             int ret = Object::perform(argc, argv, interp);
//             socket.SetOption(gz4d::Socket::RecvBuffer,524288);
//             if (ret == TCL_OK)
//             {
//                 Start(socket);
//             }
//             return ret;
//         }
//         else if (cmd == "getTextureId")
//         {
//             
//         }
//     }
//     
//     return Object::perform(argc,argv,interp);
// }
