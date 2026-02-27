#ifndef S57_LNDARE_H
#define S57_LNDARE_H

#include"S57Layer.h"
#include"S57_object.h"

class S57_land : public S57_object
{
public:
	S57_land();
	~S57_land();
	DERIVE_OBJECT(S57_land);

private:
};


#endif
