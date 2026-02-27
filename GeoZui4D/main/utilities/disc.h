#ifndef DISC_H
#define DISC_H
class disc
{
public:
	disc(int n,float dia);
	void initTex();

	void draw();
private:
	float *Xpts;
	float *Ypts;
	int nPts;
	float diam;

	GLubyte waves[32][32][4];
	GLuint waveTex; //the texture handle
	float mov;
	float step;
};

#endif
