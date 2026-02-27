#ifndef S57_LIGHTS_H
#define S57_LIGHTS_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_light : public S57_object
{
public:
	S57_light();
	~S57_light();
	DERIVE_OBJECT(S57_light);

private:
};

#endif
