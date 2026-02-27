
#include "Wii3D.h"
#include <gz4d/X3D.h>
#include <GL/glut.h>

Wii3D w;

gz4d::X3D auv;

void idle()
{
    w.doIdle();
    glutPostRedisplay();
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDrawBuffer(GL_BACK);
  glPushMatrix();

  glRotatef(w.roll,0.0,1.0,0.0);
  glRotatef(w.pitch,-1.0,0.0,0.0);

  glRotatef(w.heading,0.0,0.0,1.0);
  glScalef(w.scale,w.scale,w.scale);

    glPushMatrix();
    glTranslatef(w.xpos,w.ypos,w.zpos);
    auv.Draw();
    glPopMatrix();

  glPopMatrix();
  glPushMatrix();
    glTranslatef(w.vx,0.0,w.vy);
    glColor3f(1.0,0.0,0.0);
    glutSolidSphere(w.vsize*.1,10,10);
  glPopMatrix();
  glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( /* field of view in degree */ 40.0,
                    /* aspect ratio */ (float)w/(float)h,
                    /* Z near */ 0.1,
                    /* Z far */ 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0,-5.0, 0.0,  /* eye is at (0,-3,0) */
    0.0, 0.0, 0.0,      /* center is at (0,0,0) */
    0.0, 0.0, 1.0);      /* up is in positive Z direction */
}


int main(int argc, char * argv[])
{
    if(!w.scan())
        exit(1);

    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutCreateWindow("Wii3D");
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);

    GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* diffuse light. */
    GLfloat light_ambient[] = {0.9, 0.9, 0.9, 1.0};  /* light. */
    GLfloat light_position[] = {1.0, -1.0, 1.0, 0.0};  /* Infinite light location. */

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glShadeModel (GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);

    //top.Load(boost::filesystem::path("wiimote_top.x3d"));
    //bottom.Load(boost::filesystem::path("wiimote_bottom.x3d"));
    //guts.Load(boost::filesystem::path("wiimote_circuit.x3d"));
    auv.Load(boost::filesystem::path("/home/roland/work/models/auvs/Fetch/Fetch.x3d"));

    glutMainLoop();
    return 0;
}
