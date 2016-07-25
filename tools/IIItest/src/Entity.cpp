#include "III.h"

CEntity::CEntity(void)
{
//	printf("CEntity::CEntity()\n");
	m_type = ENTITY_TYPE_NOTHING;
	m_status = ENTITY_STATUS_4;
	m_doCollision = 0;

}

CEntity::~CEntity(void)
{
//	printf("CEntity::~CEntity()\n");
}
