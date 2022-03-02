# ifndef QUEST_EXTENDED_H
# define QUEST_EXTENDED_H

#include "QuEST.h"

qreal getQuregRealValue(Qureg *qureg, long long int index);
void setQuregRealValue(Qureg *qureg, long long int index, qreal value);

qreal getQuregImagValue(Qureg *qureg, long long int index);
void setQuregImagValue(Qureg *qureg, long long int index, qreal value);

#endif